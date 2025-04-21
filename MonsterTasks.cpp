// MonsterTasks.cpp

#include "MonsterTasks.hpp"
#include "Node.hpp"         // graphNodes, getClosestNode, AStar
#include "Steering.hpp"     // ArriveBehavior, AlignBehavior, vectorLength, mapToRange
#include <cstdlib>
#include <ctime>
#include <cmath>

// pull in the shared nav‑mesh you built in main()
extern std::vector<Node> graphNodes;

// seed rand() once
static bool _seeded = [](){
    std::srand((unsigned)std::time(nullptr));
    return true;
}();

// flag to tell tasks to clear their old paths after a reset
static bool gMonsterJustReset = false;


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
        // teleport monster back
        w.monster->position    = monStart_;
        w.monster->velocity    = {0,0};
        w.monster->orientation = 0;
        w.monster->rotation    = 0;
        // teleport player back
        w.player ->position    = plyStart_;
        w.player ->velocity    = {0,0};
        w.player ->orientation = 0;
        w.player ->rotation    = 0;

        // tell both wander & chase tasks to clear their paths
        gMonsterJustReset = true;

        done_ = true;
        return Status::Running;
    }
    done_ = false;
    return Status::Success;
}



// ——— ChasePlayerTask —————————————————————————————————————
ChasePlayerTask::ChasePlayerTask()
  : aggroRange_(600.f)
  , pathRange_(150.f)   // switch to path‑follow inside 150px
  , pathIdx_(0)
{}

Status ChasePlayerTask::tick(WorldState& w, float dt) {
    if (gMonsterJustReset) {
        path_.clear();
        pathIdx_ = 0;
        gMonsterJustReset = false;
    }
    
    w.lastAction = "chase";
    Kinematic& M = *w.monster;
    Kinematic& P = *w.player;
    float      d = vectorLength(P.position - M.position);

    // 1) if too far → let wander run
    if (d > aggroRange_) {
        path_.clear();
        pathIdx_ = 0;
        return Status::Failure;
    }
    // 2) if overlapping → we’ve eaten them
    if (d < w.eatRadius) {
        return Status::Success;
    }
    // 3) if within pathRange_ → path‑follow
    if (d < pathRange_) {
        if (gMonsterJustReset || path_.empty()) {
            int s = getClosestNode(M.position);
            int g = getClosestNode(P.position);
            path_    = AStar(s, g);
            pathIdx_ = 0;
            gMonsterJustReset = false;
        }
        if (pathIdx_ < (int)path_.size()) {
            sf::Vector2f wp   = graphNodes[path_[pathIdx_]].position;
            sf::Vector2f toWP = wp - M.position;

            Kinematic tgt{ wp, {0,0},
                           std::atan2(toWP.y, toWP.x),
                           0.f };

            auto la = ArriveBehavior(200.f,200.f,5.f,pathRange_,0.4f)
                          .getSteering(M, tgt, dt);
            auto al = AlignBehavior(300.f,3.1415f,0.05f,0.1f,0.05f)
                          .getSteering(M, tgt, dt);

            M.velocity    += la.linear  * dt;
            M.position    += M.velocity * dt;
            M.rotation    += al.angular * dt;
            M.orientation += M.rotation  * dt;
            M.orientation  = mapToRange(M.orientation);

            if (vectorLength(toWP) < 5.f)
                ++pathIdx_;
        }
        return Status::Running;
    }
    // 4) otherwise we’re in mid‑range → let wander run
    return Status::Failure;
}


// ——— GraphWanderTask —————————————————————————————————————————
GraphWanderTask::GraphWanderTask()
  : pathIdx_(0)
{}

Status GraphWanderTask::tick(WorldState& w, float dt) {
    if (gMonsterJustReset) {
        path_.clear();
        pathIdx_ = 0;
        gMonsterJustReset = false;
    }
    
    w.lastAction = "wander";
    Kinematic& m = *w.monster;

    // clear any old path right after reset
    if (gMonsterJustReset) {
        path_.clear();
        pathIdx_ = 0;
        gMonsterJustReset = false;
    }

    // pick a new random goal if we’ve exhausted our current path
    if (pathIdx_ >= static_cast<int>(path_.size())) {
        int s = getClosestNode(m.position);
        int g = s;
        // ensure the goal is different from the start
        while (g == s) {
            sf::Vector2f r{ float(std::rand()%640),
                            float(std::rand()%480) };
            g = getClosestNode(r);
        }
        path_    = AStar(s, g);
        pathIdx_ = 0;
    }

    // follow the path node by node
    if (pathIdx_ < static_cast<int>(path_.size())) {
        sf::Vector2f wp   = graphNodes[path_[pathIdx_]].position;
        sf::Vector2f toWP = wp - m.position;

        Kinematic tgt{ wp, {0,0},
                       std::atan2(toWP.y, toWP.x),
                       0.f };

        auto la = ArriveBehavior( 60.f, 60.f, 4.f, 30.f, 0.3f )
                      .getSteering(m, tgt, dt);
        auto al = AlignBehavior(80.f,3.1415f,0.05f,0.1f,0.1f)
                      .getSteering(m, tgt, dt);

        m.velocity    += la.linear  * dt;
        m.position    += m.velocity * dt;
        m.rotation    += al.angular * dt;
        m.orientation += m.rotation  * dt;
        m.orientation  = mapToRange(m.orientation);

        if (vectorLength(toWP) < 5.f)
            ++pathIdx_;
    }

    return Status::Running;
}
