#include "SequenceNode.hpp"

SequenceNode::SequenceNode(const std::vector<BTNode*>& children)
  : children_(children)
{}

Status SequenceNode::tick(WorldState& w, float dt) {
    for (auto* child : children_) {
        Status s = child->tick(w, dt);
        if (s == Status::Running || s == Status::Failure)
            return s;
    }
    return Status::Success;
}
