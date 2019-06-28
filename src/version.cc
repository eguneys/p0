#include "version.h"

std::string GetVersionStr(int major, int minor, int patch,
                          const std::string& postfix) {
  auto v = std::to_string(major) + "." + std::to_string(minor) + "." +
    std::to_string(patch);

  if (postfix.empty()) return v;
  return v + "-" + postfix;
}
