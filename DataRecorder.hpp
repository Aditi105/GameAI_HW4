#pragma once
#include <fstream>
#include <vector>
#include <string>
#include "Node.hpp"
#include "Steering.hpp"

struct Sample {
    int   roomId;          // which room the monster is in (e.g. 0–3)
    float distToPlayer;    // Euclidean distance to player
    bool  inAggro;         // distToPlayer < aggroRange?
    bool  hittingWall;     // is the next step blocked?
    std::string action;    // “wander” or “chase” or “reset”
};

class DataRecorder {
public:
    DataRecorder(const std::string& filename);
    ~DataRecorder();
    void record(const Sample& s);
private:
    std::ofstream out_;
};
