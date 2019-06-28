#pragma once

#include <chrono>
#include <deque>
#include <fstream>
#include <sstream>

#include "utils/mutex.h"

namespace pzero {

  class Logging {
  public:
    static Logging& Get();

    void SetFilename(const std::string& filename);

  private:
    void WriteLineRaw(const std::string& line);

    Mutex mutex_;
    std::string filename_ GUARDED_BY(mutex_);
    std::ofstream file_ GUARDED_BY(mutex_);
    std::deque<std::string> buffer_ GUARDED_BY(mutex_);

    Logging() = default;
    friend class LogMessage;
  };

  class LogMessage : public std::ostringstream {
  public:
    LogMessage(const char* file, int line);
    ~LogMessage();
  };


  class StderrLogMessage : public std::ostringstream {
  public:
    StderrLogMessage(const char* file, int line);
    ~StderrLogMessage();

  private:
    LogMessage log_;
  };


  std::string FormatTime(std::chrono::time_point<std::chrono::system_clock> time);
  
} // namespace pzero

#define LOGFILE ::pzero::LogMessage(__FILE__, __LINE__)
#define CERR ::pzero::StderrLogMessage(__FILE__, __LINE__)
