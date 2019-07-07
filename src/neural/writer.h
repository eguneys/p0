#include <zlib.h>
#include <fstream>
#include "utils/cppattributes.h"

#pragma once

namespace pzero {
  
#pragma pack(push, 1)

  struct V4TrainingData {
    uint32_t version;
    float probabilities[4];
    uint64_t planes[256];
    int8_t result;
    float root_q;
    float best_q;
  } PACKED_STRUCT;
  
#pragma pack(pop)

  class TrainingDataWriter {
  public:
    TrainingDataWriter(int game_id);

    TrainingDataWriter() {
      if (fout_) Finalize();
    }

    void WriteChunk(const V4TrainingData& data);

    void Finalize();

    std::string GetFileName() const { return filename_; }

  private:
    std::string filename_;
    gzFile fout_;
  };
  
} // namespace pzero
