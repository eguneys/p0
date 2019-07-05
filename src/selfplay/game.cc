#include "selfplay/game.h"
#include <algorithm>

namespace pzero {
  
  SelfPlayGame::SelfPlayGame(PlayerOptions player)
    : options_(player) {
    tree_ = std::make_shared<NodeTree>();
    tree_->ResetToPosition(SokoBoard::kStartposFen, {});
  }

  void SelfPlayGame::Play(int threads, bool training) {
    
  }

  std::vector<Move> SelfPlayGame::GetMoves() const {
    std::vector<Move> moves;
    return moves;
  }

  void SelfPlayGame::Abort() {
    std::lock_guard<std::mutex> lock(mutex_);
    abort_ = true;
    if (search_) search_->Abort();
  }
  
}
