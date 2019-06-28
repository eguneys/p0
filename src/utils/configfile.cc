
#include <fstream>
#include <sstream>

#include "utils/commandline.h"
#include "utils/configfile.h"
#include "utils/logging.h"
#include "utils/optionsparser.h"
#include "utils/string.h"

namespace pzero {
namespace {
const OptionId kConfigFileId{
    "config", "ConfigFile",
    "Path to a configuration file. The format of the file is one command line "
    "parameter per line, e.g.:\n--weights=/path/to/weights",
    'c'};
const char* kDefaultConfigFile = "lc0.config";
}  // namespace

std::vector<std::string> ConfigFile::arguments_;

void ConfigFile::PopulateOptions(OptionsParser* options) {
  options->Add<StringOption>(kConfigFileId) = kDefaultConfigFile;
}

// This is needed to get the config file from the parameters without calling
// ProcessAllFlags() that should be called only once, and needs the config file.
std::string ConfigFile::ProcessConfigFlag(
    const std::vector<std::string>& args) {
  std::string filename = kDefaultConfigFile;
  for (auto iter = args.begin(), end = args.end(); iter != end; ++iter) {
    std::string param = *iter;

    if (param.substr(0, 2) == "--") {
      param = param.substr(2);
      const auto pos = param.find('=');
      if (pos != std::string::npos) {
        if (param.substr(0, pos) == kConfigFileId.long_flag) {
          filename = param.substr(pos + 1);
        }
      }
    }
    if (param.size() == 2 && param[0] == '-') {
      if (param[1] == kConfigFileId.short_flag && iter + 1 != end) {
        filename = *(iter + 1);
        ++iter;
      }
    }
  }
  return filename;
}

bool ConfigFile::Init(OptionsParser* options) {
  arguments_.clear();

  // Get the relative path from the config file parameter.
  std::string filename = ProcessConfigFlag(CommandLine::Arguments());

  // If filename is an empty string then return true.  This is to override
  // loading the default configuration file.
  if (filename == "") return true;

  filename = CommandLine::BinaryDirectory() + "/" + filename;

  // Parses the file into the arguments_ vector.
  if (!ParseFile(filename, options)) return false;

  return true;
}

bool ConfigFile::ParseFile(const std::string& filename,
                           OptionsParser* options) {
  std::ifstream input(filename);

  // Check to see if we are using the default config file or not.
  OptionsDict dict = options->GetOptionsDict();
  const bool using_default_config =
      dict.IsDefault<std::string>(kConfigFileId.GetId());

  if (!input.is_open()) {
    // It is okay if we cannot open the default file since it is normal
    // for it to not exist.
    if (using_default_config) return true;

    CERR << "Could not open configuration file: " << filename;
    return false;
  }

  CERR << "Found configuration file: " << filename;

  for (std::string line; getline(input, line);) {
    // Remove all leading and trailing whitespace.
    line = Trim(line);
    // Ignore comments.
    if (line.substr(0, 1) == "#") continue;
    // Skip blank lines.
    if (line.length() == 0) continue;
    // Allow long form arugments that omit '--'.  If omitted, add here.
    if (line.substr(0, 1) != "-" && line.substr(0, 2) != "--") {
      line = "--" + line;
    }
    // Fail now if the argument does not begin with '--'.
    if (line.substr(0, 2) != "--") {
      CERR << "Only '--' arguments are supported in the "
           << "configuration file: '" << line << "'.";
      return false;
    }
    // Add the line to the arguments list.
    arguments_.emplace_back(line);
  }

  return true;
}

}  // namespace pzero
