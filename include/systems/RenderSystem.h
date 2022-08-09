#pragma once

#include "ECS/ECS.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

class RenderSystem : public System {
  public:
   RenderSystem() = default;

   ~RenderSystem() override = default;

   void onAddedToWorld(World* world) override;

   void tick(World* world) override;

   void handleEvent(SDL_Event& event) override {}

   void onRemovedFromWorld(World* world) override {}

   void setTransitionRendering(bool transition);

  private:
   void renderEntity(Entity* entity, bool cameraBound = true);

   void renderText(Entity* entity, bool followCamera = false);

   bool transitionRendering = false;

   std::shared_ptr<TTF_Font> normalFont;
   std::shared_ptr<TTF_Font> smallFont;
};
