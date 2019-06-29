#pragma once

#include <memory>
#include <mutex>
#include "soko/board.h"
#include "soko/position.h"
#include "neural/encoder.h"
#include "utils/mutex.h"

namespace pzero {
  
  class Node;
  class Edge {
  };

  class EdgeList {
  };

  class Node {
  public:

    bool IsTerminal() const { return is_terminal_; }

  private:

    bool is_terminal_ = false;
  };

  class EdgeAndNode {
  };

  template <bool is_const>
    class Edge_Iterator : public EdgeAndNode {
  };

  class NodeTree {
  public:
    ~NodeTree() { DeallocateTree(); }

    Node* GetCurrentHead() const { return current_head_; }
    const PositionHistory& GetPositionHistory() const { return history_; }

  private:
    void DeallocateTree();

    Node* current_head_ = nullptr;
    std::unique_ptr<Node> gamebegin_node_;
    PositionHistory history_;
  };
  
} // namespace pzero
