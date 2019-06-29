#include "mcts/search.h"

namespace pzero {
  
  Search::Search(const NodeTree& tree, Network* network,
                 const SearchLimits& limits,
                 const OptionsDict& options):
    root_node_(tree.GetCurrentHead()),
    played_history_(tree.GetPositionHistory()),
    network_(network),
    limits_(limits),
    start_time_(std::chrono::steady_clock::now()),
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

  bool Search::IsSearchActive() const {
    return !stop_.load(std::memory_order_acquire);
  }

  void Search::WatchdogThread() {
    
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
