#include "DecisionTreeController.hpp"

DecisionTreeController::DecisionTreeController(const nlohmann::json& tree)
 : tree_(tree)
{}

std::string DecisionTreeController::evaluate(
    const nlohmann::json& node,
    int room, float dist, bool aggro, bool wallHit)
{
    if (node.contains("label"))
        return node["label"];
    auto a = node["attr"].get<std::string>();
    if (a=="dist") {
        if (dist <= node["thresh"].get<float>())
            return evaluate(node["le"], room, dist, aggro, wallHit);
        else
            return evaluate(node["gt"], room, dist, aggro, wallHit);
    } else {
        int v = (a=="room" ? room
              : a=="aggro"? aggro
              : /*wall*/    wallHit);
        return evaluate(node["branches"][std::to_string(v)],
                        room, dist, aggro, wallHit);
    }
}

SteeringOutput DecisionTreeController::update(
    const Kinematic& M,
    const Kinematic& P,
    const std::vector<sf::RectangleShape>& walls,
    float dt)
{
    // compute the same features:
    int room = /* your room‑id logic */;
    float dist = vectorLength(P.position - M.position);
    bool aggro = dist < 400.f;
    sf::Vector2f probe = M.position +
        sf::Vector2f(std::cos(M.orientation),std::sin(M.orientation))*10.f;
    bool wallHit = isInsideWall(probe, walls);

    auto action = evaluate(tree_, room, dist, aggro, wallHit);
    if (action=="chase") {
        Kinematic target = P;
        return ArriveBehavior(200,200,10,400,0.2f)
               .getSteering(M,target,dt);
    }
    if (action=="wander") {
        // use your GraphWanderTask inline or call into it
        // …
    }
    // reset leaf does nothing here; actual reset happens in main
    return { {0,0}, 0 };
}
