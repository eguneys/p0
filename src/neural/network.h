#pragma once

#include <memory>
#include <vector>

namespace pzero {

  struct InputPlane {
    InputPlane() = default;
    void SetAll() { mask = ~0ull; }
    void Fill(float val) {
      SetAll();
      value = val;
    }
    std::uint64_t mask = 0ull;
    float value = 1.0f;
  };

  using InputPlanes = std::vector<InputPlane>;

  class NetworkComputation {
  public:
    virtual void AddInput(InputPlanes&& input) = 0;

    virtual void ComputeBlocking() = 0;

    virtual float GetQVal(int sample) const = 0;

    virtual float GetPVal(int sample, int move_id) const = 0;

    virtual ~NetworkComputation() {}
  };
  
  class Network {
  public:
    virtual std::unique_ptr<NetworkComputation> NewComputation() = 0;
    virtual ~Network() {};
  };
  
} // namespace pzero
