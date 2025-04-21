// part3.cpp

#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <iostream>

#include "Node.hpp"            // for Node, extern graphNodes, getClosestNode, AStar
#include "Environment.hpp"     // for drawSymmetricRoomLayout, createGraphGrid, isInsideWall
#include "Steering.hpp"        // for Kinematic, ArriveBehavior, AlignBehavior, vectorLength, mapToRange
#include "BehaviorController.hpp"
#include "MonsterController.hpp"
#include "DataRecorder.hpp"    // for Sample, DataRecorder

// ————————————————————————————————————————————————————————————————————————————————
// helper: which of the 4 rooms am I in?
//———————————————————————————————————————————————————————————————————————————————
int getRoomId(const sf::Vector2f& pos) {
    // window is 640×480, split at x=320, y=240
    bool right = pos.x >= 320.f;
    bool down  = pos.y >= 240.f;
    if (!right && !down) return 0; // top-left
    if ( right && !down) return 1; // top-right
    if (!right &&  down) return 2; // bottom-left
    return 3;                       // bottom-right
}

int main() {
    // 1) set up window & environment
    sf::RenderWindow window({640,480}, "Part3: Data Collection");
    std::vector<sf::RectangleShape> walls;
    drawSymmetricRoomLayout(walls);
    createGraphGrid(graphNodes, walls, 24, 640, 480);

    // 2) player boid + behavior‐tree controller
    Kinematic player;
    player.position    = graphNodes[0].position;
    player.velocity    = {0,0};
    player.orientation = 0.f;
    player.rotation    = 0.f;
    BehaviorController playerCtrl(graphNodes, walls);
    playerCtrl.initialize(player);

    // 3) monster boid + behavior‐tree controller
    Kinematic monster;
    monster.position    = graphNodes.back().position;
    monster.velocity    = {0,0};
    monster.orientation = 0.f;
    monster.rotation    = 0.f;
    MonsterController monsterCtrl(
        graphNodes, walls,
        monster, player,
        monster.position, player.position,
        /*eatRadius=*/30.f
    );

    // 4) data recorder
    DataRecorder recorder("monster_data.csv");

    // 5) shared sprite (black for player, red for monster)
    sf::Texture tex;
    if (!tex.loadFromFile("./boid-sm.png")) {
        std::cerr << "Failed to load boid-sm.png\n";
        return -1;
    }
    sf::Sprite playerSprite(tex), monsterSprite(tex);
    playerSprite.setOrigin(tex.getSize().x/2.f, tex.getSize().y/2.f);
    monsterSprite.setOrigin(tex.getSize().x/2.f, tex.getSize().y/2.f);
    playerSprite.setScale(2.5f,2.5f);
    monsterSprite.setScale(2.5f,2.5f);
    monsterSprite.setColor(sf::Color::Red);

    // 6) main loop
    sf::Clock clock;
    while (window.isOpen()) {
        // — events —
        sf::Event e;
        while (window.pollEvent(e))
            if (e.type == sf::Event::Closed)
                window.close();

        float dt = clock.restart().asSeconds();

        // — update player via its BT —
        SteeringOutput ps = playerCtrl.update(player, dt);
        player.velocity    += ps.linear  * dt;
        player.position    += player.velocity * dt;
        player.rotation    += ps.angular * dt;
        player.orientation += player.rotation * dt;
        player.orientation  = mapToRange(player.orientation);

        // — update monster via its BT —
        monsterCtrl.update(dt);

        // — record a Sample —
        Sample s;
        s.roomId       = getRoomId(monster.position);
        s.distToPlayer = vectorLength(player.position - monster.position);
        s.inAggro      = (s.distToPlayer < 400.f);
        // probe a little ahead to see if walking into wall
        sf::Vector2f probe = monster.position +
            sf::Vector2f(std::cos(monster.orientation),
                         std::sin(monster.orientation)) * 10.f;
        s.hittingWall  = isInsideWall(probe, walls);
        s.action       = monsterCtrl.getLastActionName();  // "chase","wander","reset"
        recorder.record(s);

        // — draw for visualization (optional) —
        window.clear(sf::Color::White);
        for (auto& w : walls) window.draw(w);
        // draw nav‐mesh nodes (optional)
        sf::CircleShape dot(3.f);
        dot.setOrigin(3,3);
        dot.setFillColor(sf::Color(180,180,180));
        for (auto& n : graphNodes) {
            dot.setPosition(n.position);
            window.draw(dot);
        }
        playerSprite.setPosition(player.position);
        playerSprite.setRotation(player.orientation*180.f/PI);
        window.draw(playerSprite);
        monsterSprite.setPosition(monster.position);
        monsterSprite.setRotation(monster.orientation*180.f/PI);
        window.draw(monsterSprite);
        window.display();
    }

    return 0;
}
