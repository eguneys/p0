#pragma once

#include <shared_mutex>
#include <thread>
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
           const SearchLimits& limits,
           const OptionsDict& options);

    ~Search();

    void StartThreads(size_t how_many);
  };
  
} // namespace pzero
