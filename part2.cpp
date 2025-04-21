// part2.cpp

#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <iostream>

#include "Environment.hpp"       // extern graphNodes; drawSymmetricRoomLayout; createGraphGrid; isInsideWall
#include "Node.hpp"              // getClosestNode; AStar
#include "Steering.hpp"          // Kinematic, vectorLength, normalize, mapToRange
#include "BehaviorController.hpp"
#include "MonsterController.hpp"

using namespace std;

// ────────────────────────────────────────────────────────────────────────────────
// A tiny crumb + breadcrumb‐buffer that takes a color
// ────────────────────────────────────────────────────────────────────────────────
struct crumb {
    sf::CircleShape shape;
    crumb(sf::Color c) {
        shape.setRadius(3.f);
        shape.setOrigin(3.f, 3.f);
        shape.setFillColor(c);
    }
};

struct BoidBreadcrumbs {
    vector<crumb> crumbs;
    float         timer;
    int           idx;

    BoidBreadcrumbs(sf::Color c)
      : timer(0.5f), idx(0)
    {
        crumbs.reserve(20);
        for (int i = 0; i < 20; ++i)
            crumbs.emplace_back(c);
    }
};

// simple alias
inline float distanceVec(const sf::Vector2f& a, const sf::Vector2f& b) {
    sf::Vector2f d = a - b;
    return std::sqrt(d.x*d.x + d.y*d.y);
}

int main() {
    // 1) Window & environment
    sf::RenderWindow window({640,480}, "Part2");
    vector<sf::RectangleShape> walls;
    drawSymmetricRoomLayout(walls);
    createGraphGrid(graphNodes, walls, 24, 640, 480);

    // 2) Player boid + controller + breadcrumbs
    Kinematic player{ graphNodes[0].position, {0,0}, 0.f, 0.f };
    BehaviorController playerCtrl(graphNodes, walls);
    playerCtrl.initialize(player);
    BoidBreadcrumbs playerCrumbs(sf::Color(100,100,100,180));

    // 3) Monster boid + controller + breadcrumbs
    Kinematic monster{ graphNodes.back().position, {0,0}, 0.f, 0.f };
    MonsterController monsterCtrl(
        graphNodes, walls,
        monster, player,
        monster.position, player.position,
        /*eatRadius=*/12.f
    );
    BoidBreadcrumbs monsterCrumbs(sf::Color::Red);

    // cache starts for manual reset on collision
    sf::Vector2f playerStart  = player.position;
    sf::Vector2f monsterStart = monster.position;

    // 4) Shared sprite setup
    sf::Texture tex;
    if (!tex.loadFromFile("./boid-sm.png")) {
        cerr << "Failed to load boid-sm.png\n";
        return -1;
    }
    sf::Sprite playerSprite(tex), monsterSprite(tex);
    playerSprite.setOrigin(tex.getSize().x/2.f, tex.getSize().y/2.f);
    monsterSprite.setOrigin(tex.getSize().x/2.f, tex.getSize().y/2.f);
    playerSprite.setScale(2.5f,2.5f);
    monsterSprite.setScale(3.5f,3.5f);
    monsterSprite.setColor(sf::Color::Red);

    sf::Clock clock;
    const float eatRadius = 12.f;

    // 5) Main loop
    while (window.isOpen()) {
        // poll
        sf::Event e;
        while (window.pollEvent(e))
            if (e.type == sf::Event::Closed)
                window.close();

        float dt = clock.restart().asSeconds();

        // — Update player w/ wall‑clamp —
        SteeringOutput ps = playerCtrl.update(player, dt);
        player.velocity += ps.linear * dt;
        {
            auto prev = player.position;
            player.position += player.velocity * dt;
            if (isInsideWall(player.position, walls)) {
                player.position = prev;
                player.velocity = {0,0};
            }
        }
        player.rotation    += ps.angular * dt;
        player.orientation += player.rotation * dt;
        player.orientation  = mapToRange(player.orientation);

        // — Drop player crumb —
        playerCrumbs.timer -= dt;
        if (playerCrumbs.timer <= 0.f) {
            playerCrumbs.timer += 0.1f;
            auto& cb = playerCrumbs.crumbs[playerCrumbs.idx];
            cb.shape.setPosition(player.position);
            playerCrumbs.idx = (playerCrumbs.idx + 1) % playerCrumbs.crumbs.size();
        }

        // — Update monster w/ wall‑clamp —
        {
            auto prevM = monster.position;
            monsterCtrl.update(dt);
            if (isInsideWall(monster.position, walls)) {
                monster.position = prevM;
                monster.velocity = {0,0};
            }
        }

        // — Drop monster crumb —
        monsterCrumbs.timer -= dt;
        if (monsterCrumbs.timer <= 0.f) {
            monsterCrumbs.timer += 0.1f;
            auto& cb = monsterCrumbs.crumbs[monsterCrumbs.idx];
            cb.shape.setPosition(monster.position);
            monsterCrumbs.idx = (monsterCrumbs.idx + 1) % monsterCrumbs.crumbs.size();
        }

        // — Manual collision reset —
        if (distanceVec(player.position, monster.position) < eatRadius) {
            // reset player
            player.position    = playerStart;
            player.velocity    = {0,0};
            player.orientation = 0.f;
            player.rotation    = 0.f;
            playerCtrl.initialize(player);

            // reset monster (its ResetTask will also fire next tick)
            monster.position    = monsterStart;
            monster.velocity    = {0,0};
            monster.orientation = 0.f;
            monster.rotation    = 0.f;
        }

        // — Draw —
        window.clear(sf::Color::White);

        // walls
        for (auto& w : walls)
            window.draw(w);

        // breadcrumbs
        for (auto& cb : playerCrumbs.crumbs)
            window.draw(cb.shape);
        for (auto& cb : monsterCrumbs.crumbs)
            window.draw(cb.shape);

        // graph nodes
        sf::CircleShape dot(3.f);
        dot.setOrigin(3,3);
        dot.setFillColor(sf::Color::Black);
        for (auto& n : graphNodes) {
            dot.setPosition(n.position);
            window.draw(dot);
        }

        // sprites
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
