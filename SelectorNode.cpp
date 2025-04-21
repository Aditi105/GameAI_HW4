#include "SelectorNode.hpp"
SelectorNode::SelectorNode(const std::vector<BTNode*>& c)
  : children_(c) {}

Status SelectorNode::tick(WorldState& w, float dt) {
    while (current_ < (int)children_.size()) {
        Status s = children_[current_]->tick(w, dt);
        if (s == Status::Running) return Status::Running;
        if (s == Status::Success) {
            current_ = 0;
            return Status::Success;
        }
        // Failure → next
        ++current_;
    }
    current_ = 0;
    return Status::Failure;
}
