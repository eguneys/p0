#pragma once

#include <string>
#include "soko/board.h"

namespace pzero {
  
  class Position {
  public:
    Position(const Position& parent, Move m);

    Position(const SokoBoard& board);

    int GetRepetitions() const { return repetitions_; }

    void SetRepetitions(int repetitions) { repetitions_ = repetitions; }


    const SokoBoard& GetBoard() const { return board_; }

  private:
    SokoBoard board_;

    int repetitions_;    
  };

  enum class GameResult { UNDECIDED, WIN, LOSE };

  class PositionHistory {
  public:
    PositionHistory() = default;
    PositionHistory(const PositionHistory& other) = default;

    const Position& Last() const { return positions_.back(); }

    const Position& GetPositionAt(int idx) const { return positions_[idx]; }

    void Trim(int size) {
      positions_.erase(positions_.begin() + size, positions_.end());
    }

    int GetLength() const { return positions_.size(); }

    void Reset(const SokoBoard& board);

    void Append(Move m);

    GameResult ComputeGameResult() const;

  private:
    int ComputeLastMoveRepetitions() const;

    std::vector<Position> positions_;
  };
  
} // namespace pzero
