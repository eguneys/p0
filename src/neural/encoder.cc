#include "neural/encoder.h"
#include <algorithm>


namespace pzero {

  namespace {
    const int kMoveHistory = 8;
    const int kPlanesPerBoard = 13;
    const int kAuxPlaneBase = kPlanesPerBoard * kMoveHistory;
  } // namespace

  InputPlanes EncodePositionForNN(const PositionHistory& history,
                                  int history_planes) {
    InputPlanes result(kAuxPlaneBase + 1);

    {
      const SokoBoard& board = history.Last().GetBoard();
      result[kAuxPlaneBase + 0].SetAll();
    }

    int history_idx = history.GetLength() - 1;
    for (int i = 0; i < std::min(history_planes, kMoveHistory);
         ++i, --history_idx) {
      
      const Position& position = 
        history.GetPositionAt(history_idx < 0 ? 0 : history_idx);

      const SokoBoard& board = position.GetBoard();

      if (history_idx < 0) break;

      const int base = i * kPlanesPerBoard;

      // result[base + 0].mask = board.walls().as_int();
      // result[base + 1].mask = board.targets().as_int();
      // result[base + 2].mask = board.boxes().as_int();
      // result[base + 3].mask = board.king().as_int();

      const int repetitions = position.GetRepetitions();
      if (repetitions >= 1) result[base + 12].SetAll();

    }
    
    return result;
  }

} // namespace pzero
