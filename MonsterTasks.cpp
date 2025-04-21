// MonsterTasks.cpp
#include "MonsterTasks.hpp"
#include "Node.hpp"      // graphNodes, getClosestNode, AStar
#include "Steering.hpp"
#include <cstdlib>
#include <ctime>

// seed rand once
static bool seeded = [](){ std::srand((unsigned)std::time(nullptr)); return true; }();
static bool gMonsterJustReset = false;

// — ResetTask —
ResetTask::ResetTask(const sf::Vector2f& monStart,
                     const sf::Vector2f& plyStart)
 : monStart_(monStart), plyStart_(plyStart), done_(false) {}

Status ResetTask::tick(WorldState& w, float /*dt*/) {
    w.lastAction = "reset";
    if (!done_) {
        w.monster->position    = monStart_;
        w.monster->velocity    = {0,0};
        w.monster->orientation = 0;
        w.monster->rotation    = 0;
        w.player ->position    = plyStart_;
        w.player ->velocity    = {0,0};
        w.player ->orientation = 0;
        w.player ->rotation    = 0;
        gMonsterJustReset      = true;
        done_ = true;
        return Status::Running;
    }
    done_ = false;
    return Status::Success;
}

// — ChasePlayerTask —
Status ChasePlayerTask::tick(WorldState& w, float dt) {
    w.lastAction = "chase";
    Kinematic& M = *w.monster;
    Kinematic& P = *w.player;
    float dist = vectorLength(P.position - M.position);
    const float aggroRange = 100.f;
    if (dist > aggroRange) return Status::Failure;
    if (dist < w.eatRadius)   return Status::Success;
    sf::Vector2f dir = normalize(P.position - M.position);
    const float chaseSpeed = 200.f;
    M.velocity    = dir * chaseSpeed;
    M.position   += M.velocity * dt;
    M.orientation = std::atan2(dir.y, dir.x);
    return Status::Running;
}

// — GraphWanderTask —
Status GraphWanderTask::tick(WorldState& w, float dt) {
    w.lastAction = "wander";
    Kinematic& m = *w.monster;

    if (gMonsterJustReset) {
        path_.clear();
        pathIdx_ = 0;
        gMonsterJustReset = false;
    }

    if (pathIdx_ >= (int)path_.size()) {
        int s = getClosestNode(m.position);
        sf::Vector2f r{float(std::rand()%640), float(std::rand()%480)};
        int g = getClosestNode(r);
        path_    = AStar(s, g);
        pathIdx_ = 0;
    }

    if (pathIdx_ < (int)path_.size()) {
        sf::Vector2f wp = graphNodes[path_[pathIdx_]].position;
        sf::Vector2f to = wp - m.position;
        Kinematic tgt{ wp, {0,0}, std::atan2(to.y,to.x), 0 };

        auto la = ArriveBehavior(60.f,60.f,4.f,30.f,0.3f)
                      .getSteering(m, tgt, dt);
        auto al = AlignBehavior(80.f,3.14f,0.05f,0.1f,0.1f)
                      .getSteering(m, tgt, dt);

        m.velocity    += la.linear  * dt;
        m.position    += m.velocity * dt;
        m.rotation    += al.angular * dt;
        m.orientation += m.rotation * dt;
        m.orientation  = mapToRange(m.orientation);

        if (vectorLength(to) < 5.f)
            pathIdx_++;
    }

    return Status::Running;
}
