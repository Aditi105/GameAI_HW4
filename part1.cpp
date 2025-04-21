#include <SFML/Graphics.hpp>
#include <vector>
#include <queue>
#include <cmath>
#include <iostream>
#include "Steering.hpp"
#include "BehaviorController.hpp"
#include "Node.hpp"

using namespace std;


vector<Node> graphNodes;
vector<int> currentPath;
int currentPathIndex = 0;

float distance(const sf::Vector2f& a, const sf::Vector2f& b) {
    sf::Vector2f diff = a - b;
    return sqrt(diff.x * diff.x + diff.y * diff.y);
}

bool isInsideWall(const sf::Vector2f& pos, const vector<sf::RectangleShape>& walls) {
    for (auto& wall : walls)
        if (wall.getGlobalBounds().contains(pos)) return true;
    return false;
}

void drawSymmetricRoomLayout(vector<sf::RectangleShape>& walls) {
    sf::RectangleShape wall;
    wall.setFillColor(sf::Color(150, 0, 0));

    // Outer walls
    wall.setSize(sf::Vector2f(12, 420)); wall.setPosition(50, 30); walls.push_back(wall);  // Left
    wall.setSize(sf::Vector2f(12, 420)); wall.setPosition(568, 30); walls.push_back(wall); // Right
    wall.setSize(sf::Vector2f(515, 12)); wall.setPosition(62, 30); walls.push_back(wall);  // Top
    wall.setSize(sf::Vector2f(515, 12)); wall.setPosition(62, 438); walls.push_back(wall); // Bottom

    // Room separators
    wall.setSize(sf::Vector2f(184, 12)); wall.setPosition(62, 240); walls.push_back(wall); // Left horizontal
    wall.setSize(sf::Vector2f(175, 12)); wall.setPosition(395, 240); walls.push_back(wall); // Right horizontal
    wall.setSize(sf::Vector2f(12, 150)); wall.setPosition(280, 30); walls.push_back(wall);  // Top vertical
    wall.setSize(sf::Vector2f(12, 160)); wall.setPosition(285, 280); walls.push_back(wall); // Bottom vertical

    // Top-left vertical
    wall.setSize(sf::Vector2f(12, 72)); wall.setPosition(165, 130); walls.push_back(wall);

    // U-shape top-right
    wall.setSize(sf::Vector2f(12, 72)); wall.setPosition(375, 80); walls.push_back(wall);
    wall.setSize(sf::Vector2f(12, 72)); wall.setPosition(430, 80); walls.push_back(wall);
    wall.setSize(sf::Vector2f(56, 12)); wall.setPosition(375, 140); walls.push_back(wall); //middle line

    // Bottom-right plus
    wall.setSize(sf::Vector2f(12, 72)); wall.setPosition(425, 325); walls.push_back(wall); //vertical
    wall.setSize(sf::Vector2f(72, 12)); wall.setPosition(395, 350); walls.push_back(wall); //hori

    // Bottom-left plus (mirrored)
    wall.setSize(sf::Vector2f(12, 72)); wall.setPosition(140, 330); walls.push_back(wall);
    wall.setSize(sf::Vector2f(72, 12)); wall.setPosition(110, 350); walls.push_back(wall);
}

void createGraphGrid(vector<Node>& graphNodes, const vector<sf::RectangleShape>& walls, int spacing, int width, int height) {
    vector<sf::Vector2f> removePositions = {
        {156, 126}, {156, 150}, {156, 174}, {156, 198},       // Top-left vertical
        {396, 102}, {420, 102}, {396, 126},                   // U-shape
        {108, 366},                                           // Bottom-left plus
        {444, 366}, {468, 366}                                // Bottom-right plus
    };

    for (int x = spacing; x < width; x += spacing) {
        for (int y = spacing; y < height; y += spacing) {
            sf::Vector2f pos(x, y);
            if (x <= 60 || x >= 580 || y <= 30 || y >= 450) continue;
            if (!isInsideWall(pos, walls)) {
                bool skip = false;
                for (const auto& rm : removePositions)
                    if (distance(pos, rm) < 1.0f) {
                        skip = true;
                        break;
                    }
                if (!skip) graphNodes.push_back({ pos, {} });
            }
        }
    }

    for (int i = 0; i < graphNodes.size(); ++i) {
        for (int j = i + 1; j < graphNodes.size(); ++j) {
            float d = distance(graphNodes[i].position, graphNodes[j].position);
            if (d <= spacing + 2) {
                graphNodes[i].neighbors.push_back(j);
                graphNodes[j].neighbors.push_back(i);
            }
        }
    }
}

