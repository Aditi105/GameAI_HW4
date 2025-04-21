#include "MonsterController.hpp"
#include "MonsterBehaviorFactory.hpp"

MonsterController::MonsterController(
    const std::vector<Node>&               graph,
    const std::vector<sf::RectangleShape>& walls,
    Kinematic&                             monster,
    Kinematic&                             player,
    const sf::Vector2f&                    monStart,
    const sf::Vector2f&                    plyStart,
    float                                  eatRadius)
{
    world_.monster    = &monster;
    world_.player     = &player;
    world_.graphNodes = &graph;      // ← wire up your nav‐mesh
    world_.walls      = &walls;
    world_.eatRadius  = eatRadius;
    world_.lastAction = "";           // ← start with empty action

    root_ = MonsterBehaviorFactory::buildTree(
        monStart, plyStart, eatRadius
    );
}

void MonsterController::update(float dt) {
    root_->tick(world_, dt);
    // world_.lastAction has now been set by the leaf that ran
}
