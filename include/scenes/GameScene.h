#pragma once

#include "Level.h"
#include "Map.h"
#include "Scene.h"
#include "systems/systems.h"

#include <functional>
#include <memory>

class PlayerSystem;  // Idk why this is needed, but it doesn't work without it
class MapSystem;
class ScoreSystem;
class SoundSystem;

class Level;
struct LevelData;

class GameScene : public Scene {
  public:
   GameScene(int level, int subLevel);

   void update() override;

   void loadLevel(int level, int subLevel);
   void loadEntities();

   void switchLevel(int level, int subLevel);
   void restartLevel();

   void startLevelMusic();
   void setLevelMusic(LevelType levelType);
   void stopMusic();

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

   enum class ScreenMode
   {
      NONE,
      GAME,
      TRANSITION,
      NEW_LEVEL,
   };

   std::shared_ptr<SDL_Texture> blockTexture;
   std::shared_ptr<SDL_Texture> enemyTexture;

  private:
   friend class MapSystem;
   friend class ScoreSystem;

   PlayerSystem* playerSystem;
   MapSystem* mapSystem;
   ScoreSystem* scoreSystem;
   SoundSystem* soundSystem;

   std::unique_ptr<Level> gameLevel = std::make_unique<Level>();

   int level;
   int subLevel;

   Map* enemiesMap;
   Map* foregroundMap;
   Map* undergroundMap;
   Map* backgroundMap;
   Map* aboveForegroundMap;

   std::vector<std::function<void(void)>> commandQueue;
};
