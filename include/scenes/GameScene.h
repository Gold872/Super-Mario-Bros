#pragma once

#include "Level.h"
#include "Map.h"
#include "Scene.h"
#include "Systems/RenderSystem.h"
#include "systems/systems.h"

#include <SDL2/SDL_thread.h>

#include <functional>
#include <memory>

class PlayerSystem;  // Idk why this is needed, but it doesn't work without it
class MapSystem;
class ScoreSystem;
class SoundSystem;
class PhysicsSystem;
class RenderSystem;
class CallbackSystem;

class Level;
struct LevelData;

class GameScene : public Scene {
  public:
   GameScene(int level, int subLevel);

   void update() override;

   bool isFinished() override;

   void loadLevel(int level, int subLevel);

   int getLevel() {
      return level;
   }

   int getSublevel() {
      return subLevel;
   }

   LevelType getCurrentLevelType() {
      return currentLevelType;
   }

   void setCurrentLevelType(LevelType levelType) {
      currentLevelType = levelType;
   }

   void setupLevel();

   void switchLevel(int level, int subLevel);
   void restartLevel();

   void startLevelMusic();
   void setLevelMusic(LevelType levelType);
   void stopMusic();
   void resumeLastPlayedMusic();

   void stopTimer();
   void startTimer();

   void scoreCountdown();
   bool scoreCountdownFinished();

   void destroyWorldEntities();

   void setUnderwater(bool val);

   void emptyCommandQueue();

   LevelData& getLevelData() {
      return gameLevel->getData();
   }

   World* getWorld() {
      return world;
   }

   std::shared_ptr<SDL_Texture> blockTexture;
   std::shared_ptr<SDL_Texture> enemyTexture;

  private:
   friend class MapSystem;

   SDL_Thread* oldLoaderThread;

   PlayerSystem* playerSystem;
   MapSystem* mapSystem;
   ScoreSystem* scoreSystem;
   SoundSystem* soundSystem;
   PhysicsSystem* physicsSystem;
   RenderSystem* renderSystem;
   CallbackSystem* callbackSystem;

   std::unique_ptr<Level> gameLevel = std::make_unique<Level>();

   int level;
   int subLevel;

   MusicID currentMusicID;
   LevelType currentLevelType;

   Map enemiesMap;
   Map foregroundMap;
   Map undergroundMap;
   Map backgroundMap;
   Map aboveForegroundMap;
   Map collectiblesMap;

   std::vector<std::function<void(void)>> commandQueue;
};
