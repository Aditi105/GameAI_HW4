#pragma once
#include "BTNode.hpp"

class SequenceNode : public BTNode {
public:
    SequenceNode(const std::vector<BTNode*>& children);
    Status tick(WorldState& w, float dt) override;
private:
    std::vector<BTNode*> children_;
    int current_ = 0;
};
