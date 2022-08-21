#include "systems/EnemySystem.h"

#include "AABBCollision.h"
#include "Camera.h"
#include "Constants.h"
#include "ECS/Components.h"
#include "ECS/ECS.h"
#include "SoundManager.h"
#include "command/CommandScheduler.h"
#include "command/Commands.h"

#include <SDL2/SDL.h>

#include <iostream>
#include <time.h>

void EnemySystem::performBowserActions(World* world, Entity* entity) {
   if (!Camera::Get().inCameraRange(entity->getComponent<PositionComponent>()) ||
       entity->hasAny<FrozenComponent, DeadComponent>()) {
      return;
   }

   auto* bowserComponent = entity->getComponent<BowserComponent>();
   auto* bowserTexture = entity->getComponent<TextureComponent>();

   Entity* player = world->findFirst<PlayerComponent>();

   bool flipHorizontal = player->getComponent<PositionComponent>()->position.x >
                         entity->getComponent<PositionComponent>()->position.x;

   if ((flipHorizontal != bowserTexture->isHorizontalFlipped())) {
      bowserComponent->lastMoveDirection = (bowserComponent->lastMoveDirection == Direction::LEFT)
                                               ? Direction::RIGHT
                                               : Direction::LEFT;

      bowserTexture->setHorizontalFlipped(flipHorizontal);
   }

   bowserComponent->lastMoveTime++;
   bowserComponent->lastStopTime++;
   bowserComponent->lastJumpTime++;
   bowserComponent->lastAttackTime++;

   switch (bowserComponent->currentMoveIndex) {
      case 0:
         if (bowserComponent->lastStopTime >= MAX_FPS * 2) {
            // Move and jump
            bowserComponent->bowserMovements[0](entity);
            bowserComponent->bowserMovements[2](entity);

            bowserComponent->currentMoveIndex++;
         }
         break;
      case 1:
         if (bowserComponent->lastMoveTime >= MAX_FPS * 3) {
            // Stop
            bowserComponent->bowserMovements[1](entity);

            bowserComponent->currentMoveIndex++;
         }
         break;
      case 2:
         if (bowserComponent->lastStopTime >= MAX_FPS * 2) {
            // Move and jump
            bowserComponent->bowserMovements[0](entity);
            bowserComponent->bowserMovements[2](entity);

            bowserComponent->currentMoveIndex++;
         }
         break;
      case 3:
         if (bowserComponent->lastMoveTime >= MAX_FPS * 3) {
            // Stop
            bowserComponent->bowserMovements[1](entity);

            bowserComponent->currentMoveIndex++;
         }
         break;
      case 4:
         if (bowserComponent->lastStopTime >= MAX_FPS * 2) {
            // Move
            bowserComponent->bowserMovements[0](entity);

            bowserComponent->currentMoveIndex++;
         }
         break;
      case 5:
         if (bowserComponent->lastMoveTime >= MAX_FPS * 3) {
            // Stop
            bowserComponent->bowserMovements[1](entity);

            bowserComponent->currentMoveIndex = 0;
         }
         break;
      default:
         break;
   }

   if (bowserComponent->lastAttackTime >= MAX_FPS * 2) {
      int attackSelect = rand() % bowserComponent->bowserAttacks.size();
      int hammerAmount = rand() % 5 + 6;

      bowserComponent->bowserAttacks[attackSelect](entity, hammerAmount);
   }
}

