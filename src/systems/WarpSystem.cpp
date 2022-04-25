#include "systems/WarpSystem.h"

#include "AABBCollision.h"
#include "Camera.h"
#include "Constants.h"
#include "ECS/Components.h"
#include "ECS/ECS.h"
#include "Math.h"
#include "TextureManager.h"

#include <iostream>

bool WarpSystem::warping = false;

Vector2i toVector2i(Vector2f vector) {
   return Vector2i((int)vector.x, (int)vector.y);
}

Vector2f toVector2f(Vector2i vector) {
   return Vector2f((float)vector.x, (float)vector.y);
}

void WarpSystem::warp(World* world, Entity* pipe, Entity* player) {
   auto* warpPipe = pipe->getComponent<WarpPipeComponent>();

   auto* playerMove = player->getComponent<MovingComponent>();

   Vector2f pipeLocation = pipe->getComponent<PositionComponent>()->position;
   Vector2i teleportLocation = warpPipe->playerLocation;

   Camera::setCameraMin(warpPipe->cameraLocation.x);

   WarpSystem::setWarping(true);
   PlayerSystem::enableInput(false);

   player->addComponent<CollisionExemptComponent>();
   player->addComponent<FrictionExemptComponent>();
   player->remove<GravityComponent>();

   // Set the player's speed to go in the pipe
   switch (warpPipe->inDirection) {
      case Direction::UP:
         playerMove->velocityY = -1.0;
         playerMove->accelerationY = 0.0;
         playerMove->velocityX = playerMove->accelerationX = 0.0;
         break;
      case Direction::DOWN:
         playerMove->velocityY = 1.0;
         playerMove->accelerationY = 0.0;
         playerMove->velocityX = playerMove->accelerationX = 0.0;
         break;
      case Direction::LEFT:
         playerMove->velocityX = -1.0;
         playerMove->accelerationX = 0.0;
         playerMove->velocityY = playerMove->accelerationY = 0.0;
         break;
      case Direction::RIGHT:
         playerMove->velocityX = 1.0;
         playerMove->accelerationX = 0.0;
         playerMove->velocityY = playerMove->accelerationY = 0.0;
         break;
      default:
         break;
   }

   // Enter the pipe
   player->addComponent<WaitUntilComponent>(
       [=](Entity* player) {
          switch (warpPipe->inDirection) {
             case Direction::UP:
                return player->getComponent<PositionComponent>()->getBottom() < pipeLocation.y - 32;
                break;
             case Direction::DOWN:
                return player->getComponent<PositionComponent>()->getTop() > pipeLocation.y + 32;
                break;
             case Direction::LEFT:
                return player->getComponent<PositionComponent>()->getRight() < pipeLocation.x - 32;
                break;
             case Direction::RIGHT:
                return player->getComponent<PositionComponent>()->getLeft() > pipeLocation.x + 32;
                break;
             default:
                return false;
                break;
          }
       },
       [=](Entity* player) {
          // Teleport or go to new level
          auto* wait = player->getComponent<WaitUntilComponent>();
          auto* playerPosition = player->getComponent<PositionComponent>();

          if (warpPipe->newLevel != Vector2i(0, 0)) {
             Camera::setCameraFrozen(false);
             Camera::setCameraX(0);
             Camera::setCameraY(0);

             wait->condition = 0;
             wait->doAfter = 0;

             player->remove<CollisionExemptComponent, FrictionExemptComponent>();
             player->addComponent<GravityComponent>();

             TextureManager::SetBackgroundColor(warpPipe->backgroundColor);

             WarpSystem::setWarping(false);

             player->remove<WaitUntilComponent>();

             scene->switchLevel(warpPipe->newLevel.x, warpPipe->newLevel.y);
             return;
          }

          scene->setUnderwater(warpPipe->underwater);

          Camera::setCameraX(warpPipe->cameraLocation.x * SCALED_CUBE_SIZE);
          Camera::setCameraY(warpPipe->cameraLocation.y * SCALED_CUBE_SIZE);

          Camera::setCameraFrozen(warpPipe->cameraFreeze);

          TextureManager::SetBackgroundColor(warpPipe->backgroundColor);

          switch (warpPipe->outDirection) {
             case Direction::UP:
                playerPosition->position = toVector2f(teleportLocation) * SCALED_CUBE_SIZE;
                playerPosition->position.y += 32;

                playerMove->velocityY = -1.0;
                playerMove->velocityX = playerMove->accelerationX = 0.0;
                break;
             case Direction::DOWN:
                playerPosition->position = toVector2f(teleportLocation) * SCALED_CUBE_SIZE;
                playerPosition->position.y -= 32;

                playerMove->velocityY = 1.0;
                playerMove->velocityX = playerMove->accelerationX = 0.0;
                break;
             case Direction::LEFT:
                playerPosition->position = toVector2f(teleportLocation) * SCALED_CUBE_SIZE;
                playerPosition->position.x += 32;

                playerMove->velocityX = -1.0;
                playerMove->velocityY = playerMove->accelerationY = 0.0;
                break;
             case Direction::RIGHT:
                playerPosition->position = toVector2f(teleportLocation) * SCALED_CUBE_SIZE;
                playerPosition->position.x -= 32;

                playerMove->velocityX = 1.0;
                playerMove->velocityY = playerMove->accelerationY = 0.0;
                break;
             case Direction::NONE:
                playerPosition->position = toVector2f(teleportLocation) * SCALED_CUBE_SIZE;
                playerMove->velocityX = playerMove->accelerationX = playerMove->velocityY =
                    playerMove->accelerationY = 0.0f;
                break;
             default:
                break;
          }

          // Back to normal
          wait->condition = [=](Entity* player) {
             switch (warpPipe->outDirection) {
                case Direction::UP:
                   return playerPosition->getBottom() < teleportLocation.y * SCALED_CUBE_SIZE;
                   break;
                case Direction::DOWN:
                   return playerPosition->getTop() > teleportLocation.y * SCALED_CUBE_SIZE;
                   break;
                case Direction::LEFT:
                   return playerPosition->getRight() < teleportLocation.x * SCALED_CUBE_SIZE;
                   break;
                case Direction::RIGHT:
                   return playerPosition->getLeft() > teleportLocation.x * SCALED_CUBE_SIZE;
                   break;
                default:
                   return true;
                   break;
             }
          };
          wait->doAfter = [&](Entity* player) {
             WarpSystem::setWarping(false);
             PlayerSystem::enableInput(true);
             PlayerSystem::setGameStart(false);

             playerMove->velocityX = playerMove->accelerationX = playerMove->velocityY =
                 playerMove->accelerationY = 0.0f;

             player->addComponent<GravityComponent>();
             player->remove<CollisionExemptComponent>();
             player->remove<FrictionExemptComponent>();
             player->remove<WaitUntilComponent>();
          };
       });
}

