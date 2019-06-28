#pragma once

#include <stdexcept>
#include "utils/logging.h"

namespace pzero {
  
  class Exception : public std::runtime_error {
  public:
  Exception(const std::string& what) : std::runtime_error(what) {
      LOGFILE << "Exception: " << what;
    }
  };
} //namespace pzero
