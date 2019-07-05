#include "soko/position.h"
#include <cassert>

namespace pzero {
  
  Position::Position(const Position& parent, Move m) {
    board_ = parent.board_;
    board_.ApplyMove(m);
  }

  Position::Position(const SokoBoard& board)
    : repetitions_(0) {
    board_ = board;
  }

  

  void PositionHistory::Reset(const SokoBoard& board) {
    positions_.clear();
    positions_.emplace_back(board);
  }

  void PositionHistory::Append(Move m) {
    positions_.push_back(Position(Last(), m));
    positions_.back().SetRepetitions(ComputeLastMoveRepetitions());
  }

  int PositionHistory::ComputeLastMoveRepetitions() const {
    const auto& last = positions_.back();

    for (int idx = positions_.size() - 1; idx >= 0; idx -= 1) {
      const auto& pos = positions_[idx];
      if (pos.GetBoard() == last.GetBoard()) {
        return 1 + pos.GetRepetitions();
      }
    }
    return 0;
  }
  
} // namespace pzero
