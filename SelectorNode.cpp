#include "SelectorNode.hpp"

SelectorNode::SelectorNode(const std::vector<BTNode*>& children)
  : children_(children)
{}

Status SelectorNode::tick(WorldState& w, float dt) {
    for (auto* child : children_) {
        Status s = child->tick(w, dt);
        if (s == Status::Running || s == Status::Success)
            return s;
    }
    return Status::Failure;
}
