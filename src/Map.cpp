#include "Map.h"

#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

std::unordered_map<int, Vector2i> Map::BlockIDCoordinates;
std::unordered_map<int, Vector2i> Map::PlayerIDCoordinates;
std::unordered_map<int, Vector2i> Map::EnemyIDCoordinates;
std::unordered_map<int, int> Map::IrregularBlockReferences;

Map::Map() {}

Map::Map(const char* dataPath) {
   loadMap(dataPath);
}

void Map::loadMap(const char* dataPath) {
   std::string line, word;

   std::ifstream levelDataFile(dataPath);
   if (levelDataFile.is_open()) {
      while (getline(levelDataFile, line)) {
         std::vector<int> row;
         std::stringstream ss(line);

         while (getline(ss, word, ',')) {
            row.push_back(std::stoi(word));
         }
         levelData.push_back(row);
      }
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
         BlockIDCoordinates.insert_or_assign(blockID, Vector2i(j, i));
         blockID++;
      }
   }
}

void Map::loadPlayerIDS() {
   int playerID = 0;
   for (int i = 0; i < 16; i++) {
      for (int j = 0; j < 25; j++) {
         PlayerIDCoordinates.insert_or_assign(playerID, Vector2i(j, i));
         playerID++;
      }
   }
}

void Map::loadEnemyIDS() {
   int enemyID = 0;
   for (int i = 0; i < 15; i++) {
      for (int j = 0; j < 35; j++) {
         EnemyIDCoordinates.insert_or_assign(enemyID, Vector2i(j, i));
         enemyID++;
      }
   }
}

void Map::loadIrregularBlockReferences() {
   std::string line;

   std::ifstream idsFile("res/sprites/blocks/IrregularReferences.blockmap");

   std::regex pairRegex("(\\d+),\\s(\\d+)");

   std::smatch pairMatch;
   while (getline(idsFile, line)) {
      int blockID = 0, referenceID = 0;

      if (std::regex_search(line, pairMatch, pairRegex)) {
         blockID = std::stoi(pairMatch[1]);
         referenceID = std::stoi(pairMatch[2]);
      }
      IrregularBlockReferences.insert_or_assign(blockID, referenceID);
   }
}

vector<vector<int>> Map::getLevelData() {
   return levelData;
}
