
#pragma once

#include <time.h>
#include <string>
#include <vector>

namespace pzero {

// Creates directory at a given path. Throws exception if cannot.
// Returns silently if already exists.
void CreateDirectory(const std::string& path);

// Returns list of full paths of regular files in this directory.
// Silently returns empty vector on error.
std::vector<std::string> GetFileList(const std::string& directory);

// Returns size of a file. Throws exception if file doesn't exist.
uint64_t GetFileSize(const std::string& filename);

// Returns modification time of a file. Throws exception if file doesn't exist.
time_t GetFileTime(const std::string& filename);

}  // namespace pzero
