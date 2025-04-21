#include "MonsterTasks.hpp"
#include "Node.hpp"         // graphNodes, getClosestNode, AStar
#include "Steering.hpp"     // ArriveBehavior, AlignBehavior, vectorLength, mapToRange
#include <cstdlib>
#include <ctime>
#include <cmath>

// pull in the same nav‐mesh you filled in main()
extern std::vector<Node> graphNodes;

// seed rand()
static bool _seeded = [](){
    std::srand((unsigned)std::time(nullptr));
    return true;
}();

// flag so we clear wander path after reset
static bool gJustReset = false;

// ——— ResetTask ——————————————————————————————————————————————
ResetTask::ResetTask(const sf::Vector2f& monStart,
                     const sf::Vector2f& plyStart)
  : monStart_(monStart)
  , plyStart_(plyStart)
  , done_(false)
{}

Status ResetTask::tick(WorldState& w, float /*dt*/) {
    w.lastAction = "reset";
    if (!done_) {
        // teleport both back
        w.monster->position    = monStart_;
        w.monster->velocity    = {0,0};
        w.monster->orientation = 0;
        w.monster->rotation    = 0;
        w.player->position     = plyStart_;
        w.player->velocity     = {0,0};
        w.player->orientation  = 0;
        w.player->rotation     = 0;
        // signal wander to clear paths
        gJustReset = true;
        done_      = true;
        return Status::Running;
    }
    done_ = false;
    return Status::Success;
}

// ——— ChasePlayerTask —————————————————————————————————————
ChasePlayerTask::ChasePlayerTask()
  : aggroRange_(600.f)
  , pathRange_(150.f)   // <<< chase when within 150px
  , pathIdx_(0)
{}

// returns Running while path‑following, Success on “eat”, Failure if out of aggroRange_
Status ChasePlayerTask::tick(WorldState& w, float dt) {
    w.lastAction = "chase";
    Kinematic& M = *w.monster;
    Kinematic& P = *w.player;
    float      d = vectorLength(P.position - M.position);

    // if too far, give up → let tree fall through to Wander
    if (d > aggroRange_) {
        path_.clear();
        pathIdx_ = 0;
        return Status::Failure;
    }
    // if close enough to “eat”
    if (d < w.eatRadius) {
        return Status::Success;
    }
    // if inside chase‑range, do A* then Arrive/Align at boosted speed
    if (d < pathRange_) {
        // clear old path once after reset
        if (gJustReset) {
            path_.clear();
            pathIdx_ = 0;
            gJustReset = false;
        }

        // recompute full path to player every tick (you can optimize this if you like)
        int s = getClosestNode(M.position);
        int g = getClosestNode(P.position);
        path_ = AStar(s, g);

        if (!path_.empty()) {
            // wrap or reset the index if it ran off
            if (pathIdx_ >= (int)path_.size()) pathIdx_ = 0;
            // target the next waypoint
            sf::Vector2f goal = graphNodes[path_[pathIdx_]].position;
            sf::Vector2f diff = goal - M.position;
            Kinematic tgt{ goal, {0,0}, std::atan2(diff.y, diff.x), 0.f };

            // *** boosted Arrive/Align for a bully‑rush ***
            auto la = ArriveBehavior(
                /*maxAccel=*/300.f,
                /*maxSpeed=*/300.f,
                /*targetRadius=*/5.f,
                /*slowRadius=*/pathRange_,
                /*timeToTarget=*/0.3f
            ).getSteering(M, tgt, dt);

            auto al = AlignBehavior(
                /*maxAngAccel=*/500.f,
                /*maxRotation=*/3.1415f,
                /*satisfactionRadius=*/0.02f,
                /*decelerationRadius=*/0.05f,
                /*timeToTarget=*/0.05f
            ).getSteering(M, tgt, dt);

            // integrate
            M.velocity    += la.linear  * dt;
            M.position    += M.velocity * dt;
            M.rotation    += al.angular * dt;
            M.orientation += M.rotation  * dt;
            M.orientation  = mapToRange(M.orientation);

            // advance if we’ve reached this waypoint
            if (vectorLength(diff) < 5.f) pathIdx_++;
        }
        return Status::Running;
    }

    // not yet in chase‑range, so fall through
    return Status::Failure;
}

// ——— GraphWanderTask —————————————————————————————————————————
GraphWanderTask::GraphWanderTask()
  : pathIdx_(0)
{}

Status GraphWanderTask::tick(WorldState& w, float dt) {
    w.lastAction = "wander";
    Kinematic& m = *w.monster;

    // if just reset, clear out old wander path
    if (gJustReset) {
        path_.clear();
        pathIdx_ = 0;
        gJustReset = false;
    }

    // if we’ve exhausted our wander path, pick a new random goal
    if (pathIdx_ >= (int)path_.size()) {
        int s = getClosestNode(m.position);
        sf::Vector2f r{ float(std::rand()%640), float(std::rand()%480) };
        int g = getClosestNode(r);
        path_    = AStar(s, g);
        pathIdx_ = 0;
    }

    // follow it just like above (but with lower speed)
    if (!path_.empty() && pathIdx_ < (int)path_.size()) {
        sf::Vector2f goal = graphNodes[path_[pathIdx_]].position;
        sf::Vector2f diff = goal - m.position;
        Kinematic tgt{ goal, {0,0}, std::atan2(diff.y, diff.x), 0.f };

        auto la = ArriveBehavior(
            /*maxAccel=*/60.f,
            /*maxSpeed=*/60.f,
            /*targetRadius=*/5.f,
            /*slowRadius=*/30.f,
            /*timeToTarget=*/0.4f
        ).getSteering(m, tgt, dt);

        auto al = AlignBehavior(
            /*maxAngAccel=*/80.f,
            /*maxRotation=*/3.1415f,
            /*satisfactionRadius=*/0.05f,
            /*decelerationRadius=*/0.1f,
            /*timeToTarget=*/0.1f
        ).getSteering(m, tgt, dt);

        m.velocity    += la.linear  * dt;
        m.position    += m.velocity * dt;
        m.rotation    += al.angular * dt;
        m.orientation += m.rotation  * dt;
        m.orientation  = mapToRange(m.orientation);

        if (vectorLength(diff) < 5.f) pathIdx_++;
    }

    return Status::Running;
}
