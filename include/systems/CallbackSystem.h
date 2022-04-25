#pragma once

#include "ECS/ECS.h"

class CallbackSystem : public System {
  public:
   void tick(World* world) override;
};
