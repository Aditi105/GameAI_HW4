#pragma once

#include <SFML/Graphics.hpp>

/// Holds the agent’s sensory inputs for decision‑making.
struct State {
    float distanceToTarget;    // current distance to the waypoint
    float timeInBehavior;      // seconds since the last behavior switch
    float minWallDist;         // distance to the nearest wall
    float speed;               // magnitude of the character’s velocity

    //––– Tuning constants –––
    static constexpr float targetEpsilon    = 10.f;  // “at target” threshold
    static constexpr float wallThreshold    = 30.f;  // “too close to wall” threshold
    static constexpr float behaviorTimeout  = 4.f;   // seconds before forcing a change
    static constexpr float maxSpeedThreshold = 150.f;

    bool atTarget() const    { return distanceToTarget <  targetEpsilon; }
    bool nearWall() const    { return minWallDist      <  wallThreshold; }
    bool timedOut() const    { return timeInBehavior  >  behaviorTimeout; }
    bool isTooFast()  const { return speed > maxSpeedThreshold; }
};
