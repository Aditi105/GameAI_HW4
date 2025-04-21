// Environment.hpp
#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include "Node.hpp"

// global nav‐mesh
extern std::vector<Node> graphNodes;

// draw the red room & walls
void drawSymmetricRoomLayout(std::vector<sf::RectangleShape>& walls);

// build the A* node grid around those walls
void createGraphGrid(std::vector<Node>& graphNodes,
                     const std::vector<sf::RectangleShape>& walls,
                     int spacing, int width, int height);

// helper for probing walls
bool isInsideWall(const sf::Vector2f& pos,
                  const std::vector<sf::RectangleShape>& walls);