void EnemySystem::performHammerBroActions(World* world, Entity* entity) {
   auto* position = entity->getComponent<PositionComponent>();
   auto* texture = entity->getComponent<TextureComponent>();
   auto* move = entity->getComponent<MovingComponent>();
   auto* hammerBroComponent = entity->getComponent<HammerBroComponent>();

   if (!Camera::Get().inCameraRange(position)) {
      return;
   }

   Entity* player = world->findFirst<PlayerComponent>();
   auto* playerPosition = player->getComponent<PositionComponent>();

   if (playerPosition->position.x > position->position.x && !texture->isHorizontalFlipped()) {
      texture->setHorizontalFlipped(true);
   } else if (playerPosition->position.x < position->position.x && texture->isHorizontalFlipped()) {
      texture->setHorizontalFlipped(false);
   }

   if (hammerBroComponent->hammer != nullptr) {
      if (!hammerBroComponent->hammer->hasComponent<GravityComponent>()) {
         hammerBroComponent->hammer->getComponent<PositionComponent>()->setCenterX(
             position->getCenterX());
      }
   }

   hammerBroComponent->lastThrowTime++;
   hammerBroComponent->lastJumpTime++;
   hammerBroComponent->lastMoveTime++;

   if (hammerBroComponent->lastThrowTime == MAX_FPS * 2) {
      hammerBroComponent->throwHammer(entity);

      if (hammerBroComponent->lastJumpTime >= MAX_FPS * 4) {
         CommandScheduler::getInstance().addCommand(new DelayedCommand(
             [=]() {
                move->velocity.y = -10;
                hammerBroComponent->lastJumpTime = 0;
             },
             0.75));
      }
   }

   if (hammerBroComponent->lastMoveTime >= MAX_FPS * 3) {
      move->velocity.x *= -1;
      hammerBroComponent->lastMoveTime = 0;
   }
}

void EnemySystem::performLakituActions(World* world, Entity* entity) {
   auto* position = entity->getComponent<PositionComponent>();
   auto* texture = entity->getComponent<TextureComponent>();
   auto* move = entity->getComponent<MovingComponent>();
   auto* lakituComponent = entity->getComponent<LakituComponent>();

   if (!Camera::Get().inCameraRange(position)) {
      return;
   }

   Entity* player = world->findFirst<PlayerComponent>();
   auto* playerPosition = player->getComponent<PositionComponent>();

   if (playerPosition->position.x > position->position.x && !texture->isHorizontalFlipped()) {
      texture->setHorizontalFlipped(true);
   } else if (playerPosition->position.x < position->position.x && texture->isHorizontalFlipped()) {
      texture->setHorizontalFlipped(false);
   }

   lakituComponent->sideChangeTimer++;

   if (lakituComponent->sideChangeTimer >= MAX_FPS * 8) {
      if (lakituComponent->lakituSide == Direction::LEFT) {
         lakituComponent->lakituSide = Direction::RIGHT;
      } else {
         lakituComponent->lakituSide = Direction::LEFT;
      }
      lakituComponent->sideChangeTimer = 0;
   }

   // Lakitu stops harassing you if you're near the flag
   {
      Entity* flag = world->findFirst<FlagComponent>();

      if (flag->getComponent<PositionComponent>()->position.x - playerPosition->position.x <
          30 * SCALED_CUBE_SIZE) {
         move->velocity.x = -4.0;
         return;
      }
   }

   // If not near the flag, move lakitu to the desired side of the screen
   if (lakituComponent->lakituSide == Direction::RIGHT) {
      move->velocity.x = lakituComponent->speedController.calculate(
          position->position.x, Camera::Get().getCameraCenterX() + SCALED_CUBE_SIZE * 6);
   } else {
      move->velocity.x = lakituComponent->speedController.calculate(
          position->position.x, Camera::Get().getCameraCenterX() - SCALED_CUBE_SIZE * 6);
   }
   // Limits the speed to prevent lakitu from going zoooooooooooooooom
   if (std::abs(move->velocity.x) > 6.0) {
      move->velocity.x = (move->velocity.x > 0) ? 6.0 : -6.0;
   }
}

