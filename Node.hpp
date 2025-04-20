#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

/// Shared graph‐node type
struct Node {
    sf::Vector2f           position;
    std::vector<int>       neighbors;
};
