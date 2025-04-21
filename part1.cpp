#include <SFML/Graphics.hpp>
#include <vector>
#include <queue>
#include <cmath>
#include <iostream>

#include "Steering.hpp"
#include "BehaviorController.hpp"
#include "Node.hpp"

using namespace std;

//———————————————————————————————————————————————————————————————————————————————
// Breadcrumb definitions
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

//———————————————————————————————————————————————————————————————————————————————
// Your existing globals & utilities
//———————————————————————————————————————————————————————————————————————————————
vector<Node> graphNodes;
vector<int>  currentPath;
int          currentPathIndex = 0;

float distance(const sf::Vector2f& a, const sf::Vector2f& b) {
    sf::Vector2f d = a - b;
    return sqrt(d.x*d.x + d.y*d.y);
}

bool isInsideWall(const sf::Vector2f& pos,
                  const vector<sf::RectangleShape>& walls)
{
    for (auto& w : walls)
        if (w.getGlobalBounds().contains(pos))
            return true;
    return false;
}

void drawSymmetricRoomLayout(vector<sf::RectangleShape>& walls) {
    sf::RectangleShape wall;
    wall.setFillColor(sf::Color(150,0,0));
    // … all your wall setup as before …
    // Outer
    wall.setSize({12,420}); wall.setPosition({50,30});  walls.push_back(wall);
    wall.setSize({12,420}); wall.setPosition({568,30}); walls.push_back(wall);
    wall.setSize({515,12}); wall.setPosition({62,30});  walls.push_back(wall);
    wall.setSize({515,12}); wall.setPosition({62,438});walls.push_back(wall);
    // Separators
    wall.setSize({184,12}); wall.setPosition({62,240}); walls.push_back(wall);
    wall.setSize({175,12}); wall.setPosition({395,240});walls.push_back(wall);
    wall.setSize({12,150}); wall.setPosition({280,30}); walls.push_back(wall);
    wall.setSize({12,160}); wall.setPosition({285,280});walls.push_back(wall);
    // extras…
    wall.setSize({12,72});  wall.setPosition({165,130});walls.push_back(wall);
    wall.setSize({12,72});  wall.setPosition({375,80}); walls.push_back(wall);
    wall.setSize({12,72});  wall.setPosition({430,80}); walls.push_back(wall);
    wall.setSize({56,12});  wall.setPosition({375,140});walls.push_back(wall);
    wall.setSize({12,72});  wall.setPosition({425,325});walls.push_back(wall);
    wall.setSize({72,12});  wall.setPosition({395,350});walls.push_back(wall);
    wall.setSize({12,72});  wall.setPosition({140,330});walls.push_back(wall);
    wall.setSize({72,12});  wall.setPosition({110,350});walls.push_back(wall);
}

void createGraphGrid(vector<Node>& graph,
                     const vector<sf::RectangleShape>& walls,
                     int spacing,
                     int W,
                     int H)
{
    vector<sf::Vector2f> removePts = {
        {156,126},{156,150},{156,174},{156,198},
        {396,102},{420,102},{396,126},
        {108,366},{444,366},{468,366}
    };
    for (int x = spacing; x < W; x += spacing) {
        for (int y = spacing; y < H; y += spacing) {
            sf::Vector2f p(x,y);
            if (x<=60||x>=580||y<=30||y>=450) continue;
            if (isInsideWall(p,walls)) continue;
            bool skip=false;
            for (auto& r : removePts)
                if (distance(p,r)<1.f) { skip=true; break; }
            if (!skip) graph.push_back({p,{}});
        }
    }
    for (int i=0; i<(int)graph.size(); ++i) {
        for (int j=i+1; j<(int)graph.size(); ++j) {
            if (distance(graph[i].position, graph[j].position) <= spacing+2) {
                graph[i].neighbors.push_back(j);
                graph[j].neighbors.push_back(i);
            }
        }
    }
}

int getClosestNode(const sf::Vector2f& pos) {
    int best = 0;
    float md = distance(pos, graphNodes[0].position);
    for (int i = 1; i < (int)graphNodes.size(); ++i) {
        float d = distance(pos, graphNodes[i].position);
        if (d < md) { md = d; best = i; }
    }
    return best;
}

