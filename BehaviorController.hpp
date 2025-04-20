// BehaviorController.hpp
#pragma once

#include "DecisionNode.hpp"
#include "State.hpp"
#include "Steering.hpp"
#include "ActionNode.hpp"
#include "BehaviorTreeFactory.hpp"
#include "Node.hpp"
#include <vector>



class BehaviorController {
public:
    BehaviorController(const std::vector<Node>& graphNodes,
                       const std::vector<sf::RectangleShape>& walls);

    /// Call each frame: returns the steering for the chosen behavior.
    SteeringOutput update(Kinematic& character, float deltaTime);
    void initialize(Kinematic& character);

private:
    DecisionNode*              root_;
    BehaviorType               lastBehavior_;
    float                      timeInBehavior_;

    // dependencies
    const std::vector<Node>& graphNodes_;  // now works, Node is complete
    const std::vector<sf::RectangleShape>& walls_;

    // steering instances
    ArriveBehavior arrive_;
    AlignBehavior  align_;
    WanderBehavior wander_;

    // path state
    int               currentWaypoint_;
    std::vector<int>  currentPath_;
    int               currentPathIndex_;

    State             state_;

    void pickNewWaypoint(Kinematic& character);
    float computeMinWallDist(const sf::Vector2f& pos) const;
};
