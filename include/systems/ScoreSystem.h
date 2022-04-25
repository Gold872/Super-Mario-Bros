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

   void showTransitionEntities();
   void hideTransitionEntities();

   void decreaseLives();

  private:
   Entity* scoreEntity;
   Entity* coinsEntity;
   Entity* timerEntity;
   Entity* worldNumberEntity;

   Entity* worldNumberTransition;
   Entity* marioIcon;
   Entity* livesText;

   int totalScore = 0;
   int coins = 0;
   int time = 255 * MAX_FPS;
   int gameTime = 255;
   int lives = 3;

   bool timerRunning = true;

   GameScene* scene;
};
