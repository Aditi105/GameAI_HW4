#include "MonsterTasks.hpp"
#include "Node.hpp"         // graphNodes, getClosestNode, AStar
#include "Steering.hpp"     // ArriveBehavior, AlignBehavior, vectorLength, mapToRange
#include <cstdlib>
#include <ctime>
#include <cmath>

// bring in the same nav‐mesh you built in main()
extern std::vector<Node> graphNodes;

// seed once
static bool _seeded = [](){
    std::srand((unsigned)std::time(nullptr));
    return true;
}();

// ——— Reset ——————————————————————————————————————————————
ResetTask::ResetTask(const sf::Vector2f& monStart,
                     const sf::Vector2f& plyStart)
  : monStart_(monStart)
  , plyStart_(plyStart)
  , done_(false)
{}

Status ResetTask::tick(WorldState& w, float /*dt*/) {
    w.lastAction = "reset";
    if (!done_) {
        // teleport both
        w.monster->position    = monStart_;
        w.monster->velocity    = {0,0};
        w.monster->orientation = 0;
        w.monster->rotation    = 0;
        w.player ->position    = plyStart_;
        w.player ->velocity    = {0,0};
        w.player ->orientation = 0;
        w.player ->rotation    = 0;
        done_ = true;
        return Status::Running;
    }
    done_ = false;
    return Status::Success;
}

// ——— Chase with path‐follow ——————————————————————————————————
ChasePlayerTask::ChasePlayerTask()
  : aggroRange_(600.f)
  , pathRange_(400.f)
  , pathIdx_(0)
{}

Status ChasePlayerTask::tick(WorldState& w, float dt) {
    w.lastAction = "chase";
    Kinematic& M = *w.monster;
    Kinematic& P = *w.player;
    float      d = vectorLength(P.position - M.position);

    if (d > aggroRange_) {
        path_.clear();
        pathIdx_ = 0;
        return Status::Failure;
    }
    if (d < w.eatRadius) {
        return Status::Success;
    }
    if (d < pathRange_) {
        int s = getClosestNode(M.position);
        int g = getClosestNode(P.position);
        path_ = AStar(s, g);
        if (!path_.empty()) {
            if (pathIdx_ >= (int)path_.size()) pathIdx_ = 0;
            sf::Vector2f goal = graphNodes[path_[pathIdx_]].position;
            sf::Vector2f diff = goal - M.position;
            Kinematic tgt{ goal, {0,0}, std::atan2(diff.y, diff.x), 0.f };
            auto la = ArriveBehavior(200.f, 200.f, 5.f, 50.f, 0.1f)
                        .getSteering(M, tgt, dt);
            auto al = AlignBehavior(300.f, 3.1415f, 0.05f, 0.1f, 0.05f)
                        .getSteering(M, tgt, dt);
            M.velocity    += la.linear  * dt;
            M.position    += M.velocity  * dt;
            M.rotation    += al.angular * dt;
            M.orientation += M.rotation  * dt;
            M.orientation  = mapToRange(M.orientation);
            if (vectorLength(diff) < 5.f) pathIdx_++;
        }
        return Status::Running;
    }
    return Status::Failure;
}


// ——— Graph Wander (unchanged) ——————————————————————————————————
Status GraphWanderTask::tick(WorldState& w, float dt) {
    w.lastAction = "wander";
    Kinematic& m = *w.monster;

    // (re‑use your existing wander‑along‑A* code here,
    //  clearing path_ & pathIdx_ on reset, picking a random graph node,
    //  then Arrive/Align to each waypoint in turn.)

    return Status::Running;
}