int getClosestNode(sf::Vector2f pos) {
    int Indx = 0;
    float minDist = distance(pos, graphNodes[0].position);
    for (int i = 1; i < graphNodes.size(); ++i) {
        float d = distance(pos, graphNodes[i].position);
        if (d < minDist) {
            minDist = d;
            Indx = i;
        }
    }
    return Indx;
}

vector<int> AStar(int startIndx, int goalIndx) {
    vector<float> gScore(graphNodes.size(), INFINITY);
    vector<float> fScore(graphNodes.size(), INFINITY);
    vector<int> cameFrom(graphNodes.size(), -1);
    auto cmp = [&](int left, int right) { return fScore[left] > fScore[right]; };
    priority_queue<int, vector<int>, decltype(cmp)> openSet(cmp);

    gScore[startIndx] = 0;
    fScore[startIndx] = distance(graphNodes[startIndx].position, graphNodes[goalIndx].position);
    openSet.push(startIndx);

    while (!openSet.empty()) {
        int current = openSet.top(); openSet.pop();

        if (current == goalIndx) {
            vector<int> path;
            while (current != -1) {
                path.push_back(current);
                current = cameFrom[current];
            }
            reverse(path.begin(), path.end());
            return path;
        }

        for (int neighbor : graphNodes[current].neighbors) {
            float tentG = gScore[current] + distance(graphNodes[current].position, graphNodes[neighbor].position);
            if (tentG < gScore[neighbor]) {
                cameFrom[neighbor] = current;
                gScore[neighbor] = tentG;
                fScore[neighbor] = tentG + distance(graphNodes[neighbor].position, graphNodes[goalIndx].position);
                openSet.push(neighbor);
            }
        }
    }
    return {};
}

int main() {
    sf::RenderWindow window(sf::VideoMode(640, 480), "Scaled 4-Room Layout");

    vector<sf::RectangleShape> walls;
    int nodeSpacing = 24;
    Kinematic character;

    drawSymmetricRoomLayout(walls);
    createGraphGrid(graphNodes, walls, nodeSpacing, 640, 480);

    BehaviorController controller(graphNodes, walls);
    controller.initialize(character);

    sf::Texture boidTexture;
    if (!boidTexture.loadFromFile("./boid-sm.png")) {
        cout << "Failed to load boid image!" << endl;
        return -1;
    }

    sf::Sprite boidSprite;
    boidSprite.setTexture(boidTexture);
    boidSprite.setOrigin(boidSprite.getLocalBounds().width / 2.f, boidSprite.getLocalBounds().height / 2.f);
    boidSprite.setScale(2.5f, 2.5f);

    
    character.position = graphNodes[0].position;
    character.velocity = sf::Vector2f(0.f, 0.f);
    character.orientation = 0.f;
    character.rotation = 0.f;

    Kinematic targetKinematic = character;
    ArriveBehavior arrive(150.f, 150.f, 10.f, 80.f, 0.4f);
    AlignBehavior align(100.f, PI / 1.0f, 0.1f, 0.1f, 0.1f);

    sf::Clock clock;
    bool frozen = true;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) 
                window.close();
        }
                // ─── decision‐tree update ───
        float deltaTime = clock.restart().asSeconds();
        SteeringOutput steer = controller.update(character, deltaTime);

        // apply whichever behavior the tree chose
        character.velocity  += steer.linear  * deltaTime;
        character.position  += character.velocity * deltaTime;
        character.rotation  += steer.angular * deltaTime;
        character.orientation+= character.rotation * deltaTime;
        character.orientation = mapToRange(character.orientation);
        

        boidSprite.setPosition(character.position);
        boidSprite.setRotation(character.orientation * 180.f / PI);

        window.clear(sf::Color::White);
        for (auto& wall : walls) window.draw(wall);

        sf::CircleShape nodeShape(3.f);
        nodeShape.setOrigin(3.f, 3.f);
        nodeShape.setFillColor(sf::Color::Black);
        for (int i = 0; i < graphNodes.size(); i++) {
            nodeShape.setPosition(graphNodes[i].position);
            window.draw(nodeShape);
            for (int neighbor : graphNodes[i].neighbors) {
                sf::Vertex line[] = {
                    sf::Vertex(graphNodes[i].position, sf::Color(180, 180, 180)),
                    sf::Vertex(graphNodes[neighbor].position, sf::Color(180, 180, 180))
                };
                window.draw(line, 2, sf::Lines);
            }
        }

        for (int i = 0; i + 1 < currentPath.size(); i++) {
            sf::Vertex line[] = {
                sf::Vertex(graphNodes[currentPath[i]].position, sf::Color::Red),
                sf::Vertex(graphNodes[currentPath[i + 1]].position, sf::Color::Red)
            };
            window.draw(line, 2, sf::Lines);
        }

        window.draw(boidSprite);
        window.display();
    }

    return 0;
}
