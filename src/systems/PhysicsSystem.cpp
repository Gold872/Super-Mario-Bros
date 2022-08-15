#include "AABBCollision.h"
#include "Camera.h"
#include "Constants.h"
#include "ECS/Components.h"
#include "systems/PhysicsSystem.h"

#include <cmath>
#include <iostream>
CollisionDirection checkCollisionY(Entity* solid, PositionComponent* position,
                                   MovingComponent* move, bool adjustPosition = true) {
   auto* solidPosition = solid->getComponent<PositionComponent>();
   CollisionDirection direction = CollisionDirection::NONE;

   if (move->velocity.y >= 0.0f) {
      // If falling
      if (AABBTotalCollision(position->position.x + position->hitbox.x + (TILE_ROUNDNESS / 2),
                             position->position.y + position->hitbox.y + move->velocity.y,
                             position->hitbox.w - TILE_ROUNDNESS, position->hitbox.h,
                             solidPosition->position.x + (TILE_ROUNDNESS / 2),
                             solidPosition->position.y, solidPosition->hitbox.w - TILE_ROUNDNESS,
                             solidPosition->hitbox.h)) {
         float topDistance =
             std::abs(solidPosition->getTop() - (position->getBottom() + move->velocity.y));
         float bottomDistance =
             std::abs((position->getTop() + move->velocity.y) - solidPosition->getBottom());

         if (topDistance < bottomDistance) {
            if (adjustPosition) {
               position->setBottom(solidPosition->getTop());
            }
            solid->addComponent<TopCollisionComponent>();
            direction = CollisionDirection::BOTTOM;
         }
      }
   } else {
      // Jumping
      if (AABBTotalCollision(position->position.x + position->hitbox.x + TILE_ROUNDNESS,
                             position->position.y + position->hitbox.y + move->velocity.y,
                             position->hitbox.w - (TILE_ROUNDNESS * 2), position->hitbox.h,
                             solidPosition->position.x + TILE_ROUNDNESS, solidPosition->position.y,
                             solidPosition->hitbox.w - (TILE_ROUNDNESS * 2),
                             solidPosition->hitbox.h)) {
         float topDistance =
             std::abs(solidPosition->getTop() - (position->getBottom() + move->velocity.y));
         float bottomDistance =
             std::abs((position->getTop() + move->velocity.y) - solidPosition->getBottom());
         if (topDistance > bottomDistance) {
            if (adjustPosition) {
               position->setTop(solidPosition->getBottom());
            }
            solid->addComponent<BottomCollisionComponent>();
            direction = CollisionDirection::TOP;
         }
      }
   }
   return direction;
}

CollisionDirection checkCollisionX(Entity* solid, PositionComponent* position,
                                   MovingComponent* move, bool adjustPosition = true) {
   auto* solidPosition = solid->getComponent<PositionComponent>();
   CollisionDirection direction = CollisionDirection::NONE;

   if (AABBTotalCollision(position->position.x + position->hitbox.x + move->velocity.x,
                          position->position.y + position->hitbox.y, position->hitbox.w,
                          position->hitbox.h - (TILE_ROUNDNESS * 2), solidPosition)) {
      float leftDistance = std::abs((position->position.x + position->hitbox.x + move->velocity.x) -
                                    solidPosition->getRight());
      float rightDistance = std::abs(
          (position->position.x + position->hitbox.x + position->hitbox.w + move->velocity.x) -
          solidPosition->getLeft());
      if (leftDistance < rightDistance) {
         if (adjustPosition) {
            // Entity is inside block, push out
            if (position->getLeft() < solidPosition->getRight()) {
               position->position.x +=
                   std::min(0.5f, solidPosition->getRight() - position->getLeft());
            } else {
               // The entity is about to get inside the block
               position->setLeft(solidPosition->getRight());
            }
         }
         solid->addComponent<RightCollisionComponent>();
         direction = CollisionDirection::LEFT;
      } else {
         if (adjustPosition) {
            // Entity is inside block, push out
            if (position->getRight() > solidPosition->getLeft()) {
               position->position.x -=
                   std::min(0.5f, position->getRight() - solidPosition->getLeft());
            } else {
               // Entity is about to get inside the block
               position->setRight(solidPosition->getLeft());
            }
         }
         solid->addComponent<LeftCollisionComponent>();
         direction = CollisionDirection::RIGHT;
      }
   }

   return direction;
}

void PhysicsSystem::updateFireBars(World* world) {
   world->find<FireBarComponent, PositionComponent>([&](Entity* entity) {
      auto* fireBar = entity->getComponent<FireBarComponent>();
      auto* position = entity->getComponent<PositionComponent>();

      if (fireBar->barAngle > 360) {
         fireBar->barAngle -= 360;
      } else if (fireBar->barAngle < 0) {
         fireBar->barAngle += 360;
      }

      position->position.x =
          fireBar->calculateXPosition(fireBar->barAngle) + fireBar->pointOfRotation.x;
      position->position.y =
          -fireBar->calculateYPosition(fireBar->barAngle) + fireBar->pointOfRotation.y;
   });
}

