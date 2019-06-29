#include "mcts/params.h"

namespace pzero {

  void SearchParams::Populate(OptionsParser* options) {
    
  }

  SearchParams::SearchParams(const OptionsDict& options)
    : options_(options) {
    
  }

} // namespace pzero
