#pragma once

#include <string>
#include "version.inc"

std::string GetVersionStr(int major = P0_VERSION_MAJOR,
                          int minor = P0_VERSION_MINOR,
                          int patch = P0_VERSION_PATCH,
                          const std::string& postfix = P0_VERSION_POSTFIX);
