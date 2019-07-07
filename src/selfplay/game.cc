#include "selfplay/game.h"
#include <algorithm>

namespace pzero {
  
  SelfPlayGame::SelfPlayGame(PlayerOptions player)
    : options_(player) {
    tree_ = std::make_shared<NodeTree>();
    tree_->ResetToPosition(SokoBoard::kStartposFen, {});
  }

  void SelfPlayGame::Play(int threads, bool training) {
    while (!abort_) {
      game_result_ = tree_->GetPositionHistory().ComputeGameResult();

      CERR << tree_->GetPositionHistory().Last().GetBoard().DebugString();

      if (game_result_ != GameResult::UNDECIDED) break;
      
      tree_->TrimTreeAtHead();

      if (options_.search_limits.movetime > -1) {
        options_.search_limits.search_deadline =
          std::chrono::steady_clock::now() +
          std::chrono::milliseconds(options_.search_limits.movetime);
      }

      {
        std::lock_guard<std::mutex> lock(mutex_);
        if (abort_) break;
        search_ = std::make_unique<Search>
          (*tree_, options_.network, options_.best_move_callback,
           options_.search_limits,
           options_.uci_options);
      }

      search_->RunBlocking(threads);

      if (abort_) break;

      auto best_eval = search_->GetBestEval();
      if (training) {
        auto best_q = best_eval;
        training_data_.push_back
          (tree_->GetCurrentHead()->GetV4TrainingData
           (GameResult::UNDECIDED,
            tree_->GetPositionHistory(),
            best_q));
      }

      if (best_eval == 1) {
        game_result_ = GameResult::WIN;
        break;
      } else if (best_eval == -1) {
        game_result_ = GameResult::LOSE;
        break;
      }

      const Move move = search_->GetBestMove();

      tree_->MakeMove(move);
    }
  }

  std::vector<Move> SelfPlayGame::GetMoves() const {
    std::vector<Move> moves;

    for (Node* node = tree_->GetCurrentHead();
         node != tree_->GetGameBeginNode();
         node = node->GetParent()) {
      moves.push_back(node->GetParent()->GetEdgeToNode(node)->GetMove());
    }
    std::reverse(moves.begin(), moves.end());
    return moves;
  }

  void SelfPlayGame::Abort() {
    std::lock_guard<std::mutex> lock(mutex_);
    abort_ = true;
    if (search_) search_->Abort();
  }
  

  void SelfPlayGame::WriteTrainingData(TrainingDataWriter* writer) const {
    assert(!training_data_.empty());

    for (auto chunk : training_data_) {
      if (game_result_ == GameResult::WIN) {
        chunk.result = 1;
      } else if (game_result_ == GameResult::LOSE) {
        chunk.result = -1;
      } else {
        chunk.result = 0;
      }
      writer->WriteChunk(chunk);
    }
  }
}
