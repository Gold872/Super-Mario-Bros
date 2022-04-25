#pragma once

#include "ECS/ECS.h"
#include "scenes/GameScene.h"

#include <SDL2/SDL.h>

class GameScene;

class WarpSystem : public System {
  public:
   WarpSystem(GameScene* scene);

   void onAddedToWorld(World* world) override;

   void tick(World* world) override;

   void handleEvent(SDL_Event& event) override;

   void warp(World* world, Entity* pipe, Entity* player);

   static bool isWarping() {
      return warping;
   }

   static void setWarping(bool val) {
      warping = val;
   }

   int up = 0;
   int down = 0;
   int left = 0;
   int right = 0;

  private:
   static bool warping;

   GameScene* scene;
};
