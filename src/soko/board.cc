#include "soko/board.h"

#include <cstdlib>
#include <cstring>
#include "utils/exception.h"

namespace pzero {

  using std::string;

  const char* SokoBoard::kStartposFen =
    "    #####\n"
    "    #   #\n"
    "    #$  #\n"
    "  ###  $##\n"
    "  #  $ $ #\n"
    "### # ## #   ######\n"
    "#   # ## #####  ..#\n"
    "# $  $          ..#\n"
    "##### ### #@##  ..#\n"
    "    #     #########\n"
    "    #######\n";


  void SokoBoard::Clear() {
    std::memset(reinterpret_cast<void*>(this), 0, sizeof(SokoBoard));
  }

  bool SokoBoard::ApplyMove(Move move) {
    
  }

  MoveList SokoBoard::GenerateLegalMoves() const {
    
  }

  void SokoBoard::SetFromFen(const std::string& fen, int* moves) {
    
    Clear();

    int row = 19;
    int col = 0;

    std::istringstream fen_str(fen);
    string board;
    fen_str >> board;

    for (char c : board) {
      if (c == '\n') {
        --row;
        col = 0;
        continue;
      }

      switch (c) {
      case '#':
        walls_.set(row, col);
        break;
      case '.':
        targets_.set(row, col);
        break;
      case '$':
        boxes_.set(row, col);
        break;
      case '*':
        boxes_.set(row, col);
        targets_.set(row, col);
        break;
      case '@':
        char_.set(row, col);
        break;
      case 'o':
        char_.set(row, col);
        targets_.set(row, col);
        break;
      default:
        throw Exception("Bad fen string: " + fen);
      }
      col++;
    }
  }

} // namespace pzero
