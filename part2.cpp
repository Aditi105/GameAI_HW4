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

//———————————————————————————————————————————————————————————————————————————————
// Breadcrumb definitions (for the player boid)
//———————————————————————————————————————————————————————————————————————————————
struct crumb {
    sf::CircleShape shape;
    crumb(int /*idx*/) {
        shape.setRadius(3.f);
        shape.setOrigin(3.f, 3.f);
        shape.setFillColor(sf::Color(100, 100, 100, 180));
    }
};

struct BoidBreadcrumbs {
    vector<crumb> crumbs;
    float         drop_timer;
    int           crumb_idx;

    BoidBreadcrumbs()
      : drop_timer(0.1f)
      , crumb_idx(0)
    {
        crumbs.reserve(20);
        for (int i = 0; i < 20; ++i)
            crumbs.emplace_back(i);
    }
};

// simple alias
inline float distanceVec(const sf::Vector2f& a, const sf::Vector2f& b) {
    sf::Vector2f d = a - b;
    return std::sqrt(d.x*d.x + d.y*d.y);
}

int main() {
    // 1) Window & environment
    sf::RenderWindow window({640,480}, "Part2: Player + Monster");
    vector<sf::RectangleShape> walls;
    drawSymmetricRoomLayout(walls);
    createGraphGrid(graphNodes, walls, 24, 640, 480);

    // 2) Player boid + controller + breadcrumbs
    Kinematic player{ graphNodes[0].position, {0,0}, 0.f, 0.f };
    BehaviorController playerCtrl(graphNodes, walls);
    playerCtrl.initialize(player);
    BoidBreadcrumbs breadcrumbs;

    // 3) Monster boid + controller
    Kinematic monster{ graphNodes.back().position, {0,0}, 0.f, 0.f };
    MonsterController monsterCtrl(
        graphNodes, walls,
        monster, player,
        monster.position, player.position,
        /*eatRadius=*/12.f
    );

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
        sf::Event e;
        while (window.pollEvent(e))
            if (e.type == sf::Event::Closed)
                window.close();

        float dt = clock.restart().asSeconds();

        // — Update player with wall‐clamp —
        SteeringOutput ps = playerCtrl.update(player, dt);
        player.velocity += ps.linear * dt;
        {
            sf::Vector2f prev = player.position;
            player.position += player.velocity * dt;
            if (isInsideWall(player.position, walls)) {
                player.position = prev;
                player.velocity = {0.f,0.f};
            }
        }
        player.rotation    += ps.angular * dt;
        player.orientation += player.rotation * dt;
        player.orientation  = mapToRange(player.orientation);

        // — Breadcrumb drop —
        breadcrumbs.drop_timer -= dt;
        if (breadcrumbs.drop_timer <= 0.f) {
            breadcrumbs.drop_timer += 0.1f;
            auto& cb = breadcrumbs.crumbs[breadcrumbs.crumb_idx];
            cb.shape.setPosition(player.position);
            breadcrumbs.crumb_idx =
              (breadcrumbs.crumb_idx + 1) % breadcrumbs.crumbs.size();
        }

        // — Update monster with wall‐clamp —
        {
            sf::Vector2f prevM = monster.position;
            monsterCtrl.update(dt);
            if (isInsideWall(monster.position, walls)) {
                monster.position = prevM;
                monster.velocity = {0.f,0.f};
            }
        }

        // — Manual collision reset —
        if (distanceVec(player.position, monster.position) < eatRadius) {
            // reset player & its controller
            player.position    = playerStart;
            player.velocity    = {0.f,0.f};
            player.orientation = 0.f;
            player.rotation    = 0.f;
            playerCtrl.initialize(player);

            // reset monster only kinematics (its BT ResetTask will fire next tick)
            monster.position    = monsterStart;
            monster.velocity    = {0.f,0.f};
            monster.orientation = 0.f;
            monster.rotation    = 0.f;
        }

        // — Draw everything —
        window.clear(sf::Color::White);

        // walls
        for (auto& w : walls)
            window.draw(w);

        // breadcrumbs
        for (auto& cb : breadcrumbs.crumbs)
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
