#pragma once

#include "Math.h"

#include <unordered_map>
#include <vector>

/*
 * The Level class will store all data related to the position of every object on the level.
 * The levels get loaded from an external file.
 * */

using std::vector;

class Map {
  public:
   Map();
   Map(const char* dataPath);

   ~Map() = default;

   void loadMap(const char* dataPath);

   void reset();

   static void loadBlockIDS();
   static void loadPlayerIDS();
   static void loadEnemyIDS();
   static void loadIrregularBlockReferences();

   vector<vector<int>> getLevelData();

   static std::unordered_map<int, Vector2i> BlockIDCoordinates;
   static std::unordered_map<int, Vector2i> PlayerIDCoordinates;
   static std::unordered_map<int, Vector2i> EnemyIDCoordinates;
   static std::unordered_map<int, int> DeadEnemyIDCoordinates;
   static std::unordered_map<int, int> IrregularBlockReferences;

  private:
   vector<vector<int>> levelData;
};
