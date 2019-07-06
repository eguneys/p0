#pragma once

#include <shared_mutex>
#include <thread>
#include "soko/callbacks.h"
#include "mcts/node.h"
#include "mcts/params.h"
#include "neural/network.h"
#include "utils/mutex.h"

namespace pzero {
  
  struct SearchLimits {
    std::chrono::steady_clock::time_point search_deadline;
  };

  class Search {
  public:
    Search(const NodeTree& tree, Network* network,
           BestMoveInfo::Callback best_move_callback,
           const SearchLimits& limits,
           const OptionsDict& options);

    ~Search();

    void StartThreads(size_t how_many);

    void RunBlocking(size_t threads);
    
    void Stop();
    
    void Abort();
    
    void Wait();

    bool IsSearchActive() const;

    Move GetBestMove();

    float GetBestEval() const;
  private:
    void EnsureBestMoveKnown();

    EdgeAndNode GetBestChildNoTemperature(Node* parent) const;
    std::vector<EdgeAndNode> GetBestChildrenNoTemperature(Node* parent, int count) const;

    int64_t GetTimeToDeadline() const;
    void MaybeTriggerStop();
    
    void FireStopInternal();
    
    void WatchdogThread();

    mutable Mutex counters_mutex_ ACQUIRED_AFTER(nodes_mutex_);

    std::atomic<bool> stop_{false};

    bool bestmove_is_sent_ GUARDED_BY(counters_mutex_) = false;

    EdgeAndNode final_bestmove_ GUARDED_BY(counters_mutex_);

    Mutex threads_mutex_;
    std::vector<std::thread> threads_ GUARDED_BY(threads_mutex_);

    Node* root_node_;

    const PositionHistory& played_history_;

    Network* const network_;
    const SearchLimits limits_;
    const std::chrono::steady_clock::time_point start_time_;

    mutable SharedMutex nodes_mutex_;

    BestMoveInfo::Callback best_move_callback_;
    const SearchParams params_;
    
    friend class SearchWorker;
  };

  class SearchWorker {
  public:
    SearchWorker(Search* search, const SearchParams& params)
      : search_(search),
      history_(search_->played_history_),
      params_(params) {}
    
    void RunBlocking() {
      LOGFILE << "Started search thread.";
      do {
        ExecuteOneIteration();
      } while (search_->IsSearchActive());
    }

    void ExecuteOneIteration();

  private:
    struct NodeToProcess {
      bool IsExtendable() const { return !is_collision && !node->IsTerminal(); }
      bool IsCollision() const { return is_collision; }
      
      Node* node;
      uint16_t depth;
      bool nn_queried = false;
      bool is_collision = false;

      static NodeToProcess Collision(Node* node, uint16_t depth) {
        return NodeToProcess(node, depth, true);
      }

      static NodeToProcess Visit(Node* node, uint16_t depth) {
        return NodeToProcess(node, depth, false);
      }

    private:
      NodeToProcess(Node* node, uint16_t depth, bool is_collision)
        : node(node),
        depth(depth),
        is_collision(is_collision) {}
    };
    
    Search* const search_;

    std::vector<NodeToProcess> minibatch_;
    PositionHistory history_;
    const SearchParams& params_;
  };
  
} // namespace pzero
