// BehaviorController.cpp
#include "BehaviorController.hpp"
#include "ConditionNode.hpp"
#include "ActionNode.hpp"
#include "Node.hpp"
#include <cstdlib>    // for rand()
#include <limits>
#include <cmath>

// externs from your part4.cpp
extern int getClosestNode(sf::Vector2f);
extern std::vector<int> AStar(int, int);

BehaviorController::BehaviorController(const std::vector<Node>& graph,
                                       const std::vector<sf::RectangleShape>& walls)
  : root_(BehaviorTreeFactory::buildBehaviorTree())
  , lastBehavior_(BehaviorType::PickNewWaypoint)
  , timeInBehavior_(0.f)
  , graphNodes_(graph)
  , walls_(walls)
  // tune these params as you like
  , arrive_(250.f, 300.f, 15.f, 300.f, 0.3f)
  , align_(200.f, PI * 3, 0.02f, 2.0f, 0.1f)
  , wander_(150.f, 150.f, 50.f, 30.f, 10.f, 0.4f)
  , currentWaypoint_(-1)
  , currentPathIndex_(0)
{
    // pick an initial waypoint so we have a path to follow immediately
    //pickNewWaypoint(*reinterpret_cast<Kinematic*>(nullptr)); 
    // (you'll actually call pickNewWaypoint in main once character exists)
}

SteeringOutput BehaviorController::update(Kinematic& character, float dt) {
    // 1) Update State
    state_.distanceToTarget = std::hypot(
        character.position.x - graphNodes_[currentWaypoint_].position.x,
        character.position.y - graphNodes_[currentWaypoint_].position.y
    );
    state_.speed         = vectorLength(character.velocity);
    state_.minWallDist   = computeMinWallDist(character.position);
    state_.timeInBehavior = timeInBehavior_;

    // 2) Traverse tree
    DecisionNode* node = root_;
    while (auto* cond = dynamic_cast<ConditionNode*>(node)) {
        node = cond->evaluate(state_);
    }
    auto* action = static_cast<ActionNode*>(node);
    auto  behavior = action->getBehavior();

    // 3) Behavior‑change logic
    if (behavior != lastBehavior_) {
        timeInBehavior_ = 0.f;
        lastBehavior_   = behavior;
    } else {
        timeInBehavior_ += dt;
    }

    // 4) Dispatch
    switch (behavior) {
        case BehaviorType::PickNewWaypoint:
            pickNewWaypoint(character);
            return {};  // no steering this frame
            case BehaviorType::Pathfind: {
                // 1) Which node we’re aiming at
                sf::Vector2f targetPos = graphNodes_[ currentPath_[currentPathIndex_] ].position;
            
                // 2) Advance segment if close enough
                float segDist = vectorLength(targetPos - character.position);
                if (segDist < State::targetEpsilon) {
                    currentPathIndex_++;
                    if (currentPathIndex_ >= (int)currentPath_.size()) {
                        lastBehavior_ = BehaviorType::PickNewWaypoint;
                        return {};
                    }
                    targetPos = graphNodes_[ currentPath_[currentPathIndex_] ].position;
                }
            
                // 3) Build correct target orientation
                Kinematic targetKin;
                targetKin.position = targetPos;
                float dx = targetPos.x - character.position.x;
                float dy = targetPos.y - character.position.y;
                targetKin.orientation = std::atan2(dy, dx);  // ← desired heading
                targetKin.velocity    = sf::Vector2f(0.f, 0.f);
                targetKin.rotation    = 0.f;
            
                // 4) Get steering
                SteeringOutput la = arrive_.getSteering(character, targetKin, dt);
                SteeringOutput al = align_.getSteering(character, targetKin, dt);
                return { la.linear, al.angular };
            }
            
        case BehaviorType::Wander:
            return wander_.getSteering(character, character, dt);
        case BehaviorType::Flee:
            // simple stand‑in: you can build a FleeBehavior similarly
            return wander_.getSteering(character, character, dt);
    }
    return {};
}

void BehaviorController::initialize(Kinematic& character) {
    pickNewWaypoint(character);
}


void BehaviorController::pickNewWaypoint(Kinematic& character) {
    int start = getClosestNode(character.position);
    currentWaypoint_ = std::rand() % graphNodes_.size();
    currentPath_     = AStar(start, currentWaypoint_);
    currentPathIndex_ = 0;
}

float BehaviorController::computeMinWallDist(const sf::Vector2f& pos) const {
    float best = std::numeric_limits<float>::infinity();
    for (auto& w : walls_) {
        // axis‑aligned distance to rectangle
        float dx = std::max({ w.getPosition().x - pos.x,
                              pos.x - (w.getPosition().x + w.getSize().x),
                              0.f });
        float dy = std::max({ w.getPosition().y - pos.y,
                              pos.y - (w.getPosition().y + w.getSize().y),
                              0.f });
        best = std::min(best, std::hypot(dx, dy));
    }
    return best;
}
