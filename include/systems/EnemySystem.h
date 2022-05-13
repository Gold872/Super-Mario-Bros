#pragma once

#include "ECS/ECS.h"

class EnemySystem : public System {
  public:
   void tick(World* world) override;

  private:
   void performBowserActions(World* world, Entity* entity);

   void checkEnemyDestroyed(World* world, Entity* enemy);
};
