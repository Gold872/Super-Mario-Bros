#include "systems/FlagSystem.h"

#include "AABBCollision.h"
#include "Camera.h"
#include "Constants.h"
#include "ECS/Components.h"
#include "systems/PlayerSystem.h"

#include <iostream>

bool FlagSystem::climbing = false;

FlagSystem::FlagSystem(GameScene* scene) {
   this->scene = scene;
}

void FlagSystem::climbFlag(Entity* player, Entity* flag) {
   if (player->hasComponent<WaitUntilComponent>()) {
      return;
   }

   auto* playerMove = player->getComponent<MovingComponent>();
   auto* playerPosition = player->getComponent<PositionComponent>();
   auto* flagMove = flag->getComponent<MovingComponent>();

   PlayerSystem::enableInput(false);

   FlagSystem::setClimbing(true);

   playerPosition->setLeft(flag->getComponent<PositionComponent>()->getLeft());

   playerMove->velocityX = playerMove->accelerationX = playerMove->accelerationY = 0;

   playerMove->velocityY = 3.0;

   flagMove->velocityY = 3.0;

   scene->stopTimer();

   player->remove<GravityComponent>();
   player->addComponent<FrictionExemptComponent>();

   player->addComponent<WaitUntilComponent>(
       [=](Entity* entity) {
          // Flag is done sliding down
          return entity->hasComponent<BottomCollisionComponent>() &&
                 flag->hasComponent<BottomCollisionComponent>();
       },
       [=](Entity* entity) {
          // Set the player to be on the other side of the flag
          auto* wait = entity->getComponent<WaitUntilComponent>();

          Camera::Get().setCameraFrozen(true);

          playerMove->velocityY = flagMove->velocityY = 0;
          entity->getComponent<TextureComponent>()->setHorizontalFlipped(true);
          playerPosition->position.x += 34;

          wait->condition = [](Entity* entity) {
             static int climbTimer = 0;
             // Delay the sequence for 1 second
             if (climbTimer++ == MAX_FPS) {
                FlagSystem::setClimbing(false);
                climbTimer = 0;
                return true;
             }
             return false;
          };

          wait->doAfter = [=](Entity* entity) {
             // Move towards the castle
         	 Camera::Get().setCameraFrozen(false);

             entity->addComponent<GravityComponent>();

             playerMove->velocityX = 2.0;
             entity->getComponent<TextureComponent>()->setHorizontalFlipped(false);

             wait->condition = [](Entity* entity) {
                // When the player has hit a block
                return entity->hasComponent<RightCollisionComponent>();
             };

             wait->doAfter = [=](Entity* entity) {
                Vector2i nextLevel = scene->getLevelData().nextLevel;

                entity->getComponent<TextureComponent>()->setVisible(false);

                entity->addComponent<CallbackComponent>(
                    [=](Entity* entity) {
                       scene->switchLevel(nextLevel.x, nextLevel.y);
                    },
                    90);
                entity->remove<WaitUntilComponent>();
             };
          };
       });
}

void FlagSystem::hitAxe(World* world, Entity* player, Entity* axe) {
   if (player->hasComponent<WaitUntilComponent>()) {
      return;
   }

   auto* playerMove = player->getComponent<MovingComponent>();

   PlayerSystem::enableInput(false);

   playerMove->velocityX = playerMove->accelerationX = playerMove->velocityY =
       playerMove->accelerationY = 0;

   scene->stopTimer();

   player->remove<GravityComponent>();
   player->addComponent<FrictionExemptComponent>();
   player->addComponent<FrozenComponent>();

   world->destroy(world->findFirst<BridgeChainComponent>());

   Entity* bridge = world->findFirst<BridgeComponent>();

   bridge->addComponent<TimerComponent>(
       [=](Entity* entity) {
          auto* bridgeComponent = entity->getComponent<BridgeComponent>();

          world->destroy(bridgeComponent->connectedBridgeParts.back());

          bridgeComponent->connectedBridgeParts.pop_back();

          bridgeComponent->connectedBridgeParts.shrink_to_fit();

          if (bridgeComponent->connectedBridgeParts.empty()) {
             entity->remove<TimerComponent>();
          }
       },
       5);

   player->addComponent<WaitUntilComponent>(
       [=](Entity* entity) {
          // Bridge is done collapsing
          return !bridge->hasComponent<TimerComponent>();
       },
       [=](Entity* entity) {
          auto* wait = entity->getComponent<WaitUntilComponent>();

          world->destroy(axe);

          playerMove->velocityX = 3.0;

          player->addComponent<GravityComponent>();

          player->remove<FrozenComponent>();

          wait->condition = [=](Entity* entity) {
             return entity->hasComponent<RightCollisionComponent>();
          };

          wait->doAfter = [=](Entity* entity) {
             playerMove->velocityX = 0;

             entity->addComponent<CallbackComponent>(
                 [=](Entity* entity) {
                    Vector2i nextLevel = scene->getLevelData().nextLevel;

                    player->getComponent<TextureComponent>()->setVisible(false);

                    scene->switchLevel(nextLevel.x, nextLevel.y);
                 },
                 240);

             entity->remove<WaitUntilComponent>();
          };
       });
}

void FlagSystem::tick(World* world) {
   world->find<FlagPoleComponent>([&](Entity* entity) {
      Entity* player = world->findFirst<PlayerComponent>();

      if (!AABBTotalCollision(entity->getComponent<PositionComponent>(),
                              player->getComponent<PositionComponent>())) {
         return;
      }

      Entity* flag = world->findFirst<FlagComponent>();

      climbFlag(player, flag);
   });
   world->find<AxeComponent>([&](Entity* entity) {
      Entity* player = world->findFirst<PlayerComponent>();

      if (!AABBTotalCollision(entity->getComponent<PositionComponent>(),
                              player->getComponent<PositionComponent>())) {
         return;
      }

      hitAxe(world, player, entity);
   });
}
