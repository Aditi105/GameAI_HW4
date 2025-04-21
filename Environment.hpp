// Environment.hpp
#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include "Node.hpp"

// global nav‐mesh storage (instantiate in Environment.cpp)
extern std::vector<Node> graphNodes;

// build & probe the four‐room layout
void drawSymmetricRoomLayout(std::vector<sf::RectangleShape>& walls);
void createGraphGrid(std::vector<Node>& graphNodes,
                     const std::vector<sf::RectangleShape>& walls,
                     int spacing, int width, int height);
bool isInsideWall(const sf::Vector2f& pos,
                  const std::vector<sf::RectangleShape>& walls);
