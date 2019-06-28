#pragma once

#include <string>
#include <vector>

namespace pzero {
  
  class Move {
  public:
    enum class Direction : std::uint8_t { Up, Down, Left, Right };

    Move() = default;
    Move(const std::string& str);
    Move(const char* str) : Move(std::string(str)) {};

    Direction direction() const { return Direction(data_); }

    void SetDirection(Direction dir) {
      data_ = static_cast<uint8_t>(dir);
    }

    uint8_t as_packed_int() const;

    uint8_t as_nn_index() const;

  private:
    uint8_t data_ = 0;
  };
  
} // namespace pzero
