// Environment.cpp
#include "Environment.hpp"
#include <cmath>

// define the global
std::vector<Node> graphNodes;

bool isInsideWall(const sf::Vector2f& pos,
                  const std::vector<sf::RectangleShape>& walls)
{
    for (auto& w : walls)
        if (w.getGlobalBounds().contains(pos))
            return true;
    return false;
}

void drawSymmetricRoomLayout(std::vector<sf::RectangleShape>& walls) {
    sf::RectangleShape wall;
    wall.setFillColor(sf::Color(150,0,0));

    // Outer
    wall.setSize({12,420}); wall.setPosition({50,30});  walls.push_back(wall);
    wall.setSize({12,420}); wall.setPosition({568,30}); walls.push_back(wall);
    wall.setSize({515,12}); wall.setPosition({62,30});  walls.push_back(wall);
    wall.setSize({515,12}); wall.setPosition({62,438});walls.push_back(wall);
    // Separators
    wall.setSize({184,12}); wall.setPosition({62,240});  walls.push_back(wall);
    wall.setSize({175,12}); wall.setPosition({395,240}); walls.push_back(wall);
    wall.setSize({12,150}); wall.setPosition({280,30});  walls.push_back(wall);
    wall.setSize({12,160}); wall.setPosition({285,280}); walls.push_back(wall);
    // Extras...
    wall.setSize({12,72});  wall.setPosition({165,130}); walls.push_back(wall);
    wall.setSize({12,72});  wall.setPosition({375,80});  walls.push_back(wall);
    wall.setSize({12,72});  wall.setPosition({430,80});  walls.push_back(wall);
    wall.setSize({56,12});  wall.setPosition({375,140}); walls.push_back(wall);
    wall.setSize({12,72});  wall.setPosition({425,325}); walls.push_back(wall);
    wall.setSize({72,12});  wall.setPosition({395,350}); walls.push_back(wall);
    wall.setSize({12,72});  wall.setPosition({140,330}); walls.push_back(wall);
    wall.setSize({72,12});  wall.setPosition({110,350}); walls.push_back(wall);
}

void createGraphGrid(std::vector<Node>& graph,
                     const std::vector<sf::RectangleShape>& walls,
                     int spacing, int width, int height)
{
    std::vector<sf::Vector2f> removePts = {
        {156,126},{156,150},{156,174},{156,198},
        {396,102},{420,102},{396,126},
        {108,366},{444,366},{468,366}
    };

    for (int x = spacing; x < width; x += spacing) {
        for (int y = spacing; y < height; y += spacing) {
            sf::Vector2f p(x,y);
            if (x<=60||x>=580||y<=30||y>=450) continue;
            if (isInsideWall(p,walls)) continue;
            bool skip=false;
            for (auto& r : removePts)
                if (std::hypot(p.x-r.x,p.y-r.y)<1.f) { skip=true; break; }
            if (!skip) graph.push_back({p,{}});
        }
    }

    // connect neighbors
    for (int i = 0; i < (int)graph.size(); ++i) {
        for (int j = i+1; j < (int)graph.size(); ++j) {
            float d = std::hypot(
                graph[i].position.x-graph[j].position.x,
                graph[i].position.y-graph[j].position.y
            );
            if (d <= spacing+2) {
                graph[i].neighbors.push_back(j);
                graph[j].neighbors.push_back(i);
            }
        }
    }
}
