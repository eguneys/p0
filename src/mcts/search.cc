#include "mcts/search.h"

#include <algorithm>
#include <cmath>
#include <thread>

#include "neural/encoder.h"

namespace pzero {
  
  Search::Search(const NodeTree& tree, Network* network,
                 BestMoveInfo::Callback best_move_callback,
                 const SearchLimits& limits,
                 const OptionsDict& options):
    root_node_(tree.GetCurrentHead()),
    played_history_(tree.GetPositionHistory()),
    network_(network),
    limits_(limits),
    start_time_(std::chrono::steady_clock::now()),
    best_move_callback_(best_move_callback),
    params_(options) {}

  void Search::StartThreads(size_t how_many) {
    Mutex::Lock lock(threads_mutex_);

    if (threads_.size() == 0) {
      threads_.emplace_back([this]() { WatchdogThread(); });
    }

    while (threads_.size() <= how_many) {
      threads_.emplace_back
        ([this]() {
           SearchWorker worker(this, params_);
           worker.RunBlocking();
         });
    }
  }

  void Search::RunBlocking(size_t threads) {
    StartThreads(threads);
    Wait();
  }

  bool Search::IsSearchActive() const {
    return !stop_.load(std::memory_order_acquire);
  }

  int64_t Search::GetTimeToDeadline() const {
    return std::chrono::duration_cast<std::chrono::milliseconds>
      (limits_.search_deadline - std::chrono::steady_clock::now()).count();
  }

  namespace {
    
    inline float GetFpu(const SearchParams& params, Node* node, bool is_root_node) {
      const auto value = params.GetFpuValue();
      return value;
    }

    inline float ComputeCpuct(const SearchParams& params, uint32_t N) {
      const float init = params.GetCpuct();
      const float k = params.GetCpuctFactor();
      const float base = params.GetCpuctBase();
      //return init + (k ? k * FastLog((N + base) / base) : 0.0f);
      return init;
    }

  } // namespace

  float Search::GetBestEval() const {
    SharedMutex::SharedLock lock(nodes_mutex_);
    Mutex::Lock counters_lock(counters_mutex_);
    float parent_q = -root_node_->GetQ();
    if (!root_node_->HasChildren()) return parent_q;
    EdgeAndNode best_edge = GetBestChildNoTemperature(root_node_);
    return best_edge.GetQ(parent_q);
  }

  Move Search::GetBestMove() {
    SharedMutex::Lock lock(nodes_mutex_);
    Mutex::Lock counters_lock(counters_mutex_);
    EnsureBestMoveKnown();
    return final_bestmove_.GetMove();
  }

  void Search::EnsureBestMoveKnown() REQUIRES(nodes_mutex_)
    REQUIRES(counters_mutex_) {
    if (bestmove_is_sent_) return;
    if (!root_node_->HasChildren()) return;

    final_bestmove_ = GetBestChildNoTemperature(root_node_);
  }

  std::vector<EdgeAndNode> Search::GetBestChildrenNoTemperature(Node* parent, int count) const {
    
    using El = std::tuple<uint64_t, float, float, EdgeAndNode>;
    std::vector<El> edges;
    for (auto edge : parent->Edges()) {
      edges.emplace_back(edge.GetN(), edge.GetQ(0), edge.GetP(), edge);
    }

    const auto middle = (static_cast<int>(edges.size()) > count)
      ? edges.begin() + count
      : edges.end();    
    
    std::partial_sort(edges.begin(), middle, edges.end(), std::greater<El>());
    
    std::vector<EdgeAndNode> res;
    std::transform(edges.begin(), middle, std::back_inserter(res),
                   [](const El& x) { return std::get<3>(x); });

    return res;
  }

  EdgeAndNode Search::GetBestChildNoTemperature(Node* parent) const {
    auto res = GetBestChildrenNoTemperature(parent, 1);
    return res.empty() ? EdgeAndNode() : res.front();
  }

