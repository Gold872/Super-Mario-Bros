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

   if (move->velocityY >= 0.0f) {
      // If falling
      if (AABBTotalCollision(position->position.x + position->hitbox.x + (TILE_ROUNDNESS / 2),
                             position->position.y + position->hitbox.y + move->velocityY,
                             position->hitbox.w - TILE_ROUNDNESS, position->hitbox.h,
                             solidPosition->position.x + (TILE_ROUNDNESS / 2),
                             solidPosition->position.y, solidPosition->hitbox.w - TILE_ROUNDNESS,
                             solidPosition->hitbox.h)) {
         float topDistance =
             std::abs(solidPosition->getTop() - (position->getBottom() + move->velocityY));
         float bottomDistance =
             std::abs((position->getTop() + move->velocityY) - solidPosition->getBottom());

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
                             position->position.y + position->hitbox.y + move->velocityY,
                             position->hitbox.w - (TILE_ROUNDNESS * 2), position->hitbox.h,
                             solidPosition->position.x + TILE_ROUNDNESS, solidPosition->position.y,
                             solidPosition->hitbox.w - (TILE_ROUNDNESS * 2),
                             solidPosition->hitbox.h)) {
         float topDistance =
             std::abs(solidPosition->getTop() - (position->getBottom() + move->velocityY));
         float bottomDistance =
             std::abs((position->getTop() + move->velocityY) - solidPosition->getBottom());
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

   if (AABBTotalCollision(position->position.x + position->hitbox.x + move->velocityX,
                          position->position.y + position->hitbox.y, position->hitbox.w,
                          position->hitbox.h - (TILE_ROUNDNESS * 2), solidPosition)) {
      float leftDistance = std::abs((position->position.x + position->hitbox.x + move->velocityX) -
                                    solidPosition->getRight());
      float rightDistance = std::abs(
          (position->position.x + position->hitbox.x + position->hitbox.w + move->velocityX) -
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

      switch (fireBar->direction) {
         case RotationDirection::CLOCKWISE:
            fireBar->barAngle += 2.0;
            if (fireBar->barAngle > 360) {
               fireBar->barAngle -= 360;
            }

            position->position.x =
                fireBar->calculateXPosition(fireBar->barAngle) + fireBar->pointOfRotation.x;
            position->position.y =
                fireBar->calculateYPosition(fireBar->barAngle) + fireBar->pointOfRotation.y;
            break;
         case RotationDirection::COUNTER_CLOCKWISE:
            fireBar->barAngle -= 2.0;
            if (fireBar->barAngle < 0) {
               fireBar->barAngle += 360;
            }

            position->position.x =
                fireBar->calculateXPosition(fireBar->barAngle) + fireBar->pointOfRotation.x;
            position->position.y =
                fireBar->calculateYPosition(fireBar->barAngle) + fireBar->pointOfRotation.y;
            break;
         default:
            break;
      }
   });
}

