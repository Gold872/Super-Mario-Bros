#pragma once

#include "ECS/Components.h"
#include "ECS/ECS.h"
#include "Math.h"

class EnemySystem : public System {
  public:
   void tick(World* world) override;

  private:
   void performBowserActions(World* world, Entity* entity);

   void performLakituActions(World* world, Entity* entity);

   void checkEnemyDestroyed(World* world, Entity* enemy);
};
