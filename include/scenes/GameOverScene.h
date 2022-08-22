#pragma once

#include "ECS/ECS.h"
#include "scenes/Scene.h"

class GameOverScene : public Scene {
  public:
	GameOverScene();

   void update() override;

   bool isFinished() override;

  private:
   Entity* gameOverText;

   int timer = 0;
};
