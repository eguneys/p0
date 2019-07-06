#include "neural/encoder.h"
#include <algorithm>


namespace pzero {

  namespace {
    const int kMoveHistory = 8;
    const int kPlanesPerBoard = 32;
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

      std::uint64_t walls[10];
      std::uint64_t targets[10];
      std::uint64_t boxes[10];


      board.walls().as_int_array(walls);
      board.targets().as_int_array(targets);
      board.boxes().as_int_array(boxes);

      for (int i = 0; i < 10; i++) {
        result[base + 0 * 10 + i].mask = walls[i];
        result[base + 1 * 10 + i].mask = targets[i];
        result[base + 2 * 10 + i].mask = boxes[i];
      }
      result[base + 3 * 10].mask = board.king().as_int();

      const int repetitions = position.GetRepetitions();
      if (repetitions >= 1) result[base + 3 * 10 + 1].SetAll();

    }
    
    return result;
  }

} // namespace pzero
