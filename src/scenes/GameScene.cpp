#include "scenes/GameScene.h"

#include "AABBCollision.h"
#include "Constants.h"
#include "ECS/Components.h"
#include "Level.h"
#include "Map.h"
#include "Math.h"
#include "TextureManager.h"
#include "systems/Systems.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

GameScene::GameScene(int level, int subLevel) {
   this->level = level;
   this->subLevel = subLevel;

   Map::loadBlockIDS();
   Map::loadEnemyIDS();
   Map::loadPlayerIDS();

   blockTexture = TextureManager::LoadSharedTexture("res/sprites/blocks/BlockTileSheet.png");
   enemyTexture = TextureManager::LoadSharedTexture("res/sprites/characters/EnemySpriteSheet.png");

   foregroundMap = gameLevel->createMap();
   backgroundMap = gameLevel->createMap();
   undergroundMap = gameLevel->createMap();
   enemiesMap = gameLevel->createMap();
   aboveForegroundMap = gameLevel->createMap();

   loadLevel(level, subLevel);

   TextureManager::SetBackgroundColor(getLevelData().levelBackgroundColor);

   mapSystem = world->registerSystem<MapSystem>(this);
   world->registerSystem<PhysicsSystem>();
   world->registerSystem<AnimationSystem>();
   playerSystem = world->registerSystem<PlayerSystem>(this);
   world->registerSystem<EnemySystem>();
   world->registerSystem<CallbackSystem>();
   world->registerSystem<CollectibleSystem>();
   world->registerSystem<WarpSystem>(this);
   world->registerSystem<FlagSystem>(this);
   scoreSystem = world->registerSystem<ScoreSystem>(this);
   world->registerSystem<RenderSystem>();

   mapSystem->loadEntities(world);

   playerSystem->reset();
   scoreSystem->reset();
}

void GameScene::update() {
   world->tick();
   emptyCommandQueue();
}

void GameScene::emptyCommandQueue() {
   for (auto command : commandQueue) {
      command();
   }
   commandQueue.clear();
}

void GameScene::switchLevel(int level, int subLevel) {
   commandQueue.push_back([=]() {
      this->level = level;
      this->subLevel = subLevel;

      //      std::cout << "Number of Entities: " << world->getEntities().size() << '\n';
      destroyWorldEntities();

      enemiesMap->reset();
      foregroundMap->reset();
      undergroundMap->reset();
      backgroundMap->reset();
      aboveForegroundMap->reset();

      gameLevel->clearLevelData();

      loadLevel(level, subLevel);

      TextureManager::SetBackgroundColor(BackgroundColor::BLACK);
      scoreSystem->showTransitionEntities();

      scoreSystem->reset();

      Entity* callbackEntity(world->create());
      callbackEntity->addComponent<CallbackComponent>(
          [=](Entity* entity) {
             TextureManager::SetBackgroundColor(getLevelData().levelBackgroundColor);

             scoreSystem->hideTransitionEntities();

             mapSystem->loadEntities(world);

             startTimer();

             playerSystem->reset();
          },
          180);
   });
}

void GameScene::restartLevel() {
   commandQueue.push_back([=]() {
      destroyWorldEntities();

      enemiesMap->reset();
      foregroundMap->reset();
      undergroundMap->reset();
      backgroundMap->reset();
      aboveForegroundMap->reset();

      gameLevel->clearLevelData();

      loadLevel(level, subLevel);

      TextureManager::SetBackgroundColor(BackgroundColor::BLACK);
      scoreSystem->showTransitionEntities();

      scoreSystem->reset();
      scoreSystem->decreaseLives();

      Entity* callbackEntity(world->create());
      callbackEntity->addComponent<CallbackComponent>(
          [=](Entity* entity) {
             TextureManager::SetBackgroundColor(getLevelData().levelBackgroundColor);

             scoreSystem->hideTransitionEntities();

             mapSystem->loadEntities(world);

             startTimer();

             playerSystem->reset();
          },
          180);
   });
}

void GameScene::stopTimer() {
   scoreSystem->stopTimer();
}

void GameScene::startTimer() {
   scoreSystem->startTimer();
}

void GameScene::destroyWorldEntities() {
   for (auto* entity : world->getEntities()) {
      if (!entity->hasAny<PlayerComponent, TextComponent, IconComponent>()) {
         world->destroy(entity);
      }
   }
   world->getEntities().shrink_to_fit();
}

void GameScene::loadLevel(int level, int subLevel) {
   // The path to the folder that the level files are in
   std::string folderPath =
       "res/data/World" + std::to_string(level) + "-" + std::to_string(subLevel) + "/";
   // The path to the map data files (where the blocks are and stuff)
   std::string mapDataPath =
       folderPath + "World" + std::to_string(level) + "-" + std::to_string(subLevel);

   std::string foregroundPath = mapDataPath + "_Foreground.csv";
   std::string backgroundPath = mapDataPath + "_Background.csv";
   std::string undergroundPath = mapDataPath + "_Underground.csv";
   std::string enemiesPath = mapDataPath + "_Enemies.csv";
   std::string aboveForegroundPath = mapDataPath + "_Above_Foreground.csv";

   // Loads the special level properties
   std::ifstream properties(mapDataPath + ".levelproperties");

   std::string propertiesString;

   if (properties.is_open()) {
      std::ostringstream ss;
      ss << properties.rdbuf();
      propertiesString = ss.str();
   }

   properties.close();

   gameLevel->loadLevelData(propertiesString);

   foregroundMap->loadMap(foregroundPath.c_str());
   backgroundMap->loadMap(backgroundPath.c_str());
   undergroundMap->loadMap(undergroundPath.c_str());
   enemiesMap->loadMap(enemiesPath.c_str());
   aboveForegroundMap->loadMap(aboveForegroundPath.c_str());
}

void GameScene::setUnderwater(bool val) {
   playerSystem->setUnderwater(val);
}