void PhysicsSystem::updateMovingPlatforms(World* world) {
   world->find<MovingPlatformComponent, MovingComponent, PositionComponent>([&](Entity* entity) {
      auto* platform = entity->getComponent<MovingPlatformComponent>();
      auto* platformMove = entity->getComponent<MovingComponent>();
      auto* position = entity->getComponent<PositionComponent>();

      switch (platform->motionType) {
         case PlatformMotionType::ONE_DIRECTION_REPEATED:
            switch (platform->movingDirection) {
               case Direction::LEFT:
               case Direction::RIGHT:
                  if (position->position.x < platform->minPoint) {
                     position->position.x = platform->maxPoint;
                  } else if (position->position.x > platform->maxPoint) {
                     position->position.x = platform->minPoint;
                  }
                  break;
               case Direction::UP:
               case Direction::DOWN:
                  if (position->position.y > platform->maxPoint) {
                     position->position.y = platform->minPoint;
                  } else if (position->position.y < platform->minPoint) {
                     position->position.y = platform->maxPoint;
                  }
                  break;
               default:
                  break;
            }
            break;
         case PlatformMotionType::BACK_AND_FORTH:
            switch (platform->movingDirection) {
               case Direction::LEFT:
                  if (position->getLeft() <= platform->minPoint) {
                     platform->movingDirection = Direction::RIGHT;
                     break;
                  }
                  {
                     float newVelocity = -platform->calculateVelocity(
                         position->getRight() - platform->minPoint,
                         (platform->maxPoint - platform->minPoint) / 3.8);

                     platformMove->velocity.x = newVelocity;
                  }
                  break;
               case Direction::RIGHT:
                  if (position->getRight() >= platform->maxPoint) {
                     platform->movingDirection = Direction::LEFT;
                     break;
                  }
                  {
                     float newVelocity = platform->calculateVelocity(
                         platform->maxPoint - position->getLeft(),
                         (platform->maxPoint - platform->minPoint) / 3.8);

                     platformMove->velocity.x = newVelocity;
                  }
                  break;
               case Direction::UP:
                  if (position->getTop() <= platform->minPoint) {
                     platform->movingDirection = Direction::DOWN;
                     break;
                  }
                  {
                     float newVelocity = -platform->calculateVelocity(
                         position->getBottom() - platform->minPoint,
                         (platform->maxPoint - platform->minPoint) / 3.8);

                     platformMove->velocity.y = newVelocity;
                  }
                  break;
               case Direction::DOWN:
                  if (position->getBottom() >= platform->maxPoint) {
                     platform->movingDirection = Direction::UP;
                     break;
                  }
                  {
                     float newVelocity = platform->calculateVelocity(
                         platform->maxPoint - position->getTop(),
                         (platform->maxPoint - platform->minPoint) / 3.8);

                     platformMove->velocity.y = newVelocity;
                  }
                  break;
               default:
                  break;
            }
            break;
         case PlatformMotionType::GRAVITY: {
            if (entity->hasComponent<TopCollisionComponent>()) {
               platformMove->acceleration.y = 0.10;
            } else {
               platformMove->acceleration.y = 0;
               platformMove->velocity.y *= 0.92;
            }

            entity->remove<TopCollisionComponent>();
         } break;
         default:
            break;
      }
   });
}

void PhysicsSystem::updatePlatformLevels(World* world) {
   world->find<PlatformLevelComponent>([](Entity* entity) {
      auto* platformLevel = entity->getComponent<PlatformLevelComponent>();
      auto* platformPosition = entity->getComponent<PositionComponent>();
      auto* platformMove = entity->getComponent<MovingComponent>();

      if (!Camera::Get().inCameraRange(platformPosition)) {
         return;
      }

      auto* pulleyPosition = platformLevel->pulleyLine->getComponent<PositionComponent>();

      pulleyPosition->scale.y = platformPosition->getTop() - pulleyPosition->getTop();

      Entity* otherPlatform = platformLevel->getOtherPlatform();

      if (platformPosition->getTop() < platformLevel->pulleyHeight) {
         platformPosition->setTop(platformLevel->pulleyHeight);
         platformMove->velocity = Vector2f(0, 0);

         otherPlatform->getComponent<MovingComponent>()->acceleration.y = 0;
         otherPlatform->addComponent<GravityComponent>();
         otherPlatform->addComponent<CollisionExemptComponent>();
         otherPlatform->addComponent<DestroyOutsideCameraComponent>();

         otherPlatform->remove<PlatformLevelComponent>();
         entity->remove<PlatformLevelComponent>();

         return;
      }

      if (!entity->hasComponent<TopCollisionComponent>()) {
         // Slows the platform down if the other platform isn't accelerating
         if (otherPlatform->getComponent<MovingComponent>()->acceleration.y == 0) {
            platformMove->velocity.y *= 0.92;

            // Sets the 2 platforms to have opposite velocities
            otherPlatform->getComponent<MovingComponent>()->velocity.y = -platformMove->velocity.y;
         }

         platformMove->acceleration.y = 0;
         return;
      }

      platformMove->acceleration.y = 0.12f;

      // Sets the 2 platforms to have opposite velocities
      otherPlatform->getComponent<MovingComponent>()->velocity.y = -platformMove->velocity.y;

      entity->remove<TopCollisionComponent>();
   });
}

