#include "mcts/search.h"

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
    std::vector<EdgeAndNode> res;

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
    
  }

  

  
} // namespace pzero
