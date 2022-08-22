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

   virtual void handleInput(SDL_Event& event) {
      world->handleInput(event);
   }

   virtual void handleInput(const Uint8* keystates) {
      world->handleInput(keystates);
   }
};
