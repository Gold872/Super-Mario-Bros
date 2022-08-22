#include "scenes/GameScene.h"

#include "AABBCollision.h"
#include "Constants.h"
#include "ECS/Components.h"
#include "Level.h"
#include "Map.h"
#include "Math.h"
#include "SoundManager.h"
#include "TextureManager.h"
#include "command/CommandScheduler.h"
#include "command/Commands.h"
#include "systems/Systems.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

static int preloadEntities(void* data) {
   MapSystem* mapSystem = (MapSystem*)data;

   mapSystem->loadEntities();

   return 0;
}

GameScene::GameScene(int level, int subLevel) {
   this->level = level;
   this->subLevel = subLevel;

   Map::loadBlockIDS();
   Map::loadEnemyIDS();
   Map::loadPlayerIDS();
   Map::loadIrregularBlockReferences();

   blockTexture = TextureManager::Get().LoadSharedTexture("res/sprites/blocks/BlockTileSheet.png");
   enemyTexture =
       TextureManager::Get().LoadSharedTexture("res/sprites/characters/EnemySpriteSheet.png");

   foregroundMap = Map();
   backgroundMap = Map();
   undergroundMap = Map();
   enemiesMap = Map();
   aboveForegroundMap = Map();

   loadLevel(level, subLevel);

   TextureManager::Get().SetBackgroundColor(BackgroundColor::BLACK);

   mapSystem = world->registerSystem<MapSystem>(this);
   physicsSystem = world->registerSystem<PhysicsSystem>();
   playerSystem = world->registerSystem<PlayerSystem>(this);
   world->registerSystem<AnimationSystem>();
   world->registerSystem<EnemySystem>();
   world->registerSystem<CollectibleSystem>();
   world->registerSystem<WarpSystem>(this);
   world->registerSystem<FlagSystem>(this);
   callbackSystem = world->registerSystem<CallbackSystem>();
   scoreSystem = world->registerSystem<ScoreSystem>(this);
   soundSystem = world->registerSystem<SoundSystem>();
   renderSystem = world->registerSystem<RenderSystem>();

   scoreSystem->showTransitionEntities();

   callbackSystem->setActive(false);
   physicsSystem->setActive(false);
   renderSystem->setTransitionRendering(true);
   loaderThread = SDL_CreateThread(preloadEntities, "MapLoader", (void*)mapSystem);

   CommandScheduler::getInstance().addCommand(new DelayedCommand(
       [=]() {
          TextureManager::Get().SetBackgroundColor(getLevelData().levelBackgroundColor);

          scoreSystem->hideTransitionEntities();

          SDL_WaitThread(loaderThread, NULL);
          callbackSystem->setActive(true);
          physicsSystem->setActive(true);
          renderSystem->setTransitionRendering(false);

          startTimer();

          startLevelMusic();

          playerSystem->reset();
          scoreSystem->reset();
       },
       3.0));
}

void GameScene::update() {
   world->tick();
   emptyCommandQueue();
   //   if (scoreSystem->getGameTime() <= 0) {
   //      SDL_WaitThread(loaderThread, NULL);
   //
   //      gameFinished = true;
   //   }
   //   std::cout << "Enemy Texture count: " << enemyTexture.use_count() << '\n';
}

void GameScene::emptyCommandQueue() {
   for (auto command : commandQueue) {
      command();
   }
   commandQueue.clear();
}

bool GameScene::isFinished() {
   return gameFinished;
}

