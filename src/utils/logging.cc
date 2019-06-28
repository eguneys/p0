#include "utils/logging.h"

#include <iomanip>
#include <iostream>
#include <thread>

namespace pzero {

  namespace {
    size_t kBufferSizeLines = 200;
    const char* kStderrFilename = "<stderr>";
  }

  Logging& Logging::Get() {
    static Logging logging;
    return logging;
  }

  void Logging::WriteLineRaw(const std::string& line) {
    Mutex::Lock lock_(mutex_);
    if (filename_.empty()) {
      buffer_.push_back(line);
      if (buffer_.size() > kBufferSizeLines) buffer_.pop_front();
    } else {
      auto& file = (filename_ == kStderrFilename) ? std::cerr : file_;
      file << line << std::endl;
    }
  }

  void Logging::SetFilename(const std::string& filename) {
    Mutex::Lock lock_(mutex_);
    if (filename_ == filename) return;
    filename_ = filename;
    if (filename.empty() || filename == kStderrFilename) {
      file_.close();
    }
    if (filename.empty()) return;
    if (filename != kStderrFilename) file_.open(filename, std::ios_base::app);
    auto& file = (filename == kStderrFilename) ? std::cerr : file_;
    file << "\n\n================ Log started. ==================" << std::endl;
    buffer_.clear();
  }


  LogMessage::LogMessage(const char* file, int line) {
    *this << FormatTime(std::chrono::system_clock::now()) << ' '
          << std::setfill(' ') << std::this_thread::get_id() << std::setfill('0') << ' ' << file << ':' << line << "] ";
  }

  LogMessage::~LogMessage() { Logging::Get().WriteLineRaw(str()); }


  StderrLogMessage::StderrLogMessage(const char* file, int line)
    : log_(file, line) {}

  StderrLogMessage::~StderrLogMessage() {
    std::cerr << str() << std::endl;
    log_ << str();
  }

  std::string FormatTime(std::chrono::time_point<std::chrono::system_clock> time) {
    std::ostringstream ss;
    using namespace std::chrono;
    const auto us =
      duration_cast<microseconds>(time.time_since_epoch()).count() % 1000000;
    auto timer = std::chrono::system_clock::to_time_t(time);
    ss << std::put_time(std::localtime(&timer), "%m%d %T") << "."
       << std::setfill('0') << std::setw(6) << us;
    return ss.str();
  }

}
