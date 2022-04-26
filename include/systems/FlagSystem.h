#pragma once

#include "ECS/ECS.h"
#include "scenes/GameScene.h"

class GameScene;

class FlagSystem : public System {
  public:
   FlagSystem(GameScene* scene);

   void tick(World* world) override;

   static void setClimbing(bool val) {
      climbing = val;
   }

   static bool IsClimbing() {
      return climbing;
   }

  private:
   GameScene* scene;

   void climbFlag(Entity* player, Entity* flag);

   static bool climbing;
};
