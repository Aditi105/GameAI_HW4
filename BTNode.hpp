#pragma once
#include "Steering.hpp"     // for Kinematic
#include "Node.hpp"         // for graphNodes if you need A*
#include <vector>
#include <SFML/Graphics.hpp> // for sf::RectangleShape

enum class Status { Success, Failure, Running };

struct WorldState {
    Kinematic*                                monster;
    Kinematic*                                player;
    const std::vector<Node>*                 graphNodes;
    const std::vector<sf::RectangleShape>*    walls;
    float                                     eatRadius;
    std::string                               lastAction;
};

struct BTNode {
    virtual ~BTNode() {}
    virtual Status tick(WorldState& w, float dt) = 0;
};