vector<int> AStar(int startIdx, int goalIdx) {
    int N = graphNodes.size();
    vector<float> G(N, INFINITY), F(N, INFINITY);
    vector<int> from(N, -1);
    auto cmp = [&](int a,int b){ return F[a]>F[b]; };
    priority_queue<int, vector<int>, decltype(cmp)> open(cmp);

    G[startIdx] = 0;
    F[startIdx] = distance(graphNodes[startIdx].position,
                           graphNodes[goalIdx].position);
    open.push(startIdx);

    while (!open.empty()) {
        int cur = open.top(); open.pop();
        if (cur == goalIdx) {
            vector<int> path;
            for (int at = cur; at != -1; at = from[at])
                path.push_back(at);
            reverse(path.begin(), path.end());
            return path;
        }
        for (int nb : graphNodes[cur].neighbors) {
            float t = G[cur] +
              distance(graphNodes[cur].position, graphNodes[nb].position);
            if (t < G[nb]) {
                from[nb] = cur;
                G[nb]    = t;
                F[nb]    = t + distance(
                  graphNodes[nb].position,
                  graphNodes[goalIdx].position
                );
                open.push(nb);
            }
        }
    }
    return {};
}

//———————————————————————————————————————————————————————————————————————————————
// main()
//———————————————————————————————————————————————————————————————————————————————
int main() {
    sf::RenderWindow window({640,480}, "Scaled 4-Room Layout");
    vector<sf::RectangleShape> walls;
    drawSymmetricRoomLayout(walls);
    createGraphGrid(graphNodes, walls, 24, 640, 480);

    // — Player Boid & Breadcrumbs —
    Kinematic character;
    character.position    = graphNodes[0].position;
    character.velocity    = {0,0};
    character.orientation = 0;
    character.rotation    = 0;

    BehaviorController controller(graphNodes, walls);
    controller.initialize(character);

    BoidBreadcrumbs breadcrumbs;  // <— here!

    // — Sprite setup —
    sf::Texture boidTexture;
    if (!boidTexture.loadFromFile("./boid-sm.png")) {
        cout << "Failed to load boid image!" << endl;
        return -1;
    }
    sf::Sprite boidSprite(boidTexture);
    boidSprite.setOrigin(
      boidSprite.getLocalBounds().width/2.f,
      boidSprite.getLocalBounds().height/2.f
    );
    boidSprite.setScale(2.5f,2.5f);

    sf::Clock clock;
    while (window.isOpen()) {
        // 1) Poll events
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        // 2) Decision‑tree update
        float dt = clock.restart().asSeconds();
        SteeringOutput steer = controller.update(character, dt);

        // 3) Integrate kinematics
        character.velocity    += steer.linear  * dt;
        character.position    += character.velocity * dt;
        character.rotation    += steer.angular * dt;
        character.orientation += character.rotation * dt;
        character.orientation  = mapToRange(character.orientation);

        // 4) Drop a breadcrumb every 0.1s
        breadcrumbs.drop_timer -= dt;
        if (breadcrumbs.drop_timer <= 0.f) {
            breadcrumbs.drop_timer += 0.4f;
            auto& cb = breadcrumbs.crumbs[breadcrumbs.crumb_idx];
            cb.shape.setPosition(character.position);
            breadcrumbs.crumb_idx =
              (breadcrumbs.crumb_idx + 1) % breadcrumbs.crumbs.size();
        }

        // 5) Draw
        window.clear(sf::Color::White);

        // • walls
        for (auto& w : walls)
            window.draw(w);

        // • breadcrumbs
        for (auto& cb : breadcrumbs.crumbs)
            window.draw(cb.shape);

        // • graph nodes
        sf::CircleShape nodeShape(3.f);
        nodeShape.setOrigin(3.f,3.f);
        nodeShape.setFillColor(sf::Color::Black);
        for (auto& n : graphNodes) {
            nodeShape.setPosition(n.position);
            window.draw(nodeShape);
        }

        // • boid sprite
        boidSprite.setPosition(character.position);
        boidSprite.setRotation(character.orientation * 180.f / PI);
        window.draw(boidSprite);

        window.display();
    }

    return 0;
}
