#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <queue>
#include <SFML/Graphics.hpp>
#include <thread>
#include <chrono>

class node {
public:
    int x;
    int y;
    char type;
    bool visited = false;
    node(int initX, int initY, char initType) {
        x = initX;
        y = initY;
        type = initType;
    }
    node() {
        x = -1;
        y = -1;
        type = 'L';
    }
};

class path {
public:
    std::vector<node> nodes;
    int length;
    path() {
        length = 0;
    }
    path extend(node extension) {
        path newPath = path();
        int extLen;
        if (extension.type == 'S' || extension.type == 'E') {
            extLen = 0;
        }
        else {
            extLen = extension.type - '0';
        }
        newPath.length = length + extLen;
        newPath.nodes = nodes;
        newPath.nodes.push_back(extension);
        return newPath;
    }
    friend bool operator>(const path& lhs, const path& rhs) {
        return lhs.length > rhs.length;
    }

};

class platform {
public:
    int xLength;
    int yLength;
    std::vector<node> nodes;
    std::pair<int, int> startCoords;
    platform(int initXLength, int initYLength) {
        xLength = initXLength;
        yLength = initYLength;
    }
    node getNode(int x, int y) {
        int index = x + y * xLength;
        if (index >= 0 && index < nodes.size()) {
            return nodes[index];
        }
        return node(-1, -1, 'L');
    }
    int getNodeIndex(int x, int y) {
        int nodeIndex = x + y * xLength;
        return nodeIndex;
    }
};

platform loadCharMap() {
    std::ifstream mapFile;
    std::string str;
    std::vector<std::string> map;
    mapFile.open("map.txt");
    while (std::getline(mapFile, str)) {
        map.push_back(str);
    }
    platform charMap = platform(map[0].size(), map.size());
    for (int i = 0; i < map.size(); i++) {
        for (int j = 0; j < map[i].size(); j++) {
            charMap.nodes.push_back(node(j, i, map[i][j]));
            if (map[i][j] == 'S') {
                charMap.startCoords = std::pair<int, int>(j, i);
            }
        }
    }
    return charMap;
}

path dijkstra(platform map, std::vector<sf::RectangleShape> &tileArray, std::vector<sf::Text> &typeArray, sf::RenderWindow &window) {

    std::priority_queue<path, std::vector<path>, std::greater<path>> pathQueue;

    int x = map.startCoords.first;
    int y = map.startCoords.second;
    node lastNodeInPath;

    int index;
    const int xLength = map.xLength;
    const int yLength = map.yLength;
    path currentPath = path();
    currentPath.nodes.push_back(map.getNode(x, y));
    pathQueue.push(currentPath);
    long nodeIndex;

    while (pathQueue.size() != 0) {
        currentPath = pathQueue.top();
        // std::cout << "Pathlen ";
        // std::cout << currentPath.length;
        // std::cout << "  ";
        pathQueue.pop();
        lastNodeInPath = currentPath.nodes.back();
        // std::cout << "NodeVal ";
        // std::cout << lastNodeInPath.type << std::endl;
        nodeIndex = lastNodeInPath.x + lastNodeInPath.y * xLength;
        map.nodes[nodeIndex].visited = true;

        if (map.nodes[nodeIndex].type != 'S' && map.nodes[nodeIndex].type != 'E') {
            tileArray[nodeIndex].setFillColor(sf::Color::Yellow);
        }

        window.clear();
        for (int i = 0; i < tileArray.size(); i++) {
            window.draw(tileArray[i]);
            window.draw(typeArray[i]);
        }
        window.display();
        
        if (lastNodeInPath.type == 'E') {
            int nodeIndex;
            for (int i = 0; i < currentPath.nodes.size(); i++) {
                nodeIndex = map.getNodeIndex(currentPath.nodes[i].x, currentPath.nodes[i].y);
                tileArray[nodeIndex].setFillColor(sf::Color::Blue);
            }
            return currentPath;
        }
        if (!lastNodeInPath.visited) {
            // If not at top edge
            if (lastNodeInPath.y != 0) {
                index = lastNodeInPath.x + xLength * (lastNodeInPath.y - 1);
                pathQueue.push(currentPath.extend(map.nodes[index]));
            }
            // If not at bottom edge
            if (lastNodeInPath.y != yLength - 1) {
                index = lastNodeInPath.x + xLength * (lastNodeInPath.y + 1);
                pathQueue.push(currentPath.extend(map.nodes[index]));
            }
            // If not on left edge
            if (lastNodeInPath.x != 0) {
                index = lastNodeInPath.x - 1 + xLength * lastNodeInPath.y;
                pathQueue.push(currentPath.extend(map.nodes[index]));
            }
            // If not on right edge
            if (lastNodeInPath.x != xLength - 1) {
                index = lastNodeInPath.x + 1 + xLength * lastNodeInPath.y;
                pathQueue.push(currentPath.extend(map.nodes[index]));
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }

    return path();
}

int main() {
    platform charMap = loadCharMap();

    // Red - Start tile
    // Green - End tile
    // White - Unvisited tiles
    // Yellow - Visited tiles
    // Blue - Path

    sf::Font calibriFont;
    calibriFont.loadFromFile("calibri.ttf");

    sf::RenderWindow window(sf::VideoMode(1000, 1000), "Dijkstra Visualizer");

    int tileSize = 50;

    std::vector<sf::RectangleShape> tileArray;
    std::vector<sf::Text> typeArray;

    for (int i = 0; i < charMap.nodes.size(); i++) {
        tileArray.push_back(sf::RectangleShape(sf::Vector2f(tileSize, tileSize)));
        tileArray.back().setPosition(sf::Vector2f(tileSize * (i % charMap.xLength), tileSize * (i/charMap.xLength)));
        tileArray.back().setOutlineThickness(1);
        tileArray.back().setOutlineColor(sf::Color::Black);
        if (charMap.nodes[i].type == 'S') {
            tileArray.back().setFillColor(sf::Color::Red);
        }
        else if (charMap.nodes[i].type == 'E') {
            tileArray.back().setFillColor(sf::Color::Green);
        }

        typeArray.push_back(sf::Text(charMap.nodes[i].type, calibriFont, 30));
        typeArray.back().setPosition(sf::Vector2f(tileSize * (i % charMap.xLength) + 10, tileSize * (i / charMap.xLength) + 10));
        typeArray.back().setFillColor(sf::Color::Black);
    }

    path idealPath = dijkstra(charMap, tileArray, typeArray, window);

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear();
        for (int i = 0; i < tileArray.size(); i++) {
            window.draw(tileArray[i]);
            window.draw(typeArray[i]);
        }
        window.display();
    }
}



