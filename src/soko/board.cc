#include "soko/board.h"

#include <cstdlib>
#include <cstring>
#include <map>
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

  namespace {

    static const Move::Direction kDirections[4] = {
      Move::Direction::Up,
      Move::Direction::Down,
      Move::Direction::Left,
      Move::Direction::Right 
    };


    static const std::pair<Move::Direction, Move::Direction> kStuckChecks[] =
      {
        {Move::Direction::Up,
         Move::Direction::Left},
        {Move::Direction::Up,
         Move::Direction::Right},
        {Move::Direction::Down,
         Move::Direction::Left},
        {Move::Direction::Down,
         Move::Direction::Right}
      };

    static const std::map<Move::Direction, std::pair<Move::Direction, Move::Direction>> kStuckChecks2 = {
      {Move::Direction::Up, {
          Move::Direction::Left,
          Move::Direction::Right
        }},
      {Move::Direction::Down, {
          Move::Direction::Left,
          Move::Direction::Right
        }},
      {Move::Direction::Left, {
          Move::Direction::Up,
          Move::Direction::Down
        }},
      {Move::Direction::Right, {
          Move::Direction::Up,
          Move::Direction::Down
        }}
    };
    
    static const std::map<Move::Direction, std::pair<int, int>> kCharMoves = {
      {Move::Direction::Up, {1, 0}},
      {Move::Direction::Down, {-1, 0}},
      {Move::Direction::Left, {0, -1}},
      {Move::Direction::Right, {0, 1}}
    };

  } // namespace

  void AddDirection(const BoardSquare& source, BoardSquare& dest, Move::Direction dir) {
    const auto& delta = kCharMoves.at(dir);

    const auto dst_row = source.row() + delta.first;
    const auto dst_col = source.col() + delta.second;
    dest.set(dst_row, dst_col);
  }

  void SokoBoard::ApplyMove(Move move) {
    const auto& from = char_;
    BoardSquare to;
    BoardSquare to2;
    
    AddDirection(from, to, move.direction());
    AddDirection(to, to2, move.direction());

    if (walls_.get(to)) {
      throw Exception("bad move");
    }

    if (boxes_.get(to)) {
      if (walls_.get(to2) || boxes_.get(to2)) {
        throw Exception("bad move");
      }
      // push
      boxes_.reset(to);
      boxes_.set(to2);
    }
    char_ = to;
  }

  bool SokoBoard::IsEnd() const {
    return boxes_.intersects(targets_);
  }

  bool SokoBoard::IsStuck() const {

    for (unsigned square = 0; square < 400; square++) {
      const BoardSquare b_sq(square);

      if (boxes_.get(b_sq)) {
        for (auto check : kStuckChecks) {
          BoardSquare c1;
          BoardSquare c2;
          AddDirection(b_sq, c1, check.first);
          AddDirection(b_sq, c2, check.second);
          if (walls_.get(c1) && walls_.get(c2)) {
            return true;
          }
        }

        for (auto check2 : kStuckChecks2) {
          BoardSquare b2;
          AddDirection(b_sq, b2, check2.first);
          if (boxes_.get(b2)) {
            BoardSquare c1;
            BoardSquare c2;
            BoardSquare c3;
            BoardSquare c4;
            AddDirection(b_sq, c1, check2.second.first);
            AddDirection(b2, c2, check2.second.first);

            AddDirection(b_sq, c3, check2.second.second);
            AddDirection(b2, c4, check2.second.second);

            if ((walls_.get(c1) && walls_.get(c2)) ||
                (walls_.get(c3) && walls_.get(c4))) {
              return true;
            }
          }
        }
      }
    }

    return false;
  }

  MoveList SokoBoard::GenerateLegalMoves() const {
    MoveList result;
    result.reserve(4);

    for (const auto& move : kCharMoves) {
      auto source = char_;
      BoardSquare destination;
      BoardSquare destination2;
      AddDirection(source, destination, move.first);
      AddDirection(destination, destination2, move.first);

      if (walls_.get(destination)) continue;
      if (boxes_.get(destination)) {
        if (boxes_.get(destination2) || walls_.get(destination2)) {
          continue;
        }
      }

      result.emplace_back(move.first);
    }

    return result;
  }

  void SokoBoard::SetFromFen(const std::string& fen, int* moves) {
    
    Clear();

    int row = 19;
    int col = 0;

    std::istringstream fen_str(fen);
    string board;
    // fen_str >> board;
    board = fen;

    for (char c : board) {
      if (c == '\n') {
        --row;
        col = 0;
        continue;
      }

      switch (c) {
      case ' ':
        break;
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

  std::string SokoBoard::DebugString() const {
    string result;

    for (int i = 19; i >= 0; --i) {
      for (int j = 0; j < 20; ++j) {
        if (walls_.get(i, j)) {
          result += '#';
          continue;
        }
        if (targets_.get(i, j)) {
          if (boxes_.get(i, j)) {
            result += '*';
            continue;
          } else if (char_ == i * 20 + j) {
            result += 'o';
            continue;
          } else {
            result += '.';
            continue;
          }
        }

        if (char_ == i * 20 + j) {
          result += '@';
          continue;
        }

        if (boxes_.get(i, j)) {
          result += '$';
          continue;
        }

        else result += ' ';
      }
      result += '\n';
    }
    return result;
  }

} // namespace pzero
