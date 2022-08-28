#pragma once

#include "Camera.h"
#include "ECS/ECS.h"
#include "Math.h"
#include "command/Command.h"
#include "command/RunCommand.h"
#include "command/SequenceCommand.h"
#include "command/WaitCommand.h"
#include "command/WaitUntilCommand.h"
#include "scenes/GameScene.h"
#include "systems/PlayerSystem.h"
#include "systems/WarpSystem.h"

#include <vector>

class WarpCommand : public SequenceCommand {
  public:
   WarpCommand(GameScene* scene, World* world, Entity* pipe, Entity* player) {
      if (player->hasAny<ParticleComponent, DeadComponent>()) {
         addCommands(std::vector<Command*>{new RunCommand([]() {})});
         return;
      }

      auto* warpPipe = pipe->getComponent<WarpPipeComponent>();

      auto* playerPosition = player->getComponent<PositionComponent>();
      auto* playerMove = player->getComponent<MovingComponent>();

      Vector2f pipeLocation = pipe->getComponent<PositionComponent>()->position;
      Vector2i teleportLocation = warpPipe->playerLocation;

      Camera::Get().setCameraLeft(warpPipe->cameraLocation.x);

      WarpSystem::setWarping(true);
      PlayerSystem::enableInput(false);

      scene->stopMusic();

      Entity* pipeSound(world->create());
      pipeSound->addComponent<SoundComponent>(SoundID::PIPE);

      player->addComponent<CollisionExemptComponent>();
      player->addComponent<FrictionExemptComponent>();
      player->remove<GravityComponent>();

      addCommands(std::vector<Command*>{
          new RunCommand([=]() {
             // Set the player's speed to go in the pipe
             switch (warpPipe->inDirection) {
                case Direction::UP:
                   playerMove->velocity.y = -1.0;
                   playerMove->acceleration.y = 0.0;
                   playerMove->velocity.x = playerMove->acceleration.x = 0.0;
                   break;
                case Direction::DOWN:
                   playerMove->velocity.y = 1.0;
                   playerMove->acceleration.y = 0.0;
                   playerMove->velocity.x = playerMove->acceleration.x = 0.0;
                   break;
                case Direction::LEFT:
                   playerMove->velocity.x = -1.0;
                   playerMove->acceleration.x = 0.0;
                   playerMove->velocity.y = playerMove->acceleration.y = 0.0;
                   break;
                case Direction::RIGHT:
                   playerMove->velocity.x = 1.0;
                   playerMove->acceleration.x = 0.0;
                   playerMove->velocity.y = playerMove->acceleration.y = 0.0;
                   break;
                default:
                   break;
             }
          }),
          /* Enter the pipe */
          new WaitUntilCommand([=]() -> bool {
             switch (warpPipe->inDirection) {
                case Direction::UP:
                   return player->getComponent<PositionComponent>()->getBottom() <
                          pipeLocation.y - 32;
                   break;
                case Direction::DOWN:
                   return player->getComponent<PositionComponent>()->getTop() > pipeLocation.y + 32;
                   break;
                case Direction::LEFT:
                   return player->getComponent<PositionComponent>()->getRight() <
                          pipeLocation.x - 32;
                   break;
                case Direction::RIGHT:
                   return player->getComponent<PositionComponent>()->getLeft() >
                          pipeLocation.x + 32;
                   break;
                default:
                   return false;
                   break;
             }
          }),
          /* Teleport or go to new level */
          new RunCommand([=]() {
             if (warpPipe->newLevel != Vector2i(0, 0)) {
                Camera::Get().setCameraFrozen(false);

                player->remove<CollisionExemptComponent, FrictionExemptComponent>();
                player->addComponent<GravityComponent>();

                TextureManager::Get().SetBackgroundColor(BackgroundColor::BLACK);

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
                   piranha->getComponent<MovingComponent>()->velocity.y = 0;

                   piranha->getComponent<TimerComponent>()->reset();
                   piranha->remove<WaitUntilComponent>();
                }
             });

             scene->setUnderwater(warpPipe->levelType == LevelType::UNDERWATER);
             scene->setLevelMusic(warpPipe->levelType);

             Camera::Get().setCameraX(warpPipe->cameraLocation.x * SCALED_CUBE_SIZE);
             Camera::Get().setCameraY(warpPipe->cameraLocation.y * SCALED_CUBE_SIZE);
             Camera::Get().updateCameraMin();

             Camera::Get().setCameraFrozen(warpPipe->cameraFreeze);

             TextureManager::Get().SetBackgroundColor(warpPipe->backgroundColor);

             switch (warpPipe->outDirection) {
                case Direction::UP:
                   playerPosition->position = toVector2f(teleportLocation) * SCALED_CUBE_SIZE;
                   playerPosition->position.y += 32;

                   playerMove->velocity.y = -1.0;
                   playerMove->velocity.x = playerMove->acceleration.x = 0.0;
                   break;
                case Direction::DOWN:
                   playerPosition->position = toVector2f(teleportLocation) * SCALED_CUBE_SIZE;
                   playerPosition->position.y -= 32;

                   playerMove->velocity.y = 1.0;
                   playerMove->velocity.x = playerMove->acceleration.x = 0.0;
                   break;
                case Direction::LEFT:
                   playerPosition->position = toVector2f(teleportLocation) * SCALED_CUBE_SIZE;
                   playerPosition->position.x += 32;

                   playerMove->velocity.x = -1.0;
                   playerMove->velocity.y = playerMove->acceleration.y = 0.0;
                   break;
                case Direction::RIGHT:
                   playerPosition->position = toVector2f(teleportLocation) * SCALED_CUBE_SIZE;
                   playerPosition->position.x -= 32;

                   playerMove->velocity.x = 1.0;
                   playerMove->velocity.y = playerMove->acceleration.y = 0.0;
                   break;
                case Direction::NONE:
                   playerPosition->position = toVector2f(teleportLocation) * SCALED_CUBE_SIZE;
                   playerMove->velocity.x = playerMove->acceleration.x = playerMove->velocity.y =
                       playerMove->acceleration.y = 0.0f;
                   break;
                default:
                   break;
             }
          })});
      // Extra commands to add on if the pipe doesn't lead to a new level
      if (warpPipe->newLevel == Vector2i(0, 0)) {
         addCommands(std::vector<Command*>{
             new WaitUntilCommand([=]() -> bool {
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
             }),
             new RunCommand([=]() {
                WarpSystem::setWarping(false);
                PlayerSystem::enableInput(true);
                PlayerSystem::setGameStart(false);

                playerMove->velocity.x = playerMove->acceleration.x = playerMove->velocity.y =
                    playerMove->acceleration.y = 0.0f;

                player->addComponent<GravityComponent>();
                player->remove<CollisionExemptComponent>();
                player->remove<FrictionExemptComponent>();
             })});
      }
   }

   void execute() override {
      SequenceCommand::execute();
   }

   bool isFinished() override {
      return SequenceCommand::isFinished();
   }

  private:
   Vector2<float> toVector2f(Vector2i vector) {
      return Vector2f((float)vector.x, (float)vector.y);
   }
};
