// part1.cpp

#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <iostream>

#include "Environment.hpp"     // declares drawSymmetricRoomLayout(), createGraphGrid(), isInsideWall(), extern graphNodes
#include "Node.hpp"            // declares getClosestNode(), AStar()
#include "Steering.hpp"        // your Arrive/Align/Wander APIs

int main() {
    sf::RenderWindow window({640,480}, "Part1");
    
    // 1) Build walls & nav‑mesh from the shared code
    std::vector<sf::RectangleShape> walls;
    drawSymmetricRoomLayout(walls);
    createGraphGrid(graphNodes, walls, /*spacing=*/24, 640, 480);

    // 2) Load your boid sprite as before
    sf::Texture boidTexture;
    if (!boidTexture.loadFromFile("./boid-sm.png")) {
        std::cerr << "Failed to load boid-sm.png\n"; 
        return -1;
    }
    sf::Sprite boidSprite(boidTexture);
    boidSprite.setOrigin(
      boidTexture.getSize().x/2.f,
      boidTexture.getSize().y/2.f
    );
    boidSprite.setScale(2.5f,2.5f);

    // 3) Kinematic state + steering behaviors
    Kinematic character;
    character.position    = graphNodes[0].position;
    character.velocity    = {0,0};
    character.orientation = 0;
    character.rotation    = 0;

    Kinematic targetKinematic = character;
    ArriveBehavior arrive(150.f, 150.f, 10.f, 80.f, 0.4f);
    AlignBehavior align(100.f, PI/1.f, 0.1f, 0.1f, 0.1f);

    sf::Clock clock;
    bool frozen = true;
    std::vector<int> currentPath;
    size_t currentPathIndex = 0;

    while (window.isOpen()) {
        // — poll events —
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            // mouse click kicks off A*
            if (event.type == sf::Event::MouseButtonPressed 
             && event.mouseButton.button == sf::Mouse::Left)
            {
                int startIdx = getClosestNode(character.position);
                int goalIdx  = getClosestNode({float(event.mouseButton.x),
                                               float(event.mouseButton.y)});
                currentPath = AStar(startIdx, goalIdx);
                currentPathIndex = 0;
                frozen = false;
            }
        }

        float dt = clock.restart().asSeconds();

        // — follow the A* path via Arrive+Align —
        if (!frozen && !currentPath.empty()) {
            sf::Vector2f targetPos = graphNodes[currentPath[currentPathIndex]].position;
            sf::Vector2f toTarget  = targetPos - character.position;
            float dist              = vectorLength(toTarget);

            // orient toward next node
            targetKinematic.position    = targetPos;
            targetKinematic.orientation = (dist>1e-3f)
                                         ? std::atan2(toTarget.y,toTarget.x)
                                         : character.orientation;

            auto arriveSteer = arrive.getSteering(character, targetKinematic, dt);
            auto alignSteer  = align .getSteering(character, targetKinematic, dt);

            character.velocity    += arriveSteer.linear  * dt;
            character.position    += character.velocity  * dt;
            character.rotation    += alignSteer.angular  * dt;
            character.orientation += character.rotation   * dt;
            character.orientation  = mapToRange(character.orientation);

            // advance to next waypoint?
            if (dist < 10.f) {
                ++currentPathIndex;
                if (currentPathIndex >= currentPath.size()) {
                    frozen = true;
                    character.velocity = {0,0};
                }
            }
        }

        // — draw —
        boidSprite.setPosition(character.position);
        boidSprite.setRotation(character.orientation*180.f/PI);

        window.clear(sf::Color::White);

        // draw walls
        for (auto& w : walls)
            window.draw(w);

        // draw graph edges & nodes (optional)
        sf::CircleShape nodeDot(3.f);
        nodeDot.setOrigin(3,3);
        nodeDot.setFillColor(sf::Color(180,180,180));
        for (auto& n : graphNodes) {
            nodeDot.setPosition(n.position);
            window.draw(nodeDot);
            for (int nb : n.neighbors) {
                sf::Vertex line[] = {
                    {n.position, sf::Color(200,200,200)},
                    {graphNodes[nb].position, sf::Color(200,200,200)}
                };
                window.draw(line, 2, sf::Lines);
            }
        }

        // draw path in red (optional)
        for (size_t i = 0; i+1 < currentPath.size(); ++i) {
            sf::Vertex seg[] = {
                {graphNodes[currentPath[i]].position, sf::Color::Red},
                {graphNodes[currentPath[i+1]].position, sf::Color::Red}
            };
            window.draw(seg,2,sf::Lines);
        }

        window.draw(boidSprite);
        window.display();
    }

    return 0;
}
