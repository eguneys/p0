#pragma once

#include <memory>
#include <mutex>
#include "soko/board.h"
#include "soko/position.h"
#include "neural/encoder.h"
#include "neural/writer.h"
#include "utils/mutex.h"

namespace pzero {
  
  class Node;
  class Edge {
  public:
    Move GetMove() const;

    float GetP() const;
    void SetP(float val);

  private:
    void SetMove(Move move) { move_ = move; }

    Move move_;

    uint16_t p_ = 0;
    
    friend class EdgeList;
  };

  class EdgeList {
  public:
    EdgeList() {}
    EdgeList(MoveList moves);
    Edge* get() const { return edges_.get(); }
    Edge& operator[](size_t idx) const { return edges_[idx]; }
    operator bool() const { return static_cast<bool>(edges_); }
    uint16_t size() const { return size_; }

  private:
    std::unique_ptr<Edge[]> edges_;
    uint16_t size_ = 0;
  };

  class EdgeAndNode;

  template <bool is_const>
    class Edge_Iterator;

  class Node {
  public:
    using Iterator = Edge_Iterator<false>;
    using ConstIterator = Edge_Iterator<true>;

  Node(Node* parent, uint16_t index) 
    : parent_(parent), index_(index) {}

    Node* CreateSingleChildNode(Move m);

    void CreateEdges(const MoveList& moves);

    Node* GetParent() const { return parent_; }

    bool HasChildren() const { return edges_; }

    uint32_t GetN() const { return n_; }
    uint32_t GetNInFlight() const { return n_in_flight_; }
    uint32_t GetChildrenVisits() const { return n_ > 0 ? n_ - 1 : 0; }

    int GetNStarted() const { return n_ + n_in_flight_; }

    float GetQ() const { return q_; }

    bool IsTerminal() const { return is_terminal_; }

    uint16_t GetNumEdges() const { return edges_.size(); }

    void MakeTerminal(GameResult result);

    bool TryStartScoreUpdate();

    void CancelScoreUpdate(int multivisit);

    void FinalizeScoreUpdate(float v, int multivisit);

    void IncrementNInFlight(int multivisit) { n_in_flight_ += multivisit; }

    V4TrainingData GetV4TrainingData(GameResult result,
                                     const PositionHistory& history,
                                     float best_q) const;

    ConstIterator Edges() const;
    Iterator Edges();

    Edge* GetEdgeToNode(const Node* node) const;

  private:

    EdgeList edges_;

    Node* parent_ = nullptr;
    
    std::unique_ptr<Node> child_;

    std::unique_ptr<Node> sibling_;

    float q_ = 0.0f;

    uint32_t n_ = 0;
    
    uint32_t n_in_flight_ = 0;

    uint16_t index_;

    bool is_terminal_ = false;

    friend class NodeTree;
    friend class Edge_Iterator<true>;
    friend class Edge_Iterator<false>;
    friend class Edge;
  };

  class EdgeAndNode {
  public:
    EdgeAndNode() = default;
  EdgeAndNode(Edge* edge, Node* node)
    : edge_(edge), node_(node) {}

    void Reset() { edge_ = nullptr; }

    bool operator==(const EdgeAndNode& other) const {
      return edge_ == other.edge_;
    }

    bool operator!=(const EdgeAndNode& other) const {
      return edge_ != other.edge_;
    }

    bool operator<(const EdgeAndNode& other) const {
      return edge_ < other.edge_;
    }

    Edge* edge() const { return edge_; }
    Node* node() const { return node_; }


    float GetQ(float default_q) const {
      return (node_ && node_->GetN() > 0) ? node_->GetQ() : default_q;
    }

    uint32_t GetN() const { return node_ ? node_->GetN() : 0; }

    int GetNStarted() const { return node_ ? node_->GetNStarted() : 0; }

    bool IsTerminal() const { return node_ ? node_->IsTerminal(): false; }


    float GetP() const { return edge_->GetP(); }
    Move GetMove() const {
      return edge_ ? edge_->GetMove() : Move();
    }

    float GetU(float numerator) const {
      return numerator * GetP() / (1 + GetNStarted());
    }

  protected:
    Edge* edge_ = nullptr;

    Node* node_ = nullptr;
  };

  template <bool is_const>
    class Edge_Iterator : public EdgeAndNode {
  public:
    using Ptr = std::conditional_t<is_const, const std::unique_ptr<Node>*,
      std::unique_ptr<Node>*>;

    Edge_Iterator() {}

    Edge_Iterator(const EdgeList& edges, Ptr node_ptr)
      : EdgeAndNode(edges.size() ? edges.get() : nullptr, nullptr),
      node_ptr_(node_ptr),
      total_count_(edges.size()) {
      if (edge_) Actualize();
    }

    Edge_Iterator<is_const> begin() { return *this; }
    Edge_Iterator<is_const> end() { return {}; }

    void operator++() {
      if (++current_idx_ == total_count_) {
        edge_ = nullptr;
      } else {
        ++edge_;
        Actualize();
      }
    }

    Edge_Iterator& operator*() { return *this; }

    Node* GetOrSpawnNode(Node* parent) {
      
      if (node_) return node_;
      Actualize();
      if (node_) return node_;

      std::unique_ptr<Node> tmp = std::move(*node_ptr_);

      *node_ptr_ = std::make_unique<Node>(parent, current_idx_);

      (*node_ptr_)->sibling_ = std::move(tmp);
      Actualize();
      return node_;      
    }

  private:
    void Actualize() {
      while (*node_ptr_ && (*node_ptr_)->index_ < current_idx_) {
        node_ptr_ = &(*node_ptr_)->sibling_;
      }

      if (*node_ptr_ && (*node_ptr_)->index_ == current_idx_) {
        node_ = (*node_ptr_).get();
        node_ptr_ = &node_->sibling_;
      } else {
        node_ = nullptr;
      }
    }

    Ptr node_ptr_;
    uint16_t current_idx_ = 0;
    uint16_t total_count_ = 0;
  };

  class NodeTree {
  public:
    ~NodeTree() { DeallocateTree(); }

    void MakeMove(Move move);

    void TrimTreeAtHead();

    bool ResetToPosition(const std::string& starting_fen,
                         const std::vector<Move>& moves);

    Node* GetCurrentHead() const { return current_head_; }
    Node* GetGameBeginNode() const { return gamebegin_node_.get(); }
    const PositionHistory& GetPositionHistory() const { return history_; }

  private:
    void DeallocateTree();

    Node* current_head_ = nullptr;
    std::unique_ptr<Node> gamebegin_node_;
    PositionHistory history_;
  };
  
} // namespace pzero
