#include "systems/FlagSystem.h"

#include "AABBCollision.h"
#include "Camera.h"
#include "Constants.h"
#include "ECS/Components.h"
#include "command/CommandScheduler.h"
#include "command/Commands.h"
#include "systems/PlayerSystem.h"

#include <cmath>
#include <iostream>
#include <vector>

bool FlagSystem::climbing = false;

FlagSystem::FlagSystem(GameScene* scene) {
   this->scene = scene;
}

void FlagSystem::climbFlag(World* world, Entity* player, Entity* flag) {
   static bool inSequence = false;

   if (inSequence) {
      return;
   }

   auto* playerMove = player->getComponent<MovingComponent>();
   auto* playerPosition = player->getComponent<PositionComponent>();
   auto* flagMove = flag->getComponent<MovingComponent>();

   PlayerSystem::enableInput(false);

   FlagSystem::setClimbing(true);

   playerPosition->setLeft(flag->getComponent<PositionComponent>()->getLeft());

   playerMove->velocity.x = playerMove->acceleration.x = playerMove->acceleration.y = 0;

   playerMove->velocity.y = 4;

   flagMove->velocity.y = 4;

   scene->stopTimer();

   scene->stopMusic();

   Entity* flagSound(world->create());
   flagSound->addComponent<SoundComponent>(SoundID::FLAG_RAISE);

   player->remove<GravityComponent>();
   player->addComponent<FrictionExemptComponent>();

   inSequence = true;

   CommandScheduler::getInstance().addCommand(new SequenceCommand(std::vector<Command*>{
       /* Move to the other side of the flag */
       new WaitUntilCommand([=]() -> bool {
          return player->hasComponent<BottomCollisionComponent>() &&
                 flag->hasComponent<BottomCollisionComponent>();
       }),
       new RunCommand([=]() {
          playerMove->velocity.y = flagMove->velocity.y = 0;
          player->getComponent<TextureComponent>()->setHorizontalFlipped(true);
          playerPosition->position.x += 34;
       }),
       new WaitCommand(0.6),
       /* Move towards the castle */
       new RunCommand([=]() {
          FlagSystem::setClimbing(false);

          Camera::Get().setCameraFrozen(false);

          player->addComponent<GravityComponent>();

          playerMove->velocity.x = 2.0;
          player->getComponent<TextureComponent>()->setHorizontalFlipped(false);
       }),
       /* Wait until the player hits a solid block */
       new WaitUntilCommand([=]() -> bool {
          return player->hasComponent<RightCollisionComponent>();
       }),
       new WaitUntilCommand([=]() -> bool {
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
       }),
       new RunCommand([=]() mutable {
          Vector2i nextLevel = scene->getLevelData().nextLevel;

          player->getComponent<TextureComponent>()->setVisible(false);

          inSequence = false;

          CommandScheduler::getInstance().addCommand(new DelayedCommand(
              [=]() {
                 scene->switchLevel(nextLevel.x, nextLevel.y);
              },
              2.0));
       })}));
}

void FlagSystem::hitAxe(World* world, Entity* player, Entity* axe) {
   static bool inSequence = false;

   if (inSequence) {
      return;
   }

   PlayerSystem::enableInput(false);

   auto* playerMove = player->getComponent<MovingComponent>();

   playerMove->velocity.x = playerMove->acceleration.x = playerMove->velocity.y =
       playerMove->acceleration.y = 0;

   scene->stopTimer();

   scene->stopMusic();

   player->remove<GravityComponent>();
   player->addComponent<FrictionExemptComponent>();
   player->addComponent<FrozenComponent>();

   inSequence = true;

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

   CommandScheduler::getInstance().addCommand(new SequenceCommand(std::vector<Command*>{
       /* Wait until the bridge is done collapsing, then make bowser fall */
       new WaitUntilCommand([=]() -> bool {
          return !bridge->hasComponent<TimerComponent>();
       }),
       new RunCommand([=]() {
          bowser->remove<FrozenComponent>();
          bowser->addComponent<DeadComponent>();

          Entity* bowserFall(world->create());
          bowserFall->addComponent<SoundComponent>(SoundID::BOWSER_FALL);
       }),
       /* Wait until bowser is not visible in the camera, then destroy the axe and move the player
        */
       new WaitUntilCommand([bowser]() -> bool {
          return !Camera::Get().inCameraRange(bowser->getComponent<PositionComponent>());
       }),
       new RunCommand([=]() {
          // Play the castle clear sound in 0.325 seconds, this is separate from the sequence to
          // avoid sequence interruption
          CommandScheduler::getInstance().addCommand(new DelayedCommand(
              [=]() {
                 Entity* castleClear(world->create());

                 castleClear->addComponent<SoundComponent>(SoundID::CASTLE_CLEAR);
              },
              0.325));

          world->destroy(axe);

          playerMove->velocity.x = 3.0;

          player->addComponent<GravityComponent>();

          player->remove<FrozenComponent>();
       }),
       /* Wait until mario runs into a block, then stop and switch to the next level */
       new WaitUntilCommand([player]() -> bool {
          return player->hasComponent<RightCollisionComponent>();
       }),
       new RunCommand([=]() mutable {
          inSequence = false;

          CommandScheduler::getInstance().addCommand(new DelayedCommand(
              [=]() {
                 Vector2i nextLevel = scene->getLevelData().nextLevel;

                 player->getComponent<TextureComponent>()->setVisible(false);

                 scene->switchLevel(nextLevel.x, nextLevel.y);
              },
              5.0));
       }),
   }));
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
