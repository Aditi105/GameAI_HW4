#pragma once
#include <vector>
#include <string>
#include <nlohmann/json.hpp>  // you can vendor a single‑header JSON lib
#include "Node.hpp"
#include "Steering.hpp"

class DecisionTreeController {
public:
    DecisionTreeController(const nlohmann::json& tree);
    SteeringOutput update(const Kinematic& monster,
                          const Kinematic& player,
                          const std::vector<sf::RectangleShape>& walls,
                          float dt);
private:
    nlohmann::json tree_;
    std::string    evaluate(const nlohmann::json& node,
                            int room,
                            float dist,
                            bool aggro,
                            bool wallHit);
};
