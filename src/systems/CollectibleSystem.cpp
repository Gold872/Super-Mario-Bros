#include "systems/CollectibleSystem.h"

#include "AABBCollision.h"
#include "Constants.h"
#include "ECS/Components.h"

#include <iostream>

void CollectibleSystem::tick(World* world) {
   world->find<CollectibleComponent>([&](Entity* entity) {
      auto* collectible = entity->getComponent<CollectibleComponent>();

      if (entity->hasComponent<LeftCollisionComponent>() &&
          entity->hasComponent<GravityComponent>()) {
         entity->getComponent<MovingComponent>()->velocity.x = COLLECTIBLE_SPEED;
      }
      if (entity->hasComponent<RightCollisionComponent>() &&
          entity->hasComponent<GravityComponent>()) {
         entity->getComponent<MovingComponent>()->velocity.x = -COLLECTIBLE_SPEED;
      }

      if (collectible->collectibleType == CollectibleType::SUPER_STAR) {
         if (entity->hasComponent<BottomCollisionComponent>()) {
            entity->getComponent<MovingComponent>()->velocity.y = -10.0;
         }
      }

      entity->remove<TopCollisionComponent, BottomCollisionComponent, LeftCollisionComponent,
                     RightCollisionComponent>();
   });
}
