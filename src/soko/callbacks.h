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

  struct GameInfo {
    GameResult game_result = GameResult::UNDECIDED;

    std::string training_filename;
    
    std::vector<Move> moves;

    int game_id = -1;

    using Callback = std::function<void(const GameInfo&)>;
  };

  struct TournamentInfo {
    bool finished = false;

    // [win/lose]
    int results[2] = { 0, 0 };

    using Callback = std::function<void(const TournamentInfo&)>;
  };
  
} // namespace pzero
