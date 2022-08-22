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

   {
      pausedText = world->create();
      pausedText->addComponent<PositionComponent>(Vector2f(10.8, 4) * SCALED_CUBE_SIZE, Vector2i());
      pausedText->addComponent<TextComponent>("PAUSED", 18, false, false);
   }

   {
      continueText = world->create();
      continueText->addComponent<PositionComponent>(Vector2f(10.2, 8.5) * SCALED_CUBE_SIZE,
                                                    Vector2i());
      continueText->addComponent<TextComponent>("CONTINUE", 14, false, false);
   }

   {
      endText = world->create();
      endText->addComponent<PositionComponent>(Vector2f(10.2, 9.5) * SCALED_CUBE_SIZE, Vector2i());
      endText->addComponent<TextComponent>("END", 14, false, false);
   }

   {
      selectCursor = world->create();
      selectCursor->addComponent<PositionComponent>(Vector2f(9.5, 8.5) * SCALED_CUBE_SIZE,
                                                    Vector2i());
      selectCursor->addComponent<TextComponent>(">", 14, false, false);
   }

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

   setupLevel();

   //   scoreSystem->showTransitionEntities();
   //
   //   callbackSystem->setEnabled(false);
   //   physicsSystem->setEnabled(false);
   //   renderSystem->setTransitionRendering(true);
   //   loaderThread = SDL_CreateThread(preloadEntities, "MapLoader", (void*)mapSystem);
   //
   //   CommandScheduler::getInstance().addCommand(new DelayedCommand(
   //       [=]() {
   //          TextureManager::Get().SetBackgroundColor(getLevelData().levelBackgroundColor);
   //
   //          scoreSystem->hideTransitionEntities();
   //
   //          SDL_WaitThread(loaderThread, NULL);
   //          callbackSystem->setEnabled(true);
   //          physicsSystem->setEnabled(true);
   //          renderSystem->setTransitionRendering(false);
   //
   //          startTimer();
   //
   //          startLevelMusic();
   //
   //          playerSystem->reset();
   //          scoreSystem->reset();
   //       },
   //       3.0));
}

void GameScene::update() {
   world->tick();
   emptyCommandQueue();
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

void GameScene::handleInput(SDL_Event& event) {
   world->handleInput(event);

   if (event.type != SDL_KEYDOWN) {
      return;
   }

   switch (event.key.keysym.scancode) {
      case SDL_SCANCODE_ESCAPE:
         // Don't pause if it's during a transition or if the player can't be controlled
         if (renderSystem->isTransitionRendering() || !playerSystem->isInputEnabled()) {
            return;
         }
         paused = !paused;

         if (paused) {
            pause();
         } else {
            unpause();
         }
         break;
      case SDL_SCANCODE_RETURN:
         if (!paused) {
            break;
         }
         switch (pauseSelectedOption) {
            case 0:  // Continue
               paused = false;
               unpause();
               break;
            case 1:  // End
               paused = false;
               gameFinished = true;
               SoundManager::Get().stopMusic();
               break;
            default:
               break;
         }
         break;
      case SDL_SCANCODE_UP:
         if (!paused) {
            break;
         }

         if (pauseSelectedOption > 0) {
            pauseSelectedOption--;

            selectCursor->getComponent<PositionComponent>()->position.y =
                (8.5 + pauseSelectedOption) * SCALED_CUBE_SIZE;
         }
         break;
      case SDL_SCANCODE_DOWN:
         if (!paused) {
            break;
         }

         if (pauseSelectedOption < 1) {
            pauseSelectedOption++;

            selectCursor->getComponent<PositionComponent>()->position.y =
                (8.5 + pauseSelectedOption) * SCALED_CUBE_SIZE;
         }
         break;
      default:
         break;
   }
}

void GameScene::pause() {
   SoundManager::Get().pauseMusic();
   SoundManager::Get().pauseSounds();

   Entity* pauseSound(world->create());
   pauseSound->addComponent<SoundComponent>(SoundID::PAUSE);

   world->disableSystem<PhysicsSystem, PlayerSystem, AnimationSystem, EnemySystem,
                        CollectibleSystem, WarpSystem, FlagSystem, CallbackSystem, ScoreSystem>();

   pausedText->getComponent<TextComponent>()->setVisible(true);
   continueText->getComponent<TextComponent>()->setVisible(true);
   endText->getComponent<TextComponent>()->setVisible(true);
   selectCursor->getComponent<TextComponent>()->setVisible(true);

   scoreSystem->showTransitionEntities();
}

void GameScene::unpause() {
   SoundManager::Get().resumeMusic();
   SoundManager::Get().resumeSounds();

   world->enableSystem<PhysicsSystem, PlayerSystem, AnimationSystem, EnemySystem, CollectibleSystem,
                       WarpSystem, FlagSystem, CallbackSystem, ScoreSystem>();

   pausedText->getComponent<TextComponent>()->setVisible(false);
   continueText->getComponent<TextComponent>()->setVisible(false);
   endText->getComponent<TextComponent>()->setVisible(false);
   selectCursor->getComponent<TextComponent>()->setVisible(false);

   scoreSystem->hideTransitionEntities();

   pauseSelectedOption = 0;
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

   callbackSystem->setEnabled(false);
   physicsSystem->setEnabled(false);
   renderSystem->setTransitionRendering(true);
   loaderThread = SDL_CreateThread(preloadEntities, "MapLoader", (void*)mapSystem);

   CommandScheduler::getInstance().addCommand(new DelayedCommand(
       [=]() {
          TextureManager::Get().SetBackgroundColor(getLevelData().levelBackgroundColor);

          scoreSystem->hideTransitionEntities();

          SDL_WaitThread(loaderThread, NULL);
          callbackSystem->setEnabled(true);
          physicsSystem->setEnabled(true);
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