void EnemySystem::checkEnemyDestroyed(World* world, Entity* enemy) {
   if (enemy->hasComponent<DeadComponent>()) {
      return;
   }

   auto* move = enemy->getComponent<MovingComponent>();
   auto* enemyComponent = enemy->getComponent<EnemyComponent>();

   if (enemyComponent->enemyType == EnemyType::PIRANHA_PLANT) {  // Destroy Pirhanna
      if (enemy->hasComponent<EnemyDestroyedComponent>()) {
         enemy->addComponent<ParticleComponent>();
         enemy->addComponent<DeadComponent>();
         enemy->remove<EnemyDestroyedComponent>();
         enemy->remove<AnimationComponent>();

         Entity* floatingText(world->create());
         floatingText->addComponent<CreateFloatingTextComponent>(enemy, std::to_string(100));

         Entity* destroyedSound(world->create());
         destroyedSound->addComponent<SoundComponent>(SoundID::KICK);

         enemy->addComponent<DestroyDelayedComponent>(1);
      }
      return;
   }

   // If enemy is crushed
   if (enemy->hasComponent<CrushableComponent, CrushedComponent>()) {
      // When the paratroopa gets crushed it's still crushable
      bool removeCrushable = enemyComponent->enemyType != EnemyType::KOOPA_PARATROOPA;
      enemy->getComponent<CrushableComponent>()->whenCrushed(enemy);
      enemy->remove<CrushedComponent>();

      if (removeCrushable) {
         enemy->remove<CrushableComponent>();
      }

      Entity* floatingText(world->create());
      floatingText->addComponent<CreateFloatingTextComponent>(enemy, std::to_string(100));

      Entity* stompSound(world->create());
      stompSound->addComponent<SoundComponent>(SoundID::STOMP);
   }

   // Enemies that were destroyed through either a projectile or super star mario
   if (enemy->hasComponent<EnemyDestroyedComponent>()) {
      if (enemyComponent->enemyType != EnemyType::BULLET_BILL) {
         move->velocity.y = -ENEMY_BOUNCE;
         enemy->getComponent<TextureComponent>()->setVerticalFlipped(true);
      }
      enemy->addComponent<ParticleComponent>();
      enemy->addComponent<DeadComponent>();
      enemy->addComponent<DestroyOutsideCameraComponent>();

      enemy->remove<EnemyDestroyedComponent>();
      enemy->remove<AnimationComponent>();

      Entity* floatingText(world->create());
      floatingText->addComponent<CreateFloatingTextComponent>(enemy, std::to_string(100));

      Entity* destroyedSound(world->create());
      destroyedSound->addComponent<SoundComponent>(SoundID::KICK);
   }
}

