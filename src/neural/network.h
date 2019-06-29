#pragma once

#include <memory>

namespace pzero {

  class NetworkComputation {
  public:
  };
  
  class Network {
  public:
    virtual std::unique_ptr<NetworkComputation> NewComputation() = 0;
    virtual ~Network() {};
  };
  
} // namespace pzero
