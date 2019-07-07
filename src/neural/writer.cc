#include "neural/writer.h"

#include <iomanip>
#include <sstream>
#include "utils/commandline.h"
#include "utils/exception.h"
#include "utils/filesystem.h"
#include "utils/random.h"

namespace pzero {
  
  TrainingDataWriter::TrainingDataWriter(int game_id) {
    static std::string directory =
      CommandLine::BinaryDirectory() + "/data-" + Random::Get().GetString(12);

    CreateDirectory(directory.c_str());

    std::ostringstream oss;
    oss << directory << '/' << "game_" << std::setfill('0') << std::setw(6) << game_id << ".gz";

    filename_ = oss.str();
    fout_ = gzopen(filename_.c_str(), "wb");

    if (!fout_) throw Exception("Cannot create gzip file " + filename_);    
  }

  void TrainingDataWriter::WriteChunk(const V4TrainingData& data) {
    auto bytes_written =
      gzwrite(fout_, reinterpret_cast<const char*>(&data), sizeof(data));
    if (bytes_written != sizeof(data)) {
      throw Exception("Unable to write into " + filename_);
    }
  }

  void TrainingDataWriter::Finalize() {
    gzclose(fout_);
    fout_ = nullptr;
  }
  
} // namespace pzero
