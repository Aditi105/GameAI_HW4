#include "BehaviorTreeFactory.hpp"
#include "ConditionNode.hpp"
#include "ActionNode.hpp"
#include "State.hpp"

namespace BehaviorTreeFactory {

DecisionNode* buildBehaviorTree() {
    // Leafs
    auto* pickWP   = new ActionNode(BehaviorType::PickNewWaypoint);
    auto* pathfind = new ActionNode(BehaviorType::Pathfind);

    // If we’re at our node-target → pick a brand-new waypoint
    // else → keep following the current path
    DecisionNode* root = new ConditionNode(
        [](const State& s){ return s.atTarget(); },
        pickWP,
        pathfind
    );

    return root;
}

} // namespace BehaviorTreeFactory
