
#include "utils/commandline.h"
#include "utils/logging.h"

namespace pzero {

std::string CommandLine::binary_;
std::vector<std::string> CommandLine::arguments_;
std::vector<std::pair<std::string, std::string>> CommandLine::modes_;

void CommandLine::Init(int argc, const char** argv) {
  binary_ = argv[0];
  arguments_.clear();
  std::ostringstream params;
  for (int i = 1; i < argc; ++i) {
    params << ' ' << argv[i];
    arguments_.push_back(argv[i]);
  }
  LOGFILE << "Command line: " << binary_ << params.str();
}

bool CommandLine::ConsumeCommand(const std::string& command) {
  if (arguments_.empty()) return false;
  if (arguments_[0] != command) return false;
  arguments_.erase(arguments_.begin());
  return true;
}

void CommandLine::RegisterMode(const std::string& mode,
                               const std::string& description) {
  modes_.emplace_back(mode, description);
}

std::string CommandLine::BinaryDirectory() {
  std::string path = binary_;
  const auto pos = path.find_last_of("\\/");
  if (pos == std::string::npos) {
    return ".";
  }
  path.resize(pos);
  return path;
}

}  // namespace pzero
