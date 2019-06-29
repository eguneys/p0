#include "mcts/node.h"

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
  
  void NodeTree::DeallocateTree() {
    gNodeGc.AddToGcQueue(std::move(gamebegin_node_));

    gamebegin_node_ = nullptr;
    current_head_ = nullptr;
  }
  
} // namespace pzero
