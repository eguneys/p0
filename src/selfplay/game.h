#pragma once

#include "soko/position.h"
#include "soko/uciloop.h"
#include "mcts/search.h"
#include "neural/network.h"
#include "utils/optionsparser.h"

namespace pzero {

  struct SelfPlayLimits : SearchLimits {
    std::int64_t movetime;
  };

  struct PlayerOptions {
    Network* network;

    BestMoveInfo::Callback best_move_callback;

    const OptionsDict* uci_options;

    SelfPlayLimits search_limits;    
  };

  class SelfPlayGame {
  public:
    SelfPlayGame(PlayerOptions player);

    void Play(int threads, bool training);
    
    void Abort();

    void WriteTrainingData(TrainingDataWriter* writer) const;

    GameResult GetGameResult() const { return game_result_; }
    std::vector<Move> GetMoves() const;

  private:

    PlayerOptions options_;

    std::shared_ptr<NodeTree> tree_;

    std::unique_ptr<Search> search_;

    bool abort_ = false;

    GameResult game_result_ = GameResult::UNDECIDED;

    std::mutex mutex_;
    
    std::vector<V4TrainingData> training_data_;
  };
  
} // namespace pzero
