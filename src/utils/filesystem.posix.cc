
#include "utils/exception.h"
#include "utils/filesystem.h"

#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>

namespace pzero {

  void CreateDirectory(const std::string& path) {
    if (mkdir(path.c_str(), 0777) < 0 && errno != EEXIST) {
      throw Exception("Cannot create directory: " + path);
    }
  }

  std::vector<std::string> GetFileList(const std::string& directory) {
    std::vector<std::string> result;
    DIR* dir = opendir(directory.c_str());
    if (!dir) return result;
    while (auto* entry = readdir(dir)) {
      bool exists = false;
      switch (entry->d_type) {
      case DT_REG:
        exists = true;
        break;
      case DT_LNK:
        // check that the soft link actually points to a regular file.
        const std::string filename = directory + "/" + entry->d_name;
        struct stat s;
        exists =
          stat(filename.c_str(), &s) == 0 && (s.st_mode & S_IFMT) == S_IFREG;
        break;
      }
      if (exists) result.push_back(entry->d_name);
    }
    closedir(dir);
    return result;
  }

  uint64_t GetFileSize(const std::string& filename) {
    struct stat s;
    if (stat(filename.c_str(), &s) < 0) {
      throw Exception("Cannot stat file: " + filename);
    }
    return s.st_size;
  }

  time_t GetFileTime(const std::string& filename) {
    struct stat s;
    if (stat(filename.c_str(), &s) < 0) {
      throw Exception("Cannot stat file: " + filename);
    }
#ifdef __APPLE__
    return s.st_mtimespec.tv_sec;
#else
    return s.st_mtim.tv_sec;
#endif
  }

}  // namespace pzero
