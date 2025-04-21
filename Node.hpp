#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

/// Shared graph‐node type
struct Node {
    sf::Vector2f           position;
    std::vector<int>       neighbors;
};

int getClosestNode(const sf::Vector2f& pos);
std::vector<int> AStar(int startIdx, int goalIdx);