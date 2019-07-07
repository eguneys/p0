
#pragma once

#include <random>
#include <string>
#include "utils/mutex.h"

namespace pzero {

class Random {
 public:
  static Random& Get();
  double GetDouble(double max_val);
  float GetFloat(float max_val);
  double GetGamma(double alpha, double beta);
  // Both sides are included.
  int GetInt(int min, int max);
  std::string GetString(int length);
  bool GetBool();

 private:
  Random();

  Mutex mutex_;
  std::mt19937 gen_ GUARDED_BY(mutex_);
};

}  // namespace pzero
