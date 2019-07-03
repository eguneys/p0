#include <cstring>
#include "neural/factory.h"
#include "utils/hashcat.h"

namespace pzero {

  namespace {
    
    class RandomNetworkComputation : public NetworkComputation {
    public:
      RandomNetworkComputation() {}

      void AddInput(InputPlanes&& input) override {
        std::uint64_t hash = 0;

        for (const auto& plane : input) {
          hash = HashCat({hash, plane.mask});
          std::uint32_t tmp;
          std::memcpy(&tmp, &plane.value, sizeof(float));
          const std::uint64_t value_hash = tmp;
          hash = HashCat({hash,value_hash});
        }
        inputs_.push_back(hash);
      }

      void ComputeBlocking() override {
      }

      float GetQVal(int sample) const override {
        return (int(inputs_[sample] % 200000) - 100000) / 100000.0;
      }

      float GetPVal(int sample, int move_id) const override {
        return (HashCat({ inputs_[sample], static_cast<unsigned long>(move_id)}) % 10000) / 10000.0;
      }

    private:
      std::vector<std::uint64_t> inputs_;
    };

    class RandomNetwork : public Network {
    public:
      RandomNetwork(const OptionsDict& options) {}

      std::unique_ptr<NetworkComputation> NewComputation() override {
        return std::make_unique<RandomNetworkComputation>();
      }
    };

    
  } // namespace

  std::unique_ptr<Network> MakeRandomNetwork
  (// const WeightsFile&,
   const OptionsDict& options) {
    return std::make_unique<RandomNetwork>(options);
  }

  REGISTER_NETWORK("random", MakeRandomNetwork, -900)
  
} // namespace pzero
