#pragma once

#include <functional>
#include "soko/bitboard.h"
#include "soko/position.h"

namespace pzero {
  
  struct BestMoveInfo {
    BestMoveInfo(Move bestmove)
      : bestmove(bestmove) {}

    Move bestmove;

    using Callback = std::function<void(const BestMoveInfo&)>;
  };

  struct TournamentInfo {
    bool finished = false;

    // [win/lose]
    int results[2] = { 0, 0 };

    using Callback = std::function<void(const TournamentInfo&)>;
  };
  
} // namespace pzero
