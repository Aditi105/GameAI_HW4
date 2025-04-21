#pragma once
#include "BTNode.hpp"
#include <cstdlib>

class RandomSelectorNode : public BTNode {
public:
    RandomSelectorNode(const std::vector<BTNode*>& children);
    Status tick(WorldState& w, float dt) override;
private:
    std::vector<BTNode*> children_;
    bool chosen_ = false;
    int  index_  = 0;
};
