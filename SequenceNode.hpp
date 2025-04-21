#pragma once
#include "BTNode.hpp"
#include <vector>

class SequenceNode : public BTNode {
public:
    SequenceNode(const std::vector<BTNode*>& children);
    Status tick(WorldState& w, float dt) override;
private:
    std::vector<BTNode*> children_;
};
