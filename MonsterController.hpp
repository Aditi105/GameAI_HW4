// MonsterController.hpp
#pragma once

#include "BTNode.hpp"
#include "Node.hpp"
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>


class MonsterController {
public:
    MonsterController(const std::vector<Node>&               graph,
                      const std::vector<sf::RectangleShape>& walls,
                      Kinematic&                             monster,
                      Kinematic&                             player,
                      const sf::Vector2f&                    monStart,
                      const sf::Vector2f&                    plyStart,
                      float                                   eatRadius);

    void update(float dt);

    std::string getLastActionName() const {
        return world_.lastAction;
    }

private:
    WorldState world_;
    BTNode*    root_;
};
