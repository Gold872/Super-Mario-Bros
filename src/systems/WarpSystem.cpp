#include "systems/WarpSystem.h"

#include "AABBCollision.h"
#include "Camera.h"
#include "Constants.h"
#include "ECS/Components.h"
#include "ECS/ECS.h"
#include "Math.h"
#include "TextureManager.h"
#include "command/CommandScheduler.h"
#include "command/Commands.h"

#include <iostream>
#include <vector>

bool WarpSystem::warping = false;
bool WarpSystem::climbing = false;
bool WarpSystem::climbed = false;

Vector2i toVector2i(Vector2f vector) {
   return Vector2i((int)vector.x, (int)vector.y);
}

Vector2f toVector2f(Vector2i vector) {
   return Vector2f((float)vector.x, (float)vector.y);
}

void WarpSystem::warp(World* world, Entity* pipe, Entity* player) {
   if (WarpSystem::isWarping()) {
      return;
   }

   CommandScheduler::getInstance().addCommand(new WarpCommand(scene, world, pipe, player));
}

void WarpSystem::climb(World* world, Entity* vine, Entity* player) {
   CommandScheduler::getInstance().addCommand(new VineCommand(scene, this, world, vine, player));
}

WarpSystem::WarpSystem(GameScene* scene) {
   this->scene = scene;
}

void WarpSystem::onAddedToWorld(World* world) {
   WarpSystem::setWarping(false);
}

void WarpSystem::tick(World* world) {
   // Warp pipe checking
   world->find<WarpPipeComponent, PositionComponent>([&](Entity* entity) {
      auto* warpPipe = entity->getComponent<WarpPipeComponent>();

      Entity* player = world->findFirst<PlayerComponent>();

      // If colliding with pipe
      if (!AABBCollision(entity->getComponent<PositionComponent>(),
                         player->getComponent<PositionComponent>()) ||
          WarpSystem::isWarping()) {
         return;
      }

      auto* playerMove = player->getComponent<MovingComponent>();

      switch (warpPipe->inDirection) {
         case Direction::UP:
            if (up || playerMove->velocity.y < 0.0) {
               warp(world, entity, player);
            }
            break;
         case Direction::DOWN:
            if (down || playerMove->velocity.y > 0.0) {
               warp(world, entity, player);
            }
            break;
         case Direction::LEFT:
            if (left || playerMove->velocity.x < 0.0) {
               warp(world, entity, player);
            }
            break;
         case Direction::RIGHT:
            if (right || playerMove->velocity.x > 0.0) {
               warp(world, entity, player);
            }
            break;
         default:
            break;
      }
   });
   // Vine checking
   world->find<VineComponent, PositionComponent>([&](Entity* entity) {
      Entity* player = world->findFirst<PlayerComponent>();

      auto* playerPosition = player->getComponent<PositionComponent>();

      auto* playerMove = player->getComponent<MovingComponent>();

      if (!AABBTotalCollision(playerPosition, entity->getComponent<PositionComponent>()) ||
          (WarpSystem::isClimbing() && playerMove->velocity.y != 0)) {
         return;
      }

      player->addComponent<CollisionExemptComponent>();
      player->addComponent<FrictionExemptComponent>();
      player->remove<GravityComponent>();

      player->getComponent<TextureComponent>()->setHorizontalFlipped(true);
      playerPosition->setLeft(entity->getComponent<PositionComponent>()->getRight() -
                              SCALED_CUBE_SIZE / 2);

      WarpSystem::setClimbing(true);
      PlayerSystem::enableInput(false);

      playerMove->velocity.x = playerMove->acceleration.x = playerMove->acceleration.y =
          playerMove->velocity.y = 0;

      if (up) {
         climb(world, entity, player);
      }
   });

   // If the player is below the Y level where it gets teleported
   if (climbed) {
      Entity* player = world->findFirst<PlayerComponent, PositionComponent, MovingComponent>();
      auto* playerPosition = player->getComponent<PositionComponent>();
      auto* playerMove = player->getComponent<MovingComponent>();

      if (playerPosition->getTop() > teleportLevelY + SCALED_CUBE_SIZE * 3) {
         player->addComponent<FrozenComponent>();

         WarpSystem::setClimbed(false);

         CommandScheduler::getInstance().addCommand(new DelayedCommand(
             [=]() {
                player->remove<FrozenComponent>();

                playerMove->velocity.x = playerMove->acceleration.x = playerMove->velocity.y =
                    playerMove->acceleration.y = 0;

                Camera::Get().setCameraX(teleportCameraCoordinates.x);
                Camera::Get().setCameraY(teleportCameraCoordinates.y);
                Camera::Get().setCameraMaxX(teleportCameraMax);

                TextureManager::Get().SetBackgroundColor(teleportBackgroundColor);

                scene->setCurrentLevelType(teleportLevelType);
                scene->setLevelMusic(teleportLevelType);

                playerPosition->position = teleportPlayerCoordinates.convertTo<float>();

                teleportPlayerCoordinates = Vector2i(0, 0);
                teleportLevelY = 0;
                teleportCameraMax = 0;
                teleportBackgroundColor = BackgroundColor::NONE;
                teleportLevelType = LevelType::NONE;
             },
             2.0));
      }
   }
}

void WarpSystem::handleInput(SDL_Event& event) {
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
