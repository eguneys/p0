#pragma once

#include <bitset>
#include <cassert>
#include <string>
#include <vector>

namespace pzero {

  class BoardSquare {
  public:
    constexpr BoardSquare() {}

    constexpr BoardSquare(std::uint16_t num) : square_(num) {}
    
    constexpr BoardSquare(int row, int col) : BoardSquare(row * 20 + col) {}

    constexpr std::uint16_t as_int() const { return square_; }

    void set(int row, int col) { square_ = row * 20 + col; }

    int row() const { return square_ / 20; }
    int col() const { return square_ % 20; }

    constexpr bool operator==(const BoardSquare& other) const {
      return square_ == other.square_;
    }

  private:
    std::uint16_t square_ = 0;
  };

  class BitBoard {
  public:
    BitBoard() = default;
    BitBoard(const BitBoard&) = default;

    unsigned long as_ulong() const { return board_.to_ulong(); }
    void clear() { board_ = 0; }

    void set(BoardSquare square) { set(square.as_int()); }
    void set(std::uint16_t pos) { board_.set(pos); }
    void set(int row, int col) { set(BoardSquare(row, col)); }
    
    void reset(BoardSquare square) { reset(square.as_int()); }
    void reset(std::uint16_t pos) { board_.reset(pos); }
    void reset(int row, int col) { reset(BoardSquare(row, col)); }

    bool get(BoardSquare square) const { return get(square.as_int()); }
    bool get(std::uint16_t pos) const {
      return board_.test(pos);
    }
    bool get(int row, int col) const { return get(BoardSquare(row, col)); }

    bool empty() const { return board_.count() == 0; }

    bool intersects(const BitBoard& other) const { return board_.to_ulong() & other.board_.to_ulong(); }

    bool operator==(const BitBoard& other) const {
      return board_.to_string() == other.board_.to_string();
    }
    bool operator!=(const BitBoard& other) const {
      return board_.to_string() != other.board_.to_string();
    }

  private:
    std::bitset<400> board_;
  };
  
  class Move {
  public:
    enum class Direction : std::uint8_t { Up, Down, Left, Right };

    Move() = default;
    Move(const std::string& str);
    Move(const char* str) : Move(std::string(str)) {};
    Move(const Move::Direction dir) {
      SetDirection(dir);
    };

    Direction direction() const { return Direction(data_); }

    void SetDirection(Direction dir) {
      data_ = static_cast<uint8_t>(dir);
    }

    uint8_t as_packed_int() const;

    uint8_t as_nn_index() const;

    bool operator==(const Move& other) const {
      return data_ == other.data_;
    }

    std::string as_string() const {
      switch (direction()) {
      case Direction::Up:
        return "up";
      case Direction::Down:
        return "down";
      case Direction::Left:
        return "left";
      case Direction::Right:
        return "right";
      }
      assert(false);
      return "Error!";
    }

  private:
    uint8_t data_ = 0;
  };


  using MoveList = std::vector<Move>;
} // namespace pzero
