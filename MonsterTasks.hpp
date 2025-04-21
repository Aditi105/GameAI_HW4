#pragma once

#include "BTNode.hpp"
#include "Node.hpp"
#include "Steering.hpp"
#include <vector>

// ——— Reset —————————————————————————————————————————
struct ResetTask : public BTNode {
    ResetTask(const sf::Vector2f& monStart,
              const sf::Vector2f& plyStart);
    virtual Status tick(WorldState& w, float dt) override;
private:
    sf::Vector2f monStart_, plyStart_;
    bool         done_;
};

// ——— Chase (with path‐follow) ——————————————————————————————————
struct ChasePlayerTask : public BTNode {
    ChasePlayerTask();

    // returns Running while chasing, Success on “eat”, Failure if too far
    virtual Status tick(WorldState& w, float dt) override;

private:
    float              aggroRange_;  // beyond this → Failure → wander
    float              pathRange_;   // within this → switch into path‑follow
    std::vector<int>   path_;
    int                pathIdx_;
};

// ——— Graph Wander —————————————————————————————————————
struct GraphWanderTask : public BTNode {
    GraphWanderTask();
    virtual Status tick(WorldState& w, float dt) override;
private:
    std::vector<int> path_;
    int              pathIdx_;
};