  void Search::MaybeTriggerStop() {
    SharedMutex::Lock nodes_lock(nodes_mutex_);
    Mutex::Lock lock(counters_mutex_);

    if (bestmove_is_sent_) return;

    if (!stop_.load(std::memory_order_acquire)) {
      if (GetTimeToDeadline() <= 0) {
        FireStopInternal();
      }
    }
    if (stop_.load(std::memory_order_acquire) && !bestmove_is_sent_) {
      EnsureBestMoveKnown();
      best_move_callback_(final_bestmove_.GetMove());
      bestmove_is_sent_ = true;
    }
  }

  void Search::WatchdogThread() {
    while (true) {
      MaybeTriggerStop();


      Mutex::Lock lock(counters_mutex_);

      if (bestmove_is_sent_) break;
    }
  }

  void Search::FireStopInternal() {
    stop_.store(true, std::memory_order_release);
  }

  void Search::Stop() {
    Mutex::Lock lock(counters_mutex_);
    FireStopInternal();
    LOGFILE << "Stopping search due to `stop` uci command.";
  }

  void Search::Abort() {
    Mutex::Lock lock(counters_mutex_);
    if (!stop_.load(std::memory_order_acquire) ||
        (!bestmove_is_sent_)) {
      bestmove_is_sent_ = true;
      FireStopInternal();
    }
  }

  void Search::Wait() {
    Mutex::Lock lock(threads_mutex_);
    while (!threads_.empty()) {
      threads_.back().join();
      threads_.pop_back();
    }
  }

  Search::~Search() {
    Abort();
    Wait();
    LOGFILE << "Search destroyed.";
  }

  ///////////////
  // SearchWorker
  ///////////////

  void SearchWorker::ExecuteOneIteration() {

    InitializeIteration(search_->network_->NewComputation());

    GatherMinibatch();

    RunNNComputation();

    FetchMinibatchResults();

    DoBackupUpdate();
  }


  void SearchWorker::InitializeIteration
  (std::unique_ptr<NetworkComputation> computation) {

    computation_ = std::move(computation);

    minibatch_.clear();
  }

  void SearchWorker::GatherMinibatch() {
    int minibatch_size = 0;
    int collision_events_left = 10;

    while (minibatch_size < params_.GetMiniBatchSize()) {
      minibatch_.emplace_back(PickNodeToExtend());

      auto& picked_node = minibatch_.back();
      auto* node = picked_node.node;
        
      if (picked_node.IsCollision()) {
        if (--collision_events_left <= 0) return;
        continue;
      }

      ++minibatch_size;

      if (picked_node.IsExtendable()) {
        ExtendNode(node);

        if (!node->IsTerminal()) {
          picked_node.nn_queried = true;
          AddNodeToComputation(node);
        }
      }
      if (search_->stop_.load(std::memory_order_acquire)) return;
    }
  }

  SearchWorker::NodeToProcess SearchWorker::PickNodeToExtend() {
    Node* node = search_->root_node_;
    Node::Iterator best_edge;

    SharedMutex::Lock lock(search_->nodes_mutex_);

    bool is_root_node = true;
    uint16_t depth = 0;
    bool node_already_updated = true;

    while (true) {
     
      if (!node_already_updated) {
        node = best_edge.GetOrSpawnNode(node);
      }
      best_edge.Reset();
      depth++;

      if (!node->TryStartScoreUpdate()) {
        if (!is_root_node) {
          // IncrementNInFlight(node->GetParent(), search_->root_node_)
        }
        return NodeToProcess::Collision(node, depth);
      }

      if (node->IsTerminal() || !node->HasChildren()) {
        return NodeToProcess::Visit(node, depth);
      }

      node_already_updated = false;

      const float cpuct = ComputeCpuct(params_, node->GetN());
      const float puct_mult =
        cpuct * std::sqrt(std::max(node->GetChildrenVisits(), 1u));
      float best = std::numeric_limits<float>::lowest();
      int possible_moves = 0;
      const float fpu = GetFpu(params_, node, is_root_node);
      for (auto child : node->Edges()) {
        if (is_root_node) {
          ++possible_moves;
        }
        const float Q = child.GetQ(fpu);
        const float score = child.GetU(puct_mult) + Q;
        if (score > best) {
          best = score;
          best_edge = child;
        }
      }
      is_root_node = false;
    }
  }

