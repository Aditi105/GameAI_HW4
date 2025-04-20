#pragma once

#include "DecisionNode.hpp"
#include "State.hpp"        // for completeness, if you reference State

/// All possible behaviors your tree can pick.
enum class BehaviorType {
    Pathfind,
    Wander,
    Flee,
    PickNewWaypoint,
};

/// A leaf in the tree that simply returns itself and carries a BehaviorType.
class ActionNode : public DecisionNode {
public:
    explicit ActionNode(BehaviorType behavior);

    // **Declaration only**—no inline body here!
    DecisionNode* evaluate(const State& state) override;
    BehaviorType   getBehavior() const;

private:
    BehaviorType behavior_;
};