void EnemySystem::tick(World* world) {
   /* Projectile bouncing */
   world->find<PositionComponent, ProjectileComponent, MovingComponent>([&](Entity* entity) {
      switch (entity->getComponent<ProjectileComponent>()->projectileType) {
         case ProjectileType::FIREBALL:
            if (entity->hasComponent<BottomCollisionComponent>()) {
               entity->getComponent<MovingComponent>()->velocity.y = -PROJECTILE_BOUNCE;
               entity->remove<BottomCollisionComponent>();
            }
            break;
         default:
            break;
      }
   });

   /* Main enemy update loop */
   world->find<EnemyComponent, PositionComponent>([&](Entity* enemy) {
      auto* position = enemy->getComponent<PositionComponent>();
      auto* move = enemy->getComponent<MovingComponent>();
      auto* enemyComponent = enemy->getComponent<EnemyComponent>();

      EnemyType enemyType = enemyComponent->enemyType;

      // Perform actions unique to each type of enemy
      switch (enemyType) {
         case EnemyType::BOWSER:
            performBowserActions(world, enemy);
            break;
         case EnemyType::HAMMER_BRO:
            performHammerBroActions(world, enemy);
            break;
         case EnemyType::LAKITU:
            performLakituActions(world, enemy);
            break;
         case EnemyType::SPINE: {
            // Turn spine eggs into spiny shells when they hit the ground
            auto* animation = enemy->getComponent<AnimationComponent>();
            if (enemy->hasComponent<BottomCollisionComponent>() &&
                animation->frameIDS != std::vector<int>{502, 503}) {
               animation->frameIDS = std::vector<int>{502, 503};
               animation->currentFrame = 0;
               animation->frameTimer = 0;
            }
         } break;
         case EnemyType::LAVA_BUBBLE: {
            auto* texture = enemy->getComponent<TextureComponent>();
            // If going up and upside down
            if (move->velocity.y <= 0 && texture->isVerticalFlipped()) {
               texture->setVerticalFlipped(false);
            } else if (move->velocity.y > 0 && !texture->isVerticalFlipped()) {
               // If going down and not upside down
               texture->setVerticalFlipped(true);
            }
         } break;
         default:
            break;
      }

      // If the enemy is standing on a block and the block gets hit
      if (enemy->hasComponent<BottomCollisionComponent>()) {
         world->find<BlockBumpComponent>([enemy](Entity* block) {
            if (AABBCollision(enemy->getComponent<PositionComponent>(),
                              block->getComponent<PositionComponent>())) {
               enemy->addComponent<EnemyDestroyedComponent>();
            }
         });
      }

      // Enemy + Projectile collisions
      world->find<ProjectileComponent, MovingComponent>([&](Entity* projectile) {
         auto* projectilePosition = projectile->getComponent<PositionComponent>();
         if (!AABBCollision(position, projectilePosition) ||
             enemy->hasAny<ProjectileComponent, ParticleComponent>() ||
             enemyType == EnemyType::LAVA_BUBBLE || enemyType == EnemyType::FIRE_BAR ||
             enemyType == EnemyType::BULLET_BILL) {
            return;
         }
         if (enemy->hasComponent<BowserComponent>()) {
            if (projectile->getComponent<ProjectileComponent>()->projectileType ==
                ProjectileType::FIREBALL) {
               // Decrease HP
            }
            return;
         }
         enemy->addComponent<EnemyDestroyedComponent>();
         world->destroy(projectile);
      });

      // Enemy + Enemy Collision (prevents to enemies from walking through each other)
      world->find<EnemyComponent, MovingComponent>([&](Entity* other) {
         auto* otherPosition = other->getComponent<PositionComponent>();
         if (!AABBCollision(position, otherPosition) || enemy == other ||
             enemy->hasAny<DeadComponent, PiranhaPlantComponent>() ||
             other->hasComponent<ParticleComponent>()) {
            return;
         }
         if (other->getComponent<EnemyComponent>()->enemyType == EnemyType::KOOPA_SHELL &&
             other->getComponent<MovingComponent>()->velocity.x != 0) {
            enemy->addComponent<EnemyDestroyedComponent>();
            enemy->addComponent<MoveOutsideCameraComponent>();

            Entity* addScore(world->create());
            addScore->addComponent<AddScoreComponent>(100);
            return;
         }
         // If the other enemy is to the left
         if (otherPosition->getLeft() < position->getLeft() &&
             otherPosition->getRight() < position->getRight()) {
            other->addComponent<RightCollisionComponent>();
         }
         // If the other enemy is to the right
         if (otherPosition->getLeft() > position->getLeft() &&
             otherPosition->getRight() > position->getRight()) {
            other->addComponent<LeftCollisionComponent>();
         }
      });

      // Moves Koopas in the opposite direction if not on the ground
      if (Camera::Get().inCameraRange(position) && enemyType == EnemyType::KOOPA) {
         if (!enemy->hasAny<BottomCollisionComponent, DeadComponent>()) {
            move->velocity.x *= -1;
            bool horizontalFlipped = enemy->getComponent<TextureComponent>()->isHorizontalFlipped();
            enemy->getComponent<TextureComponent>()->setHorizontalFlipped(!horizontalFlipped);
         }
      }

      if (enemyType != EnemyType::PIRANHA_PLANT && enemyType != EnemyType::CHEEP_CHEEP &&
          enemyType != EnemyType::BLOOPER && enemyType != EnemyType::LAKITU &&
          enemyType != EnemyType::LAVA_BUBBLE && enemyType != EnemyType::BULLET_BILL) {
         // Reverses the direction of the enemy when it hits a wall or another enemy
         if (enemy->hasComponent<LeftCollisionComponent>()) {
            move->velocity.x = (enemyType == EnemyType::KOOPA_SHELL) ? 6.0 : ENEMY_SPEED;

            enemy->getComponent<TextureComponent>()->setHorizontalFlipped(true);

            enemy->remove<LeftCollisionComponent>();
         } else if (enemy->hasComponent<RightCollisionComponent>()) {
            move->velocity.x = (enemyType == EnemyType::KOOPA_SHELL) ? -6.0 : -ENEMY_SPEED;

            enemy->getComponent<TextureComponent>()->setHorizontalFlipped(false);

            enemy->remove<RightCollisionComponent>();
         }
      }

      checkEnemyDestroyed(world, enemy);

      if (enemyType != EnemyType::KOOPA_PARATROOPA) {
         enemy->remove<BottomCollisionComponent>();
      }

      enemy->remove<TopCollisionComponent, LeftCollisionComponent, RightCollisionComponent>();
   });
}
