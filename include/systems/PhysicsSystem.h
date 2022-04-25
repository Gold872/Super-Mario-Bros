#pragma once

#include "ECS/ECS.h"

class PhysicsSystem : public System {
  public:
   PhysicsSystem() = default;

   void onAddedToWorld(World* world) override{};

   void tick(World* world) override;

   void onRemovedFromWorld(World* world) override{};
};
