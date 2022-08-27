#include <SFML/Graphics.hpp>
#include <chrono>
#include <fstream>
#include <iostream>
#include <limits.h>
#include <queue>
#include <string>
#include <thread>
#include <vector>

class node {
  public:
	int x;
	int y;
	char type;
	int distance = INT_MAX;
	int previousNode = -1;
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

class nodeDistancePair {
  public:
	int node;
	int distance;
	bool operator<(const nodeDistancePair &o) const {
		return distance < o.distance;
	}
	bool operator>(const nodeDistancePair &o) const {
		return distance > o.distance;
	}
};

using path = std::vector<int>;

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

	std::priority_queue<nodeDistancePair, std::vector<nodeDistancePair>, std::greater<nodeDistancePair>> nextNodes;
	const int xLength = map.xLength;
	const int yLength = map.yLength;

	{
		int x = map.startCoords.first;
		int y = map.startCoords.second;
		int index = map.getNodeIndex(x, y);
		map.nodes[index].distance = 0;
		nextNodes.push({index, map.nodes[index].distance});
	}

	while (!nextNodes.empty()) {
		auto [nodeIndex, distance] = nextNodes.top();
		nextNodes.pop();
		const node &currentNode = map.nodes[nodeIndex];
		if (distance > currentNode.distance)
			continue;

		if (currentNode.type != 'S' && currentNode.type != 'E') {
			tileArray[nodeIndex].setFillColor(sf::Color::Yellow);
		}

		window.clear();
		for (int i = 0; i < tileArray.size(); i++) {
			window.draw(tileArray[i]);
			window.draw(typeArray[i]);
		}
		window.display();

		if (map.nodes[nodeIndex].type == 'E') {
			// Paint the shortest path blue
			path shortestPath;
			while (nodeIndex != -1) {
				shortestPath.push_back(nodeIndex);
				nodeIndex = map.nodes[nodeIndex].previousNode;
			}
			reverse(shortestPath.begin(), shortestPath.end());
			for (int n : shortestPath) {
				tileArray[n].setFillColor(sf::Color::Blue);
			}
			return shortestPath;
		}
		// If not at top edge
		if (currentNode.y != 0) {
			int nextIndex = currentNode.x + xLength * (currentNode.y - 1);
			node &nextNode = map.nodes[nextIndex];
			int newDistance = currentNode.distance + (nextNode.type < 'E' ? nextNode.type - '0' : 0);
			if (newDistance < nextNode.distance) {
				nextNode.distance = newDistance;
				nextNode.previousNode = nodeIndex;
				nextNodes.push({nextIndex, nextNode.distance});
			}
		}
		// If not at bottom edge
		if (currentNode.y != yLength - 1) {
			int nextIndex = currentNode.x + xLength * (currentNode.y + 1);
			node &nextNode = map.nodes[nextIndex];
			int newDistance = currentNode.distance + (nextNode.type < 'E' ? nextNode.type - '0' : 0);
			if (newDistance < nextNode.distance) {
				nextNode.distance = newDistance;
				nextNode.previousNode = nodeIndex;
				nextNodes.push({nextIndex, nextNode.distance});
			}
		}
		// If not on left edge
		if (currentNode.x != 0) {
			int nextIndex = currentNode.x - 1 + xLength * currentNode.y;
			node &nextNode = map.nodes[nextIndex];
			int newDistance = currentNode.distance + (nextNode.type < 'E' ? nextNode.type - '0' : 0);
			if (newDistance < nextNode.distance) {
				nextNode.distance = newDistance;
				nextNode.previousNode = nodeIndex;
				nextNodes.push({nextIndex, nextNode.distance});
			}
		}
		// If not on right edge
		if (currentNode.x != xLength - 1) {
			int nextIndex = currentNode.x + 1 + xLength * currentNode.y;
			node &nextNode = map.nodes[nextIndex];
			int newDistance = currentNode.distance + (nextNode.type < 'E' ? nextNode.type - '0' : 0);
			if (newDistance < nextNode.distance) {
				nextNode.distance = newDistance;
				nextNode.previousNode = nodeIndex;
				nextNodes.push({nextIndex, nextNode.distance});
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
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
		tileArray.back().setPosition(sf::Vector2f(tileSize * (i % charMap.xLength), tileSize * (i / charMap.xLength)));
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

	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
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
