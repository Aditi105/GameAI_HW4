// Node.cpp
#include "Node.hpp"
#include <queue>
#include <cmath>
#include <algorithm>

static float nodeDistance(const sf::Vector2f& a, const sf::Vector2f& b) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return std::sqrt(dx*dx + dy*dy);
}

int getClosestNode(const sf::Vector2f& pos) {
    int bestIdx = 0;
    float bestD = nodeDistance(pos, graphNodes[0].position);
    for (int i = 1; i < (int)graphNodes.size(); ++i) {
        float d = nodeDistance(pos, graphNodes[i].position);
        if (d < bestD) {
            bestD = d;
            bestIdx = i;
        }
    }
    return bestIdx;
}

std::vector<int> AStar(int startIdx, int goalIdx) {
    int N = graphNodes.size();
    std::vector<float> g(N, INFINITY), f(N, INFINITY);
    std::vector<int> cameFrom(N, -1);
    auto cmp = [&](int a,int b){ return f[a] > f[b]; };
    std::priority_queue<int, std::vector<int>, decltype(cmp)> openSet(cmp);

    g[startIdx] = 0;
    f[startIdx] = nodeDistance(graphNodes[startIdx].position,
                                graphNodes[goalIdx].position);
    openSet.push(startIdx);

    while (!openSet.empty()) {
        int current = openSet.top(); openSet.pop();
        if (current == goalIdx) {
            std::vector<int> path;
            for (int at = current; at != -1; at = cameFrom[at])
                path.push_back(at);
            std::reverse(path.begin(), path.end());
            return path;
        }
        for (int nb : graphNodes[current].neighbors) {
            float tentative = g[current] +
                nodeDistance(graphNodes[current].position,
                             graphNodes[nb].position);
            if (tentative < g[nb]) {
                cameFrom[nb] = current;
                g[nb] = tentative;
                f[nb] = tentative +
                    nodeDistance(graphNodes[nb].position,
                                 graphNodes[goalIdx].position);
                openSet.push(nb);
            }
        }
    }
    return {};
}
