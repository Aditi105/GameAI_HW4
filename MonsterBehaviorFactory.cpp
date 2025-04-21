#include "MonsterBehaviorFactory.hpp"
#include "SequenceNode.hpp"
#include "SelectorNode.hpp"
#include "RandomSelectorNode.hpp"
#include "MonsterTasks.hpp"

BTNode* MonsterBehaviorFactory::buildTree(
    const sf::Vector2f& monStart,
    const sf::Vector2f& plyStart,
    float eatRadius)
{
    // Leaves
    auto* chase  = new ChasePlayerTask();

    auto* reset  = new ResetTask(monStart, plyStart);

    // Sequence: chase until Success → then reset
    auto* catchSeq = new SequenceNode({ chase, reset });
    auto* wander = new GraphWanderTask();

    // Random idle: wander
    //auto* roam     = new RandomSelectorNode({ wander });

    // Top selector: if catchSeq returns Success (ate+reset), do it; else roam
    return new SelectorNode({ catchSeq, wander });
}
