#include <gtest/gtest.h>

#include "src/neural/encoder.h"

namespace pzero {

    // "    #####\n"
    // "    #   #\n"
    // "    #$  #\n"
    // "  ###  $##\n"
    // "  #  $ $ #\n"
    // "### # ## #   ######\n"
    // "#   # ## #####  ..#\n"
    // "# $  $          ..#\n"
    // "##### ### #@##  ..#\n"
    // "    #     #########\n"
    // "    #######\n";

  std::uint64_t MakeMask(std::string board, int idx, char t) {
    int row = 19;
    std::string res;

    for (char c : board) {
      if (row == (idx * 2) || row == (idx * 2) + 1) {
        res = res + c;
      }
      if (c == '\n') {
        row--;
      }
    }

    std::uint64_t mask = 0;

    int pos = 20;
    for (char c : res) {
      if (c == t) {
        mask |= 1ull << pos;
      }

      if (c == '\n') {
        pos = 0;
      } else {
        pos++;
      }
    }

    // std::cerr << res << idx << " " << mask;

    return mask;
  }

  TEST(EncodePositionForNN, EncodeStartPosition) {
    
    SokoBoard board;
    PositionHistory history;
    board.SetFromFen(SokoBoard::kStartposFen);
    history.Reset(board);
    
    InputPlanes encoded_planes = 
      EncodePositionForNN(history, 8);

    auto fen = SokoBoard::kStartposFen;

    for (int i = 0; i < 10; i++) {
      InputPlane walls_plane = encoded_planes[0 * 10 + i];
      InputPlane targets_plane = encoded_planes[1 * 10 + i];
      InputPlane boxes_plane = encoded_planes[2 * 10 + i];
    
      auto walls_mask = MakeMask(fen, i, '#');
      auto targets_mask = MakeMask(fen, i, '.');
      auto boxes_mask = MakeMask(fen, i, '$');

      EXPECT_EQ(walls_plane.mask, walls_mask);
      EXPECT_EQ(walls_plane.value, 1.0f);

      EXPECT_EQ(targets_plane.mask, targets_mask);
      EXPECT_EQ(targets_plane.value, 1.0f);

      EXPECT_EQ(boxes_plane.mask, boxes_mask);
      EXPECT_EQ(boxes_plane.value, 1.0f);
    }

    InputPlane king_plane = encoded_planes[3 * 10];
    
    EXPECT_EQ(king_plane.mask, (20 * 11 + 11));
    
  }
  
} // namespace pzero

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
