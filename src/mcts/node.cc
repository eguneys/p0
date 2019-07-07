#include "mcts/node.h"

#include <cassert>
#include <thread>
#include "neural/encoder.h"
#include "neural/network.h"
#include "utils/exception.h"

namespace pzero {

  namespace {
    // Periodicity of garbage collection, milliseconds.
    const int kGCIntervalMs = 100;

    // Every kGCIntervalMs milliseconds release nodes in a separate GC thread.
    class NodeGarbageCollector {
    public:
      NodeGarbageCollector() : gc_thread_([this]() { Worker(); }) {}

      // Takes ownership of a subtree, to dispose it in a separate thread when
      // it has time.
      void AddToGcQueue(std::unique_ptr<Node> node) {
        if (!node) return;
        Mutex::Lock lock(gc_mutex_);
        subtrees_to_gc_.emplace_back(std::move(node));
      }

      ~NodeGarbageCollector() {
        // Flips stop flag and waits for a worker thread to stop.
        stop_.store(true);
        gc_thread_.join();
      }

    private:
      void GarbageCollect() {
        while (!stop_.load()) {
          // Node will be released in destructor when mutex is not locked.
          std::unique_ptr<Node> node_to_gc;
          {
            // Lock the mutex and move last subtree from subtrees_to_gc_ into
            // node_to_gc.
            Mutex::Lock lock(gc_mutex_);
            if (subtrees_to_gc_.empty()) return;
            node_to_gc = std::move(subtrees_to_gc_.back());
            subtrees_to_gc_.pop_back();
          }
        }
      }

      void Worker() {
        while (!stop_.load()) {
          std::this_thread::sleep_for(std::chrono::milliseconds(kGCIntervalMs));
          GarbageCollect();
        };
      }

      mutable Mutex gc_mutex_;
      std::vector<std::unique_ptr<Node>> subtrees_to_gc_ GUARDED_BY(gc_mutex_);

      // When true, Worker() should stop and exit.
      std::atomic<bool> stop_{false};
      std::thread gc_thread_;
    };  // namespace

    NodeGarbageCollector gNodeGc;
  }  // namespace

  ////////
  // Edge
  ///////

  Move Edge::GetMove() const {
    return move_;
  }

  void Edge::SetP(float p) {
    assert(0.0f <= p && p <= 1.0f);
    p_ = p;
  }

  float Edge::GetP() const {
    return p_;
  }
  
  ////////////
  // NodeTree
  ///////////

  void NodeTree::MakeMove(Move move) {
    Node* new_head = nullptr;
    for (auto& n : current_head_->Edges()) {
      if (n.GetMove() == move) {
        new_head = n.GetOrSpawnNode(current_head_);
        break;
      }
    }
    current_head_ = new_head ? new_head : current_head_->CreateSingleChildNode(move);
    history_.Append(move);
  }

  void NodeTree::TrimTreeAtHead() {
    auto tmp = std::move(current_head_->sibling_);
    gNodeGc.AddToGcQueue(std::move(current_head_->child_));
    *current_head_ = Node(current_head_->GetParent(), current_head_->index_);
    current_head_->sibling_ = std::move(tmp);    
  }

  bool NodeTree::ResetToPosition(const std::string& starting_fen,
                                 const std::vector<Move>& moves) {

    SokoBoard starting_board;
    starting_board.SetFromFen(starting_fen);

    DeallocateTree();

    if (!gamebegin_node_) {
      gamebegin_node_ = std::make_unique<Node>(nullptr, 0);
    }

    history_.Reset(starting_board);

    Node* old_head = current_head_;
    current_head_ = gamebegin_node_.get();
    for (const auto& move : moves) {
      MakeMove(move);
    }

    return true;
  }


  void NodeTree::DeallocateTree() {
    gNodeGc.AddToGcQueue(std::move(gamebegin_node_));

    gamebegin_node_ = nullptr;
    current_head_ = nullptr;
  }

  ////////////
  // EdgeList
  ///////////

  EdgeList::EdgeList(MoveList moves)
    : edges_(std::make_unique<Edge[]>(moves.size())), size_(moves.size()) {
        auto* edge= edges_.get();
        for (const auto move: moves) edge++->SetMove(move);
  }


  ////////
  // Node
  ///////

  Node* Node::CreateSingleChildNode(Move move) {
    assert(!edges_);
    assert(!child_);
    edges_ = EdgeList({move});
    child_ = std::make_unique<Node>(this, 0);
    return child_.get();
  }

  void Node::CreateEdges(const MoveList& moves) {
    assert(!edges_);
    assert(!child_);
    edges_ = EdgeList(moves);
  }

  Node::ConstIterator Node::Edges() const { return {edges_, &child_}; }
  Node::Iterator Node::Edges() { return {edges_, &child_}; }

  Edge* Node::GetEdgeToNode(const Node* node) const {
    assert(node->parent_ == this);
    assert(node->index_ < edges_.size());
    return &edges_[node->index_];
  }


  void Node::MakeTerminal(GameResult result) {
    is_terminal_ = true;
    if (result == GameResult::WIN) {
      q_ = 1.0f;
    } else if (result == GameResult::LOSE) {
      q_ = -1.0f;
    }
  }

  bool Node::TryStartScoreUpdate() {
    if (n_ == 0 && n_in_flight_ > 0) return false;
    ++n_in_flight_;
    return true;
  }

  void Node::CancelScoreUpdate(int multivisit) {
    n_in_flight_ -= multivisit;
  }

  void Node::FinalizeScoreUpdate(float v, int multivisit) {
    q_ += multivisit * (v - q_) / (n_ + multivisit);

    n_ += multivisit;
    n_in_flight_ -= multivisit;
  }
  
  V4TrainingData Node::GetV4TrainingData
  (GameResult game_result,
   const PositionHistory& history,
   float best_q) const {
    V4TrainingData result;
    
    result.version = 4;

    const float total_n = static_cast<float>(GetChildrenVisits());
    if (total_n <= 0.0f) throw Exception("Search generated invalid data!");

    std::fill(std::begin(result.probabilities), std::end(result.probabilities), -1);

    for (const auto& child : Edges()) {
      result.probabilities[child.edge()->GetMove().as_nn_index()] =
        child.GetN() / total_n;
    }

    InputPlanes planes = EncodePositionForNN(history, 8);
    int plane_idx = 0;
    for (auto& plane : result.planes) {
      plane = planes[plane_idx++].mask;
    }

    const auto& position = history.Last();

    if (game_result == GameResult::WIN) {
      result.result = 1;
    } else if (game_result == GameResult::LOSE) {
      result.result = -1;
    } else {
      result.result = 0;
    }

    result.root_q = -GetQ();
    result.best_q = best_q;
    
    return result;
  }

} // namespace pzero
