#pragma once

#include "utils/optionsdict.h"
#include "utils/optionsparser.h"

namespace pzero {
  
  class SearchParams {
  public:
    SearchParams(const OptionsDict& options);
    SearchParams(const SearchParams&) = delete;

    static void Populate(OptionsParser* options);

    int GetMiniBatchSize() const {
      return kMiniBatchSize;
    }

    float GetCpuct() const { return kCpuct; }
    float GetCpuctBase() const { return kCpuctBase; }
    float GetCpuctFactor() const { return kCpuctFactor; }
    float GetTemperature() const {
      return options_.Get<float>(kTemperatureId.GetId());
    }

    float GetFpuValue() const { return kFpuValue; }


    static const OptionId kMiniBatchSizeId;
    static const OptionId kCpuctId;
    static const OptionId kCpuctBaseId;
    static const OptionId kCpuctFactorId;
    static const OptionId kTemperatureId;
    static const OptionId kFpuValueId;
    
  private:
    const OptionsDict& options_;

    const float kCpuct;
    const float kCpuctBase;
    const float kCpuctFactor;
    const float kFpuValue;
    const int kMiniBatchSize;
    
  };
  
} // namespace pzero
