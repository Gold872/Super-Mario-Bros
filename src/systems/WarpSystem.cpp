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
bool WarpSystem::climbing = false;
bool WarpSystem::climbed = false;

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

   Camera::Get().setCameraMin(warpPipe->cameraLocation.x);

   WarpSystem::setWarping(true);
   PlayerSystem::enableInput(false);

   scene->stopMusic();

   Entity* pipeSound(world->create());
   pipeSound->addComponent<SoundComponent>(SoundID::PIPE);

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
             Camera::Get().setCameraFrozen(false);
             Camera::Get().setCameraX(0);
             Camera::Get().setCameraY(0);

             wait->condition = 0;
             wait->doAfter = 0;

             player->remove<CollisionExemptComponent, FrictionExemptComponent>();
             player->addComponent<GravityComponent>();

             TextureManager::Get().SetBackgroundColor(warpPipe->backgroundColor);

             WarpSystem::setWarping(false);

             player->remove<WaitUntilComponent>();

             scene->switchLevel(warpPipe->newLevel.x, warpPipe->newLevel.y);
             return;
          }

          // Puts the piranha plants back in the pipe
          world->find<PiranhaPlantComponent>([&](Entity* piranha) {
             auto* piranhaComponent = piranha->getComponent<PiranhaPlantComponent>();

             if (!piranhaComponent->inPipe) {
                piranha->getComponent<PositionComponent>()->position.y =
                    piranhaComponent->pipeCoordinates.y;
                piranha->getComponent<PiranhaPlantComponent>()->inPipe = true;
                piranha->getComponent<MovingComponent>()->velocityY = 0;

                piranha->remove<WaitUntilComponent>();
             }
          });

          scene->setUnderwater(warpPipe->levelType == LevelType::UNDERWATER);
          scene->setLevelMusic(warpPipe->levelType);

          Camera::Get().setCameraX(warpPipe->cameraLocation.x * SCALED_CUBE_SIZE);
          Camera::Get().setCameraY(warpPipe->cameraLocation.y * SCALED_CUBE_SIZE);

          Camera::Get().setCameraFrozen(warpPipe->cameraFreeze);

          TextureManager::Get().SetBackgroundColor(warpPipe->backgroundColor);

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

void WarpSystem::climb(World* world, Entity* vine, Entity* player) {
   auto* vineComponent = vine->getComponent<VineComponent>();

   auto* playerMove = player->getComponent<MovingComponent>();
   auto* playerPosition = player->getComponent<PositionComponent>();

   playerMove->velocityY = -1.5;

   teleportLevelY = vineComponent->resetYValue * SCALED_CUBE_SIZE;

   teleportPlayerCoordinates = vineComponent->resetTeleportLocation * SCALED_CUBE_SIZE;

   teleportCameraCoordinates =
       vineComponent->resetTeleportLocation * SCALED_CUBE_SIZE - Vector2i(2, 1) * SCALED_CUBE_SIZE;

   std::vector<Entity*>& vineParts = vineComponent->vineParts;

   // Wait until the player is out of camera range, then change the camera location
   player->addComponent<WaitUntilComponent>(
       [=](Entity* entity) {
          return !Camera::Get().inCameraRange(playerPosition);
       },
       [=, &vineParts](Entity* entity) {
          auto* waitUntil = player->getComponent<WaitUntilComponent>();

          Camera::Get().setCameraX(vineComponent->cameraCoordinates.x * SCALED_CUBE_SIZE);
          Camera::Get().setCameraY(vineComponent->cameraCoordinates.y * SCALED_CUBE_SIZE);

          // Moves the vines upwards
          for (Entity* vinePiece : vineParts) {
             vinePiece->getComponent<MovingComponent>()->velocityY = -1.0;
          }

          // Sets mario's position to be at the bottom of the vine
          playerPosition->setTop(vineParts.back()->getComponent<PositionComponent>()->getTop());

          playerPosition->setLeft(vineParts.back()->getComponent<PositionComponent>()->getRight() -
                                  SCALED_CUBE_SIZE / 2);

          // Wait until the vine has fully moved up, and then stop the vines from growing more
          waitUntil->condition = [=](Entity* entity) {
             return vineParts.front()->getComponent<PositionComponent>()->getTop() <=
                    (vineComponent->teleportCoordinates.y * SCALED_CUBE_SIZE) -
                        (SCALED_CUBE_SIZE * 4);
          };

          waitUntil->doAfter = [=, &vineParts](Entity* entity) {
             for (Entity* vinePiece : vineParts) {
                vinePiece->getComponent<MovingComponent>()->velocityY = 0.0;
                vinePiece->remove<VineComponent>();
             }

             // Wait until the player has climbed to the top of the vine, then end the sequence
             waitUntil->condition = [=](Entity* entity) {
                return playerPosition->getBottom() <=
                       vineParts[1]->getComponent<PositionComponent>()->getBottom();
             };

             waitUntil->doAfter = [=](Entity* entity) {
                // Moves the player away from the vine
                playerPosition->setLeft(
                    vineParts.front()->getComponent<PositionComponent>()->getRight());

                player->getComponent<TextureComponent>()->setHorizontalFlipped(false);
                player->addComponent<GravityComponent>();
                player->remove<FrictionExemptComponent>();
                player->remove<CollisionExemptComponent>();

                PlayerSystem::enableInput(true);
                WarpSystem::setClimbing(false);

                WarpSystem::setClimbed(true);

                player->remove<WaitUntilComponent>();
             };
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
   // Vine checking
   world->find<VineComponent, PositionComponent>([&](Entity* entity) {
      Entity* player = world->findFirst<PlayerComponent>();

      auto* playerPosition = player->getComponent<PositionComponent>();

      auto* playerMove = player->getComponent<MovingComponent>();

      if (!AABBTotalCollision(playerPosition, entity->getComponent<PositionComponent>()) ||
          (WarpSystem::isClimbing() && playerMove->velocityY != 0)) {
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

      playerMove->velocityX = playerMove->accelerationX = playerMove->accelerationY =
          playerMove->velocityY = 0;

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

         Entity* tempCallback(world->create());
         tempCallback->addComponent<CallbackComponent>(
             [=](Entity* entity) {
                player->remove<FrozenComponent>();

                playerMove->velocityX = playerMove->accelerationX = playerMove->velocityY =
                    playerMove->accelerationY = 0;

                Camera::Get().setCameraX(teleportCameraCoordinates.x);
                Camera::Get().setCameraY(teleportCameraCoordinates.y);

                playerPosition->position = teleportPlayerCoordinates.convertTo<float>();

                teleportPlayerCoordinates = Vector2i(0, 0);
                teleportLevelY = 0;

                world->destroy(entity);
             },
             MAX_FPS * 2);
      }
   }
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
