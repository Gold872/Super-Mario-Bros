#pragma once

#include "Camera.h"
#include "ECS/ECS.h"
#include "SMBMath.h"
#include "command/Command.h"
#include "command/RunCommand.h"
#include "command/SequenceCommand.h"
#include "command/WaitUntilCommand.h"
#include "systems/PlayerSystem.h"
#include "systems/WarpSystem.h"

#include <iostream>
#include <vector>

class VineCommand : public SequenceCommand {
  public:
   VineCommand(GameScene* scene, WarpSystem* warpSystem, World* world, Entity* vine,
               Entity* player) {
      auto* vineComponent = vine->getComponent<VineComponent>();

      auto* playerMove = player->getComponent<MovingComponent>();
      auto* playerPosition = player->getComponent<PositionComponent>();

      playerMove->velocity.y = -1.5;

      warpSystem->setTeleportLevelY(vineComponent->resetYValue * SCALED_CUBE_SIZE);

      warpSystem->setTeleportPlayerCoordinates(vineComponent->resetTeleportLocation *
                                               SCALED_CUBE_SIZE);

      warpSystem->setTeleportCameraCoordinates(vineComponent->resetTeleportLocation *
                                                   SCALED_CUBE_SIZE -
                                               Vector2i(2, 1) * SCALED_CUBE_SIZE);

      std::vector<Entity*>& vineParts = vineComponent->vineParts;

      addCommands(std::vector<Command*>{
          /* When the player is out of camera range, change the camera location */
          new WaitUntilCommand([=]() -> bool {
             return !Camera::Get().inCameraRange(playerPosition);
          }),
          new RunCommand([=, &vineParts]() {
             warpSystem->setTeleportCameraMax(Camera::Get().getCameraMaxX());
             Camera::Get().setCameraMaxX(vineComponent->newCameraMax * SCALED_CUBE_SIZE);

             Camera::Get().setCameraX(vineComponent->cameraCoordinates.x * SCALED_CUBE_SIZE);
             Camera::Get().setCameraY(vineComponent->cameraCoordinates.y * SCALED_CUBE_SIZE);

             warpSystem->setTeleportBackgroundColor(TextureManager::Get().getBackgroundColor());
             TextureManager::Get().SetBackgroundColor(vineComponent->newBackgroundColor);

             warpSystem->setTeleportLevelType(scene->getCurrentLevelType());
             scene->setCurrentLevelType(vineComponent->newLevelType);
             scene->setLevelMusic(vineComponent->newLevelType);

             // Moves the vines upwards
             for (Entity* vinePiece : vineParts) {
                vinePiece->getComponent<MovingComponent>()->velocity.y = -1.0;
             }

             // Sets mario's position to be at the bottom of the vine
             playerPosition->setTop(vineParts.back()->getComponent<PositionComponent>()->getTop());

             playerPosition->setLeft(
                 vineParts.back()->getComponent<PositionComponent>()->getRight() -
                 SCALED_CUBE_SIZE / 2);
          }),
          /* Wait until the vine has fully moved up, and then stop the vines from growing more */
          new WaitUntilCommand([=, &vineParts]() -> bool {
             return vineParts.front()->getComponent<PositionComponent>()->getTop() <=
                    (vineComponent->teleportCoordinates.y * SCALED_CUBE_SIZE) -
                        (SCALED_CUBE_SIZE * 4);
          }),
          new RunCommand([=, &vineParts]() {
             for (Entity* vinePiece : vineParts) {
                vinePiece->getComponent<MovingComponent>()->velocity.y = 0.0;
                vinePiece->remove<VineComponent>();
             }
          }),
          /* Wait until the player has climbed to the top of the vine, then end the sequence */
          new WaitUntilCommand([=, &vineParts]() -> bool {
             return playerPosition->getBottom() <=
                    vineParts[1]->getComponent<PositionComponent>()->getBottom();
          }),
          new RunCommand([=, &vineParts]() {
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
          })});
   }

   void execute() override {
      SequenceCommand::execute();
   }

   bool isFinished() override {
      return SequenceCommand::isFinished();
   }
};
