#pragma once

#include "ECS/ECS.h"

class CollectibleSystem : public System {
  public:
   void tick(World* world) override;
};
