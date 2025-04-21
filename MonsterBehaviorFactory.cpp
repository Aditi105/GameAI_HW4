// MonsterBehaviorFactory.cpp
#include "MonsterBehaviorFactory.hpp"
#include "SequenceNode.hpp"
#include "SelectorNode.hpp"
#include "MonsterTasks.hpp"

BTNode* MonsterBehaviorFactory::buildTree(
    const sf::Vector2f& monStart,
    const sf::Vector2f& plyStart,
    float eatRadius)
{
    // chase‐then‐reset sequence
    auto* chase    = new ChasePlayerTask();
    auto* reset    = new ResetTask(monStart, plyStart);
    auto* catchSeq = new SequenceNode({ chase, reset });

    // wander fallback
    auto* wander   = new GraphWanderTask();

    // top‐level selector: try catchSeq first, else wander
    return new SelectorNode({ catchSeq, wander });
}
