#pragma once

#include "ECS/ECS.h"

class EnemySystem : public System {
  public:
   void tick(World* world) override;
};
