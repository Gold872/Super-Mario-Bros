#include "Map.h"

#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

std::unordered_map<int, Vector2i> Map::BlockIDCoordinates;
std::unordered_map<int, Vector2i> Map::PlayerIDCoordinates;
std::unordered_map<int, Vector2i> Map::EnemyIDCoordinates;
//std::unordered_map<int, int> Map::DeadEnemyIDCoordinates{{70, 2}, {175, 2}, {280, 2}, {385, 2},
//                                                         {38, 6}, {143, 6}, {248, 6}, {353, 6}};

Map::Map() {}

Map::Map(const char* dataPath) {
   loadMap(dataPath);
}

void Map::loadMap(const char* dataPath) {
   std::string line, word;

   std::ifstream levelDataFile(dataPath);

   while (getline(levelDataFile, line)) {
      std::vector<int> row;
      std::stringstream ss(line);

      while (getline(ss, word, ',')) {
         row.push_back(std::stoi(word));
      }
      levelData.push_back(row);
   }
   levelDataFile.close();
}

void Map::reset() {
	levelData.clear();
}

void Map::loadBlockIDS() {
   int blockID = 0;
   for (int i = 0; i < 22; i++) {
      for (int j = 0; j < 48; j++) {
         BlockIDCoordinates.insert({blockID, Vector2i(j, i)});
         blockID++;
      }
   }
}

void Map::loadPlayerIDS() {
   int playerID = 0;
   for (int i = 0; i < 16; i++) {
      for (int j = 0; j < 25; j++) {
         PlayerIDCoordinates.insert({playerID, Vector2i(j, i)});
         playerID++;
      }
   }
}

void Map::loadEnemyIDS() {
   int enemyID = 0;
   for (int i = 0; i < 15; i++) {
      for (int j = 0; j < 35; j++) {
         EnemyIDCoordinates.insert({enemyID, Vector2i(j, i)});
         enemyID++;
      }
   }
}

vector<vector<int>> Map::getLevelData() {
   return levelData;
}
