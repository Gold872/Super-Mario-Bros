#pragma once

#include "ECS/ECS.h"

#include <SDL2/SDL.h>

class Scene {
  protected:
   World* world = new World();

  public:
   virtual ~Scene() {
      delete world;
   }

   virtual void update() {
      world->tick();
   }

   virtual bool isFinished() {
      return true;
   }

   virtual void handleInput() {
      world->handleInput();
   }
};
