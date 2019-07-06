#include "mcts/params.h"

namespace pzero {

  const OptionId SearchParams::kMiniBatchSizeId{
    "minibatch-size", "MinibatchSize",
      "How many positions the engine tries to batch together for parallel NN "
      "computation."};

  const OptionId SearchParams::kCpuctId{
    "cpuct", "CPuct",
      "cpuct_init constant from \"UCT search\" algorithm."};

  const OptionId SearchParams::kCpuctBaseId{
    "cpuct-base", "CPuctBase",
      "cpuct_base constant from \"UCT search\" algorithm."};

  const OptionId SearchParams::kCpuctFactorId{
    "cpuct-factor", "CPuctFactor",
      "Multiplier for the cpuct growth formula."};

  const OptionId SearchParams::kTemperatureId{
    "temperature", "Temperature",
      "Tau value from softmax formula for the first move."};

  const OptionId SearchParams::kFpuValueId{
    "fpu-value", "FpuValue",
      "\"First Play Urgency\" value used to adjust unvisited node eval."};

  
  void SearchParams::Populate(OptionsParser* options) {

    options->Add<IntOption>(kMiniBatchSizeId, 1, 1024) = 256;
    options->Add<FloatOption>(kCpuctId, 0.0f, 100.0f) = 3.0f;
    options->Add<FloatOption>(kCpuctBaseId, 1.0f, 1000000.0f) = 19652.0f;
    options->Add<FloatOption>(kCpuctFactorId, 0.0f, 1000.0f) = 2.0f;
    options->Add<FloatOption>(kTemperatureId, 0.0f, 100.0f) = 0.0f;
    
    options->Add<FloatOption>(kFpuValueId, -100.0f, 100.0f) = 1.2f;
    
  }

  SearchParams::SearchParams(const OptionsDict& options)
    : options_(options),
      kCpuct(options.Get<float>(kCpuctId.GetId())),
      kCpuctBase(options.Get<float>(kCpuctBaseId.GetId())),
      kCpuctFactor(options.Get<float>(kCpuctFactorId.GetId())),
      kFpuValue(options.Get<float>(kFpuValueId.GetId())),
      kMiniBatchSize(options.Get<int>(kMiniBatchSizeId.GetId())) {
    
  }

} // namespace pzero
