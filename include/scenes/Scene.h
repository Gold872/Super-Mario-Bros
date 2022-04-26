#pragma once

#include "ECS/ECS.h"

#include <SDL2/SDL.h>

#include <memory>

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

   virtual void handleEvents(SDL_Event& event) {
      world->handleEvent(event);
   }

   virtual void handleEvents(const Uint8* keystates) {
      world->handleEvent(keystates);
   }
};
