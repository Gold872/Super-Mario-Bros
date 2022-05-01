#pragma once

#include "ECS/ECS.h"

class SoundSystem : public System {
  public:
   void tick(World* world) override;
};