  void SearchWorker::ExtendNode(Node* node) {
    history_.Trim(search_->played_history_.GetLength());

    std::vector<Move> to_add;

    to_add.reserve(4);

    Node* cur = node;
    while (cur != search_->root_node_) {
      Node* prev = cur->GetParent();
      to_add.push_back(prev->GetEdgeToNode(cur)->GetMove());
      cur = prev;
    }

    for (int i = to_add.size() - 1; i >= 0; i--) {
      history_.Append(to_add[i]);
    }

    const auto& board = history_.Last().GetBoard();
    auto legal_moves = board.GenerateLegalMoves();

    if (board.IsEnd()) {
      node->MakeTerminal(GameResult::WIN);
      return;
    }

    if (board.IsStuck()) {
      node->MakeTerminal(GameResult::LOSE);
      return;
    }

    if (history_.Last().GetRepetitions() >= 1) {
      node->MakeTerminal(GameResult::LOSE);
      return;
    }

    node->CreateEdges(legal_moves);
  }


  void SearchWorker::AddNodeToComputation(Node* node) {

    auto planes = EncodePositionForNN(history_, 8);
    
    std::vector<uint16_t> moves;

    if (node && node->HasChildren()) {
      moves.reserve(node->GetNumEdges());
      for (const auto& edge : node->Edges()) {
        moves.emplace_back(edge.GetMove().as_nn_index());
      }
    }
    
    //computation_->AddInput(std::move(planes), std::move(moves));
    computation_->AddInput(std::move(planes));
  }

  void SearchWorker::RunNNComputation() { computation_->ComputeBlocking(); }

  void SearchWorker::FetchMinibatchResults() {
    int idx_in_computation = 0;
    for (auto& node_to_process : minibatch_) {
      FetchSingleNodeResult(&node_to_process, idx_in_computation);
      if (node_to_process.nn_queried) ++idx_in_computation;
    }
  }

  void SearchWorker::FetchSingleNodeResult(NodeToProcess* node_to_process, int idx_in_computation) {
   
    Node* node = node_to_process->node;

    if (!node_to_process->nn_queried) {
      node_to_process->v = node->GetQ();
      return;
    }

    node_to_process->v = -computation_->GetQVal(idx_in_computation);

    float total = 0.0;
    for (auto edge : node->Edges()) {
      float p = 
        computation_->GetPVal(idx_in_computation, edge.GetMove().as_nn_index());
      edge.edge()->SetP(p);

      total += edge.edge()->GetP();
    }

    if (total > 0.0f) {
      const float scale = 1.0f / total;
      for (auto edge : node->Edges()) edge.edge()->SetP(edge.GetP() * scale);
    }
  }

  void SearchWorker::DoBackupUpdate() {
    SharedMutex::Lock lock(search_->nodes_mutex_);

    for (const NodeToProcess& node_to_process : minibatch_) {
      DoBackupUpdateSingleNode(node_to_process);
    }
  }

  void SearchWorker::DoBackupUpdateSingleNode
  (const NodeToProcess& node_to_process) REQUIRES(search_->nodes_mutex_) {
    Node* node = node_to_process.node;

    if (node_to_process.IsCollision()) {
      for (node = node->GetParent(); node != search_->root_node_->GetParent();
           node = node->GetParent()) {
        node->CancelScoreUpdate(node_to_process.multivisit);
      }
      return;
    }

    float v = node_to_process.v;

    for (Node *n = node, *p; n != search_->root_node_->GetParent(); n = p) {
      p = n->GetParent();

      if (n->IsTerminal()) {
        v = n->GetQ();
      }

      n->FinalizeScoreUpdate(v, node_to_process.multivisit);

      if (!p) break;
    }
  }  
} // namespace pzero
