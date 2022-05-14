#include "systems/FlagSystem.h"

#include "AABBCollision.h"
#include "Camera.h"
#include "Constants.h"
#include "ECS/Components.h"
#include "systems/PlayerSystem.h"

#include <cmath>
#include <iostream>

bool FlagSystem::climbing = false;

FlagSystem::FlagSystem(GameScene* scene) {
   this->scene = scene;
}

void FlagSystem::climbFlag(World* world, Entity* player, Entity* flag) {
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

   playerMove->velocityY = 4;

   flagMove->velocityY = 4;

   scene->stopTimer();

   scene->stopMusic();

   Entity* flagSound(world->create());
   flagSound->addComponent<SoundComponent>(SoundID::FLAG_RAISE);

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
             if (climbTimer++ == (int)std::round(MAX_FPS * 0.6)) {
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
                wait->condition = [=](Entity* entity) {
                   // Count down the score and timer (if needed)
                   static int nextLevelDelay = (int)std::round(MAX_FPS * 4.5);

                   if (nextLevelDelay > 0) {
                      nextLevelDelay--;
                   }

                   scene->scoreCountdown();

                   if (scene->scoreCountdownFinished() && nextLevelDelay == 0) {
                      nextLevelDelay = (int)std::round(MAX_FPS * 4.5);
                      return true;
                   }
                   return false;
                };
                wait->doAfter = [=](Entity* entity) {
                   Vector2i nextLevel = scene->getLevelData().nextLevel;

                   entity->getComponent<TextureComponent>()->setVisible(false);

                   entity->addComponent<CallbackComponent>(
                       [=](Entity* entity) {
                          scene->switchLevel(nextLevel.x, nextLevel.y);
                       },
                       MAX_FPS * 2);
                   entity->remove<WaitUntilComponent>();
                };
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

   scene->stopMusic();

   player->remove<GravityComponent>();
   player->addComponent<FrictionExemptComponent>();
   player->addComponent<FrozenComponent>();

   world->destroy(world->findFirst<BridgeChainComponent>());

   Entity* bowser = world->findFirst<BowserComponent>();

   bowser->addComponent<FrozenComponent>();

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

          Entity* bridgeCollapseSound(world->create());
          bridgeCollapseSound->addComponent<SoundComponent>(SoundID::BLOCK_BREAK);
       },
       5);

   player->addComponent<WaitUntilComponent>(
       [=](Entity* entity) {
          // Bridge is done collapsing
          return !bridge->hasComponent<TimerComponent>();
       },
       [=](Entity* entity) {
          auto* wait = entity->getComponent<WaitUntilComponent>();

          bowser->remove<FrozenComponent>();
          bowser->addComponent<DeadComponent>();

          Entity* bowserFall(world->create());
          bowserFall->addComponent<SoundComponent>(SoundID::BOWSER_FALL);

          wait->condition = [=](Entity* entity) {
             return !Camera::Get().inCameraRange(bowser->getComponent<PositionComponent>());
          };

          wait->doAfter = [=](Entity* entity) {
             Entity* worldClear(world->create());
             worldClear->addComponent<CallbackComponent>(
                 [=](Entity* entity) {
                    entity->addComponent<SoundComponent>(SoundID::CASTLE_CLEAR);
                 },
                 MAX_FPS * 0.325);

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
          };
       });
}

void FlagSystem::tick(World* world) {
   world->find<FlagPoleComponent>([&](Entity* entity) {
      Entity* player = world->findFirst<PlayerComponent>();

      if (!AABBTotalCollision(entity->getComponent<PositionComponent>(),
                              player->getComponent<PositionComponent>()) ||
          FlagSystem::isClimbing()) {
         return;
      }

      Entity* flag = world->findFirst<FlagComponent>();

      climbFlag(world, player, flag);
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
