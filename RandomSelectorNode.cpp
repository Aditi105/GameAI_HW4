#include "RandomSelectorNode.hpp"
RandomSelectorNode::RandomSelectorNode(const std::vector<BTNode*>& c)
  : children_(c) {}

Status RandomSelectorNode::tick(WorldState& w, float dt) {
    if (!chosen_) {
        index_ = std::rand() % children_.size();
        chosen_ = true;
    }
    Status s = children_[index_]->tick(w, dt);
    if (s != Status::Running)
        chosen_ = false;
    return s;
}