WarpSystem::WarpSystem(GameScene* scene) {
   this->scene = scene;
}

void WarpSystem::onAddedToWorld(World* world) {
   WarpSystem::setWarping(false);
}

void WarpSystem::tick(World* world) {
   world->find<WarpPipeComponent>([&](Entity* entity) {
      auto* warpPipe = entity->getComponent<WarpPipeComponent>();

      Entity* player = world->findFirst<PlayerComponent>();

      // If colliding with pipe
      if (!AABBCollision(entity->getComponent<PositionComponent>(),
                         player->getComponent<PositionComponent>())) {
         return;
      }

      auto* playerMove = player->getComponent<MovingComponent>();

      switch (warpPipe->inDirection) {
         case Direction::UP:
            if (up || playerMove->velocityY < 0.0) {
               warp(world, entity, player);
            }
            break;
         case Direction::DOWN:
            if (down || playerMove->velocityY > 0.0) {
               warp(world, entity, player);
            }
            break;
         case Direction::LEFT:
            if (left || playerMove->velocityX < 0.0) {
               warp(world, entity, player);
            }
            break;
         case Direction::RIGHT:
            if (right || playerMove->velocityX > 0.0) {
               warp(world, entity, player);
            }
            break;
         default:
            break;
      }
   });
}

void WarpSystem::handleEvent(SDL_Event& event) {
   if (event.type != SDL_KEYDOWN && event.type != SDL_KEYUP) {
      return;
   }
   switch (event.type) {
      case SDL_KEYDOWN:
         switch (event.key.keysym.scancode) {
            case SDL_SCANCODE_W:
            case SDL_SCANCODE_UP:
               up = true;
               break;
            case SDL_SCANCODE_S:
            case SDL_SCANCODE_DOWN:
               down = true;
               break;
            case SDL_SCANCODE_A:
            case SDL_SCANCODE_LEFT:
               left = true;
               break;
            case SDL_SCANCODE_D:
            case SDL_SCANCODE_RIGHT:
               right = true;
               break;
            default:
               break;
         }
         break;
      case SDL_KEYUP:
         switch (event.key.keysym.scancode) {
            case SDL_SCANCODE_W:
            case SDL_SCANCODE_UP:
               up = false;
               break;
            case SDL_SCANCODE_S:
            case SDL_SCANCODE_DOWN:
               down = false;
               break;
            case SDL_SCANCODE_A:
            case SDL_SCANCODE_LEFT:
               left = false;
               break;
            case SDL_SCANCODE_D:
            case SDL_SCANCODE_RIGHT:
               right = false;
               break;
            default:
               break;
         }
         break;
      default:
         break;
   }
}
