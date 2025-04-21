#pragma once
#include "BTNode.hpp"

namespace MonsterBehaviorFactory {
    BTNode* buildTree(const sf::Vector2f& monStart,
                      const sf::Vector2f& plyStart,
                      float eatRadius);
}
