#pragma once

#include "soko/bitboard.h"

namespace pzero {
  
  class SokoBoard {
  public:

    SokoBoard() = default;
    SokoBoard(const std::string& fen) { SetFromFen(fen); }

    static const char* kStartposFen;

    void SetFromFen(const std::string& fen, int* moves = nullptr);

    void Clear();

    void ApplyMove(Move move);

    bool IsEnd() const;
    bool IsStuck() const;

    MoveList GenerateLegalMoves() const;

    bool operator==(const SokoBoard& other) const {
      return (walls_ == other.walls_) &&
      (targets_ == other.targets_) && 
      (boxes_ == other.boxes_) && 
      (char_ == other.char_);
    }

    bool operator!=(const SokoBoard& other) const { return !operator==(other); }

    std::string DebugString() const;

  private:
    BitBoard walls_;
    BitBoard targets_;
    BitBoard boxes_;
    BoardSquare char_;
  };
  
} // namespace pzero