void PhysicsSystem::tick(World* world) {
   // Update gravity for entities that have a gravity component
   world->find<GravityComponent, MovingComponent>([&](Entity* entity) {
      if ((!Camera::Get().inCameraRange(entity->getComponent<PositionComponent>()) &&
           !entity->hasAny<MoveOutsideCameraComponent, PlayerComponent>()) ||
          entity->hasComponent<FrozenComponent>()) {
         return;
      }
      entity->getComponent<MovingComponent>()->velocity.y += 0.575;
   });

   // Change the y position of the block being bumped
   world->find<BlockBumpComponent, PositionComponent>([&](Entity* entity) {
      auto blockBump = entity->getComponent<BlockBumpComponent>();

      entity->getComponent<PositionComponent>()->position.y +=
          blockBump->yChanges[blockBump->yChangeIndex];

      blockBump->yChangeIndex++;

      if (blockBump->yChangeIndex >= blockBump->yChanges.size()) {
         entity->remove<BlockBumpComponent>();
      }
   });

   // Main Physics update loop
   world->find<MovingComponent, PositionComponent>([&](Entity* entity) {
      if (entity->hasComponent<FrozenComponent>()) {
         return;
      }

      if (!Camera::Get().inCameraRange(entity->getComponent<PositionComponent>()) &&
          !entity->hasAny<MoveOutsideCameraComponent, PlayerComponent>()) {
         if (entity->hasComponent<DestroyOutsideCameraComponent>()) {
            world->destroy(entity);
         }
         return;
      }

      auto* move = entity->getComponent<MovingComponent>();
      auto* position = entity->getComponent<PositionComponent>();

      position->position.x += move->velocity.x;
      position->position.y += move->velocity.y;

      move->velocity.x += move->acceleration.x;
      move->velocity.y += move->acceleration.y;

      if (!entity->hasAny<EnemyComponent, CollectibleComponent>() &&
          !entity->hasComponent<FrictionExemptComponent>()) {
         move->velocity.x *= FRICTION;
      }

      if (move->velocity.x > MAX_SPEED_X) {
         move->velocity.x = MAX_SPEED_X;
      }
      if (move->velocity.y > MAX_SPEED_Y) {
         move->velocity.y = MAX_SPEED_Y;
      }

      if (move->velocity.x < -MAX_SPEED_X) {
         move->velocity.x = -MAX_SPEED_X;
      }

      // Entity + Tile Collisions
      world->find<TileComponent, ForegroundComponent>([&](Entity* other) {
         // We don't check collisions of particles
         if (entity == other || other->hasComponent<ParticleComponent>() ||
             entity->hasComponent<ParticleComponent>()) {
            return;
         }

         CollisionDirection collidedDirectionVertical;
         CollisionDirection collidedDirectionHorizontal;

         if (entity->hasComponent<CollisionExemptComponent>() ||
             other->hasComponent<InvisibleBlockComponent>()) {
            collidedDirectionVertical = checkCollisionY(other, position, move, false);
            collidedDirectionHorizontal = checkCollisionX(other, position, move, false);
         } else {
            collidedDirectionVertical = checkCollisionY(other, position, move, true);
            collidedDirectionHorizontal = checkCollisionX(other, position, move, true);

            if (collidedDirectionVertical != CollisionDirection::NONE) {
               move->velocity.y = move->acceleration.y = 0.0f;
            }
            if (collidedDirectionHorizontal != CollisionDirection::NONE) {
               move->velocity.x = move->acceleration.x = 0.0f;
            }
         }

         switch (collidedDirectionVertical) {
            case CollisionDirection::TOP:
               entity->addComponent<TopCollisionComponent>();
               break;
            case CollisionDirection::BOTTOM:
               entity->addComponent<BottomCollisionComponent>();
               break;
            default:
               break;
         }

         switch (collidedDirectionHorizontal) {
            case CollisionDirection::LEFT:
               entity->addComponent<LeftCollisionComponent>();
               break;
            case CollisionDirection::RIGHT:
               entity->addComponent<RightCollisionComponent>();
               break;
            default:
               break;
         }
      });

      if (std::abs(move->velocity.y) < MARIO_ACCELERATION_X / 2 && move->acceleration.y == 0.0) {
         move->velocity.y = 0;
      }
      if (std::abs(move->velocity.x) < MARIO_ACCELERATION_X / 2 && move->acceleration.x == 0.0) {
         move->velocity.x = 0;
      }
   });

   // Update the spinning of the fire bars
   updateFireBars(world);

   // Update the velocities for the moving platforms
   updateMovingPlatforms(world);

   // Update the velocities for the platform levels
   updatePlatformLevels(world);
}
