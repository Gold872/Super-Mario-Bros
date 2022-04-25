#pragma once

#include "ECS/ECS.h"

#include <SDL2/SDL.h>

class AnimationSystem : public System {
  public:
   AnimationSystem() = default;

   ~AnimationSystem() override = default;

   void onAddedToWorld(World* world) override;

   void tick(World* world) override;

  private:
};
