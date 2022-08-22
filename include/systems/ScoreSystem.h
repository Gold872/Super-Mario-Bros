#pragma once

#include "Constants.h"
#include "ECS/ECS.h"
#include "scenes/GameScene.h"

class GameScene;

class ScoreSystem : public System {
  public:
   ScoreSystem() = default;

   ScoreSystem(GameScene* scene);

   void onAddedToWorld(World* world) override;

   void tick(World* world) override;

   void reset();

   void startTimer();
   void stopTimer();

   void scoreCountdown(World* world);

   bool scoreCountFinished();

   int getGameTime();

   int getLives();

   void showTransitionEntities();
   void hideTransitionEntities();

   void decreaseLives();

  private:
   Entity* createFloatingText(World* world, Entity* originalEntity, std::string text);

   Entity* scoreEntity;
   Entity* coinsEntity;
   Entity* timerEntity;
   Entity* worldNumberEntity;

   Entity* worldNumberTransition;
   Entity* marioIcon;
   Entity* livesText;

   int totalScore = 0;
   int coins = 0;
   int time = 400 * MAX_FPS;
   int gameTime = 400;
   int scoreCountTime = 0;
   int lives = 3;

   bool timerRunning = false;

   GameScene* scene;
};
