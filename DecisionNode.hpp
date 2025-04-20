// DecisionNode.hpp
#pragma once

#include "State.hpp"

// Base class: every node in the tree implements evaluate().
class DecisionNode {
public:
    virtual ~DecisionNode() = default;
    // Given the current State, returns the next node (could be itself if leaf)
    virtual DecisionNode* evaluate(const State& state) = 0;
};