void PhysicsSystem::updateMovingPlatforms(World* world) {
   world->find<MovingPlatformComponent, MovingComponent, PositionComponent>([&](Entity* entity) {
      auto* platform = entity->getComponent<MovingPlatformComponent>();
      auto* platformMove = entity->getComponent<MovingComponent>();
      auto* position = entity->getComponent<PositionComponent>();

      if (platform->connectedParts.empty()) {
         return;
      }

      switch (platform->motionType) {
         case PlatformMotionType::ONE_DIRECTION_REPEATED:
            switch (platform->movingDirection) {
               case Direction::LEFT:
               case Direction::RIGHT:
                  if (position->position.x < platform->minPoint) {
                     position->position.x = platform->maxPoint;

                     for (auto* connectedPart : platform->connectedParts) {
                        connectedPart->getComponent<PositionComponent>()->position.x =
                            platform->maxPoint;
                     }
                  } else if (position->position.x > platform->maxPoint) {
                     position->position.x = platform->minPoint;

                     for (auto* connectedPart : platform->connectedParts) {
                        connectedPart->getComponent<PositionComponent>()->position.x =
                            platform->minPoint;
                     }
                  }
                  // Re-aligns the platform pieces together
                  for (unsigned int i = 0; i < platform->connectedParts.size(); i++) {
                     Entity* connectedPart = platform->connectedParts[i];

                     if (i != 0) {
                        connectedPart->getComponent<PositionComponent>()->setTop(
                            platform->connectedParts[i - 1]
                                ->getComponent<PositionComponent>()
                                ->getTop());
                     } else {
                        connectedPart->getComponent<PositionComponent>()->setTop(
                            position->getTop());
                     }
                  }
                  break;
               case Direction::UP:
               case Direction::DOWN:
                  if (position->getTop() > platform->minPoint) {
                     position->setBottom(platform->maxPoint);

                     for (auto* connectedPart : platform->connectedParts) {
                        connectedPart->getComponent<PositionComponent>()->position.y =
                            platform->maxPoint;
                     }
                  } else if (position->getBottom() < platform->maxPoint) {
                     position->setTop(platform->minPoint);

                     for (auto* connectedPart : platform->connectedParts) {
                        connectedPart->getComponent<PositionComponent>()->position.y =
                            platform->minPoint;
                     }
                  }
                  // Re-aligns the platform pieces together
                  for (unsigned int i = 0; i < platform->connectedParts.size(); i++) {
                     Entity* connectedPart = platform->connectedParts[i];

                     if (i != 0) {
                        connectedPart->getComponent<PositionComponent>()->setTop(
                            platform->connectedParts[i - 1]
                                ->getComponent<PositionComponent>()
                                ->getTop());
                     } else {
                        connectedPart->getComponent<PositionComponent>()->setTop(
                            position->getTop());
                     }
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

                     platformMove->velocityX = newVelocity;

                     for (unsigned int i = 0; i < platform->connectedParts.size(); i++) {
                        Entity* connectedPart = platform->connectedParts[i];

                        connectedPart->getComponent<MovingComponent>()->velocityX = newVelocity;
                        if (i != 0) {
                           connectedPart->getComponent<PositionComponent>()->setLeft(
                               platform->connectedParts[i - 1]
                                   ->getComponent<PositionComponent>()
                                   ->getRight());
                        } else {
                           connectedPart->getComponent<PositionComponent>()->setLeft(
                               position->getRight());
                        }
                     }
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

                     platformMove->velocityX = newVelocity;

                     for (unsigned int i = 0; i < platform->connectedParts.size(); i++) {
                        Entity* connectedPart = platform->connectedParts[i];

                        connectedPart->getComponent<MovingComponent>()->velocityX = newVelocity;
                        if (i != 0) {
                           connectedPart->getComponent<PositionComponent>()->setLeft(
                               platform->connectedParts[i - 1]
                                   ->getComponent<PositionComponent>()
                                   ->getRight());
                        } else {
                           connectedPart->getComponent<PositionComponent>()->setLeft(
                               position->getRight());
                        }
                     }
                  }
                  break;
               case Direction::UP:
                  if (position->getTop() <= platform->maxPoint) {
                     platform->movingDirection = Direction::DOWN;
                     break;
                  }
                  {
                     float newVelocity = -platform->calculateVelocity(
                         position->getBottom() - platform->maxPoint,
                         (platform->minPoint - platform->maxPoint) / 3.8);

                     platformMove->velocityY = newVelocity;

                     for (unsigned int i = 0; i < platform->connectedParts.size(); i++) {
                        Entity* connectedPart = platform->connectedParts[i];

                        connectedPart->getComponent<MovingComponent>()->velocityY = newVelocity;
                        if (i != 0) {
                           connectedPart->getComponent<PositionComponent>()->setTop(
                               platform->connectedParts[i - 1]
                                   ->getComponent<PositionComponent>()
                                   ->getTop());
                        } else {
                           connectedPart->getComponent<PositionComponent>()->setTop(
                               position->getTop());
                        }
                     }
                  }

                  break;
               case Direction::DOWN:
                  if (position->getBottom() >= platform->minPoint) {
                     platform->movingDirection = Direction::UP;
                     break;
                  }
                  {
                     float newVelocity = platform->calculateVelocity(
                         platform->minPoint - position->getTop(),
                         (platform->minPoint - platform->maxPoint) / 3.8);

                     platformMove->velocityY = newVelocity;

                     for (unsigned int i = 0; i < platform->connectedParts.size(); i++) {
                        Entity* connectedPart = platform->connectedParts[i];

                        connectedPart->getComponent<MovingComponent>()->velocityY = newVelocity;
                        if (i != 0) {
                           connectedPart->getComponent<PositionComponent>()->setTop(
                               platform->connectedParts[i - 1]
                                   ->getComponent<PositionComponent>()
                                   ->getTop());
                        } else {
                           connectedPart->getComponent<PositionComponent>()->setTop(
                               position->getTop());
                        }
                     }
                  }
                  break;
               default:
                  break;
            }
            break;
         default:
            break;
      }
   });
}

void PhysicsSystem::tick(World* world) {
   // Update the spinning of the fire bars
   updateFireBars(world);

   // Update the velocities for the moving platforms
   updateMovingPlatforms(world);

   // Update gravity for entities that have a gravity component
   world->find<GravityComponent, MovingComponent>([&](Entity* entity) {
      if ((!Camera::Get().inCameraRange(entity->getComponent<PositionComponent>()) &&
           !entity->hasAny<MoveOutsideCameraComponent, PlayerComponent>()) ||
          entity->hasComponent<FrozenComponent>()) {
         return;
      }
      entity->getComponent<MovingComponent>()->velocityY += GRAVITY;
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

      position->position.x += move->velocityX;
      position->position.y += move->velocityY;

      move->velocityX += move->accelerationX;
      move->velocityY += move->accelerationY;

      if (!entity->hasAny<EnemyComponent, CollectibleComponent>() &&
          !entity->hasComponent<FrictionExemptComponent>()) {
         move->velocityX *= FRICTION;
      }

      if (move->velocityX > MAX_SPEED_X) {
         move->velocityX = MAX_SPEED_X;
      }
      if (move->velocityY > MAX_SPEED_Y) {
         move->velocityY = MAX_SPEED_Y;
      }

      if (move->velocityX < -MAX_SPEED_X) {
         move->velocityX = -MAX_SPEED_X;
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

         if (entity->hasComponent<CollisionExemptComponent>()) {
            collidedDirectionVertical = checkCollisionY(other, position, move, false);
            collidedDirectionHorizontal = checkCollisionX(other, position, move, false);
         } else {
            collidedDirectionVertical = checkCollisionY(other, position, move, true);
            collidedDirectionHorizontal = checkCollisionX(other, position, move, true);

            if (collidedDirectionVertical != CollisionDirection::NONE) {
               move->velocityY = move->accelerationY = 0.0f;
            }
            if (collidedDirectionHorizontal != CollisionDirection::NONE) {
               move->velocityX = move->accelerationX = 0.0f;
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

      if (std::abs(move->velocityY) < MARIO_ACCELERATION_X / 2) {
         move->velocityY = 0;
      }
      if (std::abs(move->velocityX) < MARIO_ACCELERATION_X / 2) {
         move->velocityX = 0;
      }
   });
}
