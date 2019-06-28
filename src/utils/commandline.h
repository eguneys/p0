
#pragma once

#include <string>
#include <vector>

namespace pzero {

class CommandLine {
 public:
  CommandLine() = delete;

  // This function must be called before any other.
  static void Init(int argc, const char** argv);

  // Name of the executable filename that was run.
  static const std::string& BinaryName() { return binary_; }

  // Directory where the binary is run. Without trailing slash.
  static std::string BinaryDirectory();

  // If the first command line parameter is @command, remove it and return
  // true. Otherwise return false.
  static bool ConsumeCommand(const std::string& command);

  // Command line arguments.
  static const std::vector<std::string>& Arguments() { return arguments_; }

  static void RegisterMode(const std::string& mode,
                           const std::string& description);

  static const std::vector<std::pair<std::string, std::string>>& GetModes() {
    return modes_;
  }

 private:
  static std::string binary_;
  static std::vector<std::string> arguments_;
  static std::vector<std::pair<std::string, std::string>> modes_;
};

}  // namespace pzero
