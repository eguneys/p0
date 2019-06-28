#pragma once

#include <mutex>
#include "utils/cppattributes.h"

namespace pzero {
  
  class CAPABILITY("mutex") Mutex {
  public:
  
    class SCOPED_CAPABILITY Lock {
    public:
      Lock(Mutex& m) ACQUIRE(m) : lock_(m.get_raw()) {}
      ~Lock() RELEASE() {}
      std::unique_lock<std::mutex>& get_raw() { return lock_; }

    private:
      std::unique_lock<std::mutex> lock_;
    };

    void lock() ACQUIRE() { mutex_.lock(); }
    void unlock() RELEASE() { mutex_.unlock(); }
    std::mutex& get_raw() { return mutex_; }

  private:
    std::mutex mutex_;

  };

}