void GameScene::setupLevel() {
   destroyWorldEntities();

   enemiesMap.reset();
   foregroundMap.reset();
   undergroundMap.reset();
   backgroundMap.reset();
   aboveForegroundMap.reset();
   collectiblesMap.reset();

   WarpSystem::setClimbed(false);

   gameLevel->clearLevelData();

   loadLevel(level, subLevel);

   TextureManager::Get().SetBackgroundColor(BackgroundColor::BLACK);
   scoreSystem->showTransitionEntities();

   scoreSystem->reset();

   callbackSystem->setActive(false);
   physicsSystem->setActive(false);
   renderSystem->setTransitionRendering(true);
   loaderThread = SDL_CreateThread(preloadEntities, "MapLoader", (void*)mapSystem);

   CommandScheduler::getInstance().addCommand(new DelayedCommand(
       [=]() {
          TextureManager::Get().SetBackgroundColor(getLevelData().levelBackgroundColor);

          scoreSystem->hideTransitionEntities();

          SDL_WaitThread(loaderThread, NULL);
          callbackSystem->setActive(true);
          physicsSystem->setActive(true);
          renderSystem->setTransitionRendering(false);

          startTimer();

          startLevelMusic();

          playerSystem->reset();
       },
       3.0));
}

void GameScene::switchLevel(int level, int subLevel) {
   commandQueue.push_back([=]() {
      this->level = level;
      this->subLevel = subLevel;

      //      std::cout << "Number of Entities: " << world->getEntities().size() << '\n';
      setupLevel();
   });
}

void GameScene::restartLevel() {
   commandQueue.push_back([=]() {
      scoreSystem->reset();
      scoreSystem->decreaseLives();

      if (scoreSystem->getLives() == 0) {
         gameFinished = true;
         return;
      }

      setupLevel();
   });
}

void GameScene::startLevelMusic() {
   setLevelMusic(getLevelData().levelType);
}

void GameScene::setLevelMusic(LevelType levelType) {
   switch (levelType) {
      case LevelType::OVERWORLD:
      case LevelType::START_UNDERGROUND: {
         Entity* overworldMusic(world->create());
         overworldMusic->addComponent<MusicComponent>(currentMusicID = MusicID::OVERWORLD);
      } break;
      case LevelType::UNDERGROUND: {
         Entity* undergroundMusic(world->create());
         undergroundMusic->addComponent<MusicComponent>(currentMusicID = MusicID::UNDERGROUND);
      } break;
      case LevelType::CASTLE: {
         Entity* castleMusic(world->create());
         castleMusic->addComponent<MusicComponent>(currentMusicID = MusicID::CASTLE);
      } break;
      case LevelType::UNDERWATER: {
         Entity* underwaterMusic(world->create());
         underwaterMusic->addComponent<MusicComponent>(currentMusicID = MusicID::UNDERWATER);
      } break;
      default:
         break;
   }
}

void GameScene::resumeLastPlayedMusic() {
   Entity* music(world->create());
   music->addComponent<MusicComponent>(currentMusicID);
}

void GameScene::stopMusic() {
   SoundManager::Get().stopMusic();
}

void GameScene::stopTimer() {
   scoreSystem->stopTimer();
}

void GameScene::startTimer() {
   scoreSystem->startTimer();
}

int GameScene::getTimeLeft() {
   return scoreSystem->getGameTime();
}

void GameScene::scoreCountdown() {
   scoreSystem->scoreCountdown(world);
}

bool GameScene::scoreCountdownFinished() {
   return scoreSystem->scoreCountFinished();
}

void GameScene::destroyWorldEntities() {
   std::vector<Entity*> entities = world->getEntities();
   for (auto* entity : entities) {
      if (!entity->hasAny<PlayerComponent, IconComponent>()) {
         if (entity->hasComponent<TextComponent>() &&
             !entity->hasComponent<FloatingTextComponent>()) {
            continue;
         }
         world->destroy(entity);
      }
   }
   entities.shrink_to_fit();
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
   std::string collectiblesPath = mapDataPath + "_Collectibles.csv";

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

   foregroundMap.loadMap(foregroundPath.c_str());
   backgroundMap.loadMap(backgroundPath.c_str());
   undergroundMap.loadMap(undergroundPath.c_str());
   enemiesMap.loadMap(enemiesPath.c_str());
   aboveForegroundMap.loadMap(aboveForegroundPath.c_str());
   collectiblesMap.loadMap(collectiblesPath.c_str());
}

void GameScene::setUnderwater(bool val) {
   playerSystem->setUnderwater(val);
}
