#include <gtest/gtest.h>

#include <iostream>
#include "src/soko/bitboard.h"
#include "src/soko/board.h"

namespace pzero {
  
  TEST(BoardSquare, BoardSquare) {
    {
      auto x = BoardSquare(19);
      EXPECT_EQ(x.row(), 0);
      EXPECT_EQ(x.col(), 19);
    }

    {
      auto x = BoardSquare(20);
      EXPECT_EQ(x.row(), 1);
      EXPECT_EQ(x.col(), 0);
    }

    {
      auto x = BoardSquare(1, 2);
      EXPECT_EQ(x.row(), 1);
      EXPECT_EQ(x.col(), 2);
    }
  }


  TEST(SokoBoard, LegalMovesStartingPos) {
    SokoBoard board;
    board.SetFromFen(SokoBoard::kStartposFen);
    auto moves = board.GenerateLegalMoves();

    MoveList expected = {"up"};

    EXPECT_EQ(moves, expected);
  }

  namespace {

    void PlayMoves(SokoBoard& board_seq, const std::string& moves) {
      std::istringstream moves_str(moves);
      std::string move_str;

      while (moves_str >> move_str) {
        auto new_board = board_seq;
        const auto& move = Move(move_str);
        new_board.ApplyMove(move);
        board_seq = new_board;
      }
    }

  } // namespace

  TEST(SokoBoard, ApplyMoveStartingPos) {
    SokoBoard board;
    board.SetFromFen(SokoBoard::kStartposFen);

    SokoBoard board2 = board;
    PlayMoves(board2, "up");
    auto moves = board2.GenerateLegalMoves();
    MoveList expected = {"down", "left", "right"};
    EXPECT_EQ(moves, expected);

    SokoBoard board3 = board;
    PlayMoves(board3, "up right right right right right right");
    moves = board3.GenerateLegalMoves();
    expected = {"up", "down", "left"};
    EXPECT_EQ(moves, expected);

    // std::cerr << board3.DebugString();

    SokoBoard board4 = board;
    PlayMoves(board4, "up left left left left left left");
    moves = board4.GenerateLegalMoves();
    expected = {"up", "down", "left", "right"};
    EXPECT_EQ(moves, expected);
    
    // std::cerr << board4.DebugString();

    SokoBoard board5 = board;
    PlayMoves(board5, "up left left left left left left left");
    moves = board5.GenerateLegalMoves();
    expected = {"right"};
    EXPECT_EQ(moves, expected);
    
    std::cerr << board5.DebugString();
    
  }
  
  
} // namespace pzero

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
