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

   static bool isClimbing() {
      return climbing;
   }

  private:
   GameScene* scene;

   void climbFlag(World* world, Entity* player, Entity* flag);

   void hitAxe(World* world, Entity* player, Entity* axe);

   static bool climbing;
};
