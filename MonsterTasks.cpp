// MonsterTasks.cpp

#include "MonsterTasks.hpp"
#include "Node.hpp"           // for Node, graphNodes, getClosestNode, AStar
#include "Steering.hpp"       // for ArriveBehavior, AlignBehavior, vectorLength, mapToRange
#include <SFML/System/Vector2.hpp>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>

// bring in the same graphNodes you fill in main()
extern std::vector<Node> graphNodes;

// seed rand() once
static bool seeded = [](){
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    return true;
}();

// flag to tell GraphWanderTask to clear its old path after a reset
static bool gMonsterJustReset = false;

// ——— ResetTask ——————————————————————————————————————————————
ResetTask::ResetTask(const sf::Vector2f& monsterStart,
                     const sf::Vector2f& playerStart)
  : monStart_(monsterStart)
  , plyStart_(playerStart)
  , done_(false)
{}

Status ResetTask::tick(WorldState& w, float /*dt*/) {
    w.lastAction = "reset";
    if (!done_) {
        w.monster ->position    = monStart_;
        w.monster ->velocity    = {0.f,0.f};
        w.monster ->orientation = 0.f;
        w.monster ->rotation    = 0.f;

        w.player  ->position    = plyStart_;
        w.player  ->velocity    = {0.f,0.f};
        w.player  ->orientation = 0.f;
        w.player  ->rotation    = 0.f;

        // inform the wander task to clear its path
        gMonsterJustReset = true;

        done_ = true;
        return Status::Running;
    }
    done_ = false;
    return Status::Success;
}

// ——— ChasePlayerTask —————————————————————————————————————
Status ChasePlayerTask::tick(WorldState& w, float dt) {
    w.lastAction = "chase";
    Kinematic& M = *w.monster;
    Kinematic& P = *w.player;
    float      d = vectorLength(P.position - M.position);

    // ─── AGGRO CHECK ───
    const float aggroRange = 600.f;
    if (d > aggroRange) {
        // too far—let the tree pick the wander branch
        return Status::Failure;
    }

    // ─── COLLISION CHECK ───
    if (d < w.eatRadius) {
        // we’ve “caught” the boid—trigger reset
        return Status::Success;
    }

    // ─── STEER TOWARD THE BOID ───
    // Use the boid’s position as your Arrive/Align target:
    Kinematic targetKin;
    targetKin.position    = P.position;
    targetKin.orientation = std::atan2(
        P.position.y - M.position.y,
        P.position.x - M.position.x
    );
    targetKin.velocity    = {0.f, 0.f};
    targetKin.rotation    = 0.f;

    // give it a speed/accel boost for bullying the boid
    ArriveBehavior arrive(
        /*maxAccel=*/200.f,
        /*maxSpeed=*/200.f,
        /*targetRadius=*/10.f,
        /*slowRadius=*/aggroRange,
        /*timeToTarget=*/0.6f
    );
    AlignBehavior align(
        /*maxAngAccel=*/300.f,
        /*maxRotation=*/3.1415f,
        /*satisfactionRadius=*/0.05f,
        /*decelerationRadius=*/0.1f,
        /*timeToTarget=*/0.05f
    );

    SteeringOutput la = arrive.getSteering(M, targetKin, dt);
    SteeringOutput al = align .getSteering(M, targetKin, dt);

    // integrate
    M.velocity    += la.linear  * dt;
    M.position    += M.velocity * dt;
    M.rotation    += al.angular * dt;
    M.orientation += M.rotation * dt;
    M.orientation  = mapToRange(M.orientation);

    return Status::Running;
}



// ——— GraphWanderTask —————————————————————————————————————————
Status GraphWanderTask::tick(WorldState& w, float dt) {
    w.lastAction = "wander";
    Kinematic& m = *w.monster;

    // clear old path right after reset
    if (gMonsterJustReset) {
        path_.clear();
        pathIdx_ = 0;
        gMonsterJustReset = false;
    }

    // if done with current path, pick a new random node
    if (pathIdx_ >= static_cast<int>(path_.size())) {
        int s = getClosestNode(m.position);
        sf::Vector2f r{ float(std::rand()%640), float(std::rand()%480) };
        int g = getClosestNode(r);
        path_    = AStar(s, g);
        pathIdx_ = 0;
        // even if empty, stay in Running so we retry next tick
    }

    if (pathIdx_ < static_cast<int>(path_.size())) {
        sf::Vector2f waypoint = graphNodes[path_[pathIdx_]].position;
        sf::Vector2f toWP     = waypoint - m.position;

        Kinematic tgt;
        tgt.position    = waypoint;
        tgt.orientation = std::atan2(toWP.y, toWP.x);
        tgt.velocity    = {0.f,0.f};
        tgt.rotation    = 0.f;

        auto la = ArriveBehavior(
            /*maxAccel=*/60.f,
            /*maxSpeed=*/60.f,
            /*targetRadius=*/4.f,
            /*slowRadius=*/30.f,
            /*timeToTarget=*/0.3f
        ).getSteering(m, tgt, dt);

        auto al = AlignBehavior(
            /*maxAngAccel=*/80.f,
            /*maxRotation=*/3.1415f,
            /*satisfactionRadius=*/0.05f,
            /*decelerationRadius=*/0.1f,
            /*timeToTarget=*/0.1f
        ).getSteering(m, tgt, dt);

        // integrate wander
        m.velocity    += la.linear  * dt;
        m.position    += m.velocity * dt;
        m.rotation    += al.angular * dt;
        m.orientation += m.rotation * dt;
        m.orientation  = mapToRange(m.orientation);

        // advance to next node
        if (vectorLength(toWP) < 5.f) {
            pathIdx_++;
        }
    }

    return Status::Running;
}
