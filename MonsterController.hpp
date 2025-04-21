#pragma once

#include "BTNode.hpp"
#include "Node.hpp"
#include <SFML/Graphics.hpp>   // for sf::RectangleShape
#include <vector>
#include <string>

class MonsterController {
public:
    // no more 'lastAction' parameter here:
    MonsterController(const std::vector<Node>&               graph,
                      const std::vector<sf::RectangleShape>& walls,
                      Kinematic&                             monster,
                      Kinematic&                             player,
                      const sf::Vector2f&                    monStart,
                      const sf::Vector2f&                    plyStart,
                      float                                   eatRadius);

    // tick the BT one frame
    void update(float dt);

    // after update() you can call this to see which leaf ran last
    std::string getLastActionName() const {
        return world_.lastAction;
    }

private:
    WorldState world_;
    BTNode*    root_;
};
