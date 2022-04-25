#pragma once

#include "ECS/ECS.h"

class EnemySystem : public System {
   void tick(World* world) override;
};
