// MonsterTasks.hpp
#pragma once

#include "BTNode.hpp"
#include "Steering.hpp"
#include "Node.hpp"
#include <SFML/Graphics.hpp>

/// 1) Chase the player until within eatRadius
class ChasePlayerTask : public BTNode {
public:
    Status tick(WorldState& w, float dt) override;
};

/// 2) Wander around when not chasing
class WanderTask : public BTNode {
public:
    Status tick(WorldState& w, float dt) override;
};

/// 3) Once “eaten,” teleport both back and succeed exactly once
class ResetTask : public BTNode {
public:
    ResetTask(const sf::Vector2f& monStart,
              const sf::Vector2f& plyStart);
    Status tick(WorldState& w, float dt) override;

private:
    sf::Vector2f monStart_, plyStart_;
    bool         done_;
};

/// A leaf that wanders *on* the graph: pathfinds to random nodes.
class GraphWanderTask : public BTNode {
public:
    GraphWanderTask()
        : pathIdx_(0)
    {}

    virtual Status tick(WorldState& w, float dt) override;

private:
    std::vector<int> path_;
    int              pathIdx_;
};