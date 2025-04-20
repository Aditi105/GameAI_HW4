#include "ActionNode.hpp"

ActionNode::ActionNode(BehaviorType behavior)
  : behavior_(behavior)
{}

// **Definitions go here, once each**
DecisionNode* ActionNode::evaluate(const State& /*state*/) {
    return this;
}

BehaviorType ActionNode::getBehavior() const {
    return behavior_;
}
