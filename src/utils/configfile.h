
#pragma once

#include <string>
#include <vector>

namespace pzero {

class OptionsParser;

class ConfigFile {
 public:
  ConfigFile() = delete;

  // This function must be called after PopulateOptions.
  static bool Init(OptionsParser* options);

  // Returns the command line arguments from the config file.
  static const std::vector<std::string>& Arguments() { return arguments_; }

  // Add the config file parameter to the options dictionary.
  static void PopulateOptions(OptionsParser* options);

 private:
  // Parses the config file into the arguments_ vector.
  static bool ParseFile(const std::string& filename, OptionsParser* options);

  static std::string ProcessConfigFlag(const std::vector<std::string>& args);

  static std::vector<std::string> arguments_;
};

}  // namespace pzero
