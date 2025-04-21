#include "SequenceNode.hpp"
SequenceNode::SequenceNode(const std::vector<BTNode*>& c)
  : children_(c) {}

Status SequenceNode::tick(WorldState& w, float dt) {
    while (current_ < (int)children_.size()) {
        Status s = children_[current_]->tick(w, dt);
        if (s == Status::Running) return Status::Running;
        if (s == Status::Failure) {
            current_ = 0;
            return Status::Failure;
        }
        // Success → next child
        ++current_;
    }
    current_ = 0;
    return Status::Success;
}
