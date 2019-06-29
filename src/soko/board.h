#pragma once

#include "soko/bitboard.h"

namespace pzero {
  
  class SokoBoard {
  public:

    SokoBoard() = default;
    SokoBoard(const std::string& fen) { SetFromFen(fen); }

    static const char* kStartposFen;

    void SetFromFen(const std::string& fen, int* moves = nullptr);
  };
  
} // namespace pzero
