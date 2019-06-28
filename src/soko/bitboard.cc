#include "soko/bitboard.h"
#include "utils/exception.h"

namespace pzero {

  namespace {
    const Move kIdxToMove[] = {
      "up", "down", "left", "right"
    };

    std::vector<unsigned short> BuildMoveIndices() {
      std::vector<unsigned short> res(4);
      for (size_t i = 0; i < 4; ++i) {
        res[kIdxToMove[i].as_packed_int()] = i;
      }
    }

    const std::vector<unsigned short> kMoveToIdx = BuildMoveIndices();

  } // namespace

  Move::Move(const std::string& str) {
    switch (str[0]) {
    case 'u':
      SetDirection(Direction::Up);
      break;
    case 'd':
      SetDirection(Direction::Down);
      break;
    case 'l':
      SetDirection(Direction::Left);
      break;
    case 'r':
      SetDirection(Direction::Right);
      break;
    default:
      throw Exception("Bad move: " + str);
    }
  }

  uint8_t Move::as_packed_int() const {
    return static_cast<int>(direction());
  }

  uint8_t Move::as_nn_index() const {
    return kMoveToIdx[as_packed_int()];
  }

} // namespaze pzero
