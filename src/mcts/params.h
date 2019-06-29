#pragma once

#include "utils/optionsdict.h"
#include "utils/optionsparser.h"

namespace pzero {
  
  class SearchParams {
  public:
    SearchParams(const OptionsDict& options);
    SearchParams(const SearchParams&) = delete;

    static void Populate(OptionsParser* options);

  private:
    const OptionsDict& options_;
    
  };
  
} // namespace pzero
