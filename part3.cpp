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
// breadcrumb types
//———————————————————————————————————————————————————————————————————————————————
struct crumb {
    sf::CircleShape shape;
    crumb(sf::Color color) {
        shape.setRadius(3.f);
        shape.setOrigin(3.f, 3.f);
        shape.setFillColor(color);
    }
};

struct BoidBreadcrumbs {
    std::vector<crumb> crumbs;
    float              drop_timer = 0.1f;
    int                idx        = 0;

    BoidBreadcrumbs(sf::Color color) {
        crumbs.reserve(20);
        for (int i = 0; i < 20; ++i)
            crumbs.emplace_back(color);
    }
};

// ————————————————————————————————————————————————————————————————————————————————
// which of the 4 rooms am I in?
//———————————————————————————————————————————————————————————————————————————————
int getRoomId(const sf::Vector2f& pos) {
    bool right = pos.x >= 320.f;
    bool down  = pos.y >= 240.f;
    if (!right && !down) return 0;
    if ( right && !down) return 1;
    if (!right &&  down) return 2;
    return 3;
}

int main() {
    // 1) set up window & environment
    sf::RenderWindow window({640,480}, "Part3");
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

    // 5) breadcrumb trails
    BoidBreadcrumbs playerCrumbs(sf::Color(100,100,100,180));
    BoidBreadcrumbs monsterCrumbs(sf::Color(200,  0,  0,180));

    // 6) shared sprite (black for player, red for monster)
    sf::Texture tex;
    if (!tex.loadFromFile("./boid-sm.png")) {
        std::cerr << "Failed to load boid-sm.png\n";
        return -1;
    }
    sf::Sprite playerSprite(tex), monsterSprite(tex);
    playerSprite.setOrigin(tex.getSize().x/2.f, tex.getSize().y/2.f);
    monsterSprite.setOrigin(tex.getSize().x/2.f, tex.getSize().y/2.f);
    playerSprite.setScale(2.5f,2.5f);
    monsterSprite.setScale(3.5f,3.5f);
    monsterSprite.setColor(sf::Color::Red);

    // 7) main loop
    sf::Clock clock;
    while (window.isOpen()) {
        // — events —
        sf::Event e;
        while (window.pollEvent(e))
            if (e.type == sf::Event::Closed)
                window.close();

        float dt = clock.restart().asSeconds();

        // — update player via its BT, clamp to walls —
        SteeringOutput ps = playerCtrl.update(player, dt);
        player.velocity    += ps.linear  * dt;
        {
            sf::Vector2f prev = player.position;
            player.position += player.velocity * dt;
            if (isInsideWall(player.position, walls)) {
                player.position = prev;
                player.velocity = {0.f,0.f};
            }
        }
        player.rotation    += ps.angular * dt;
        player.orientation += player.rotation   * dt;
        player.orientation  = mapToRange(player.orientation);

        // — drop player breadcrumb —
        playerCrumbs.drop_timer -= dt;
        if (playerCrumbs.drop_timer <= 0.f) {
            playerCrumbs.drop_timer += 0.4f;
            auto& cb = playerCrumbs.crumbs[playerCrumbs.idx];
            cb.shape.setPosition(player.position);
            playerCrumbs.idx =
                (playerCrumbs.idx + 1) % playerCrumbs.crumbs.size();
        }

        // — update monster via its BT, clamp to walls —
        {
            sf::Vector2f prev = monster.position;
            monsterCtrl.update(dt);
            if (isInsideWall(monster.position, walls)) {
                monster.position = prev;
                monster.velocity = {0.f,0.f};
            }
        }

        // — drop monster breadcrumb —
        monsterCrumbs.drop_timer -= dt;
        if (monsterCrumbs.drop_timer <= 0.f) {
            monsterCrumbs.drop_timer += 0.4f;
            auto& cb = monsterCrumbs.crumbs[monsterCrumbs.idx];
            cb.shape.setPosition(monster.position);
            monsterCrumbs.idx =
                (monsterCrumbs.idx + 1) % monsterCrumbs.crumbs.size();
        }

        // — record a Sample —
        Sample s;
        s.roomId       = getRoomId(monster.position);
        s.distToPlayer = vectorLength(player.position - monster.position);
        s.inAggro      = (s.distToPlayer < 400.f);
        sf::Vector2f probe = monster.position +
            sf::Vector2f(std::cos(monster.orientation),
                         std::sin(monster.orientation)) * 10.f;
        s.hittingWall  = isInsideWall(probe, walls);
        s.action       = monsterCtrl.getLastActionName();
        recorder.record(s);

        // — draw —
        window.clear(sf::Color::White);

        // • walls
        for (auto& w : walls)
            window.draw(w);

        // • nav‑mesh (optional)
        sf::CircleShape dot(3.f);
        dot.setOrigin(3,3);
        dot.setFillColor(sf::Color(180,180,180));
        for (auto& n : graphNodes) {
            dot.setPosition(n.position);
            window.draw(dot);
        }

        // • breadcrumbs
        for (auto& cb : playerCrumbs.crumbs)
            window.draw(cb.shape);
        for (auto& cb : monsterCrumbs.crumbs)
            window.draw(cb.shape);

        // • sprites
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
