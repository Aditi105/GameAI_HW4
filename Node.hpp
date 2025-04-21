// Node.hpp
#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

struct Node {
    sf::Vector2f position;
    std::vector<int> neighbors;
};

// nav‐mesh storage
extern std::vector<Node> graphNodes;

// pathfinding helpers
int getClosestNode(const sf::Vector2f& pos);
std::vector<int> AStar(int startIndx, int goalIndx);
