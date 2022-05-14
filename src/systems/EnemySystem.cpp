#include "AABBCollision.h"
#include "Camera.h"
#include "Constants.h"
#include "ECS/Components.h"
#include "ECS/ECS.h"
#include "SoundManager.h"
#include "systems/EnemySystem.h"

#include <SDL2/SDL.h>

#include <iostream>
#include <time.h>

void EnemySystem::performBowserActions(World* world, Entity* entity) {
   if (!Camera::Get().inCameraRange(entity->getComponent<PositionComponent>())) {
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

void EnemySystem::checkEnemyDestroyed(World* world, Entity* enemy) {
   auto* move = enemy->getComponent<MovingComponent>();
   auto* enemyComponent = enemy->getComponent<EnemyComponent>();

   if (!enemy->hasComponent<ParticleComponent>() &&
       enemyComponent->enemyType != EnemyType::PIRANHA_PLANT) {
      // If enemy is crushed
      if (enemy->hasComponent<CrushableComponent, CrushedComponent>()) {
         if (enemyComponent->enemyType != EnemyType::KOOPA) {
            enemy->addComponent<DeadComponent>();
         }
         enemy->getComponent<CrushableComponent>()->whenCrushed(enemy);
         enemy->remove<CrushableComponent>();
         enemy->remove<CrushedComponent>();

         Entity* floatingText(world->create());
         floatingText->addComponent<CreateFloatingTextComponent>(enemy, std::to_string(100));

         Entity* stompSound(world->create());
         stompSound->addComponent<SoundComponent>(SoundID::STOMP);
      }
      // Enemies that were destroyed through either a projectile or super star mario
      if (enemy->hasComponent<EnemyDestroyedComponent>()) {
         move->velocityY = -ENEMY_BOUNCE;
         enemy->addComponent<ParticleComponent>();
         enemy->addComponent<DeadComponent>();
         enemy->getComponent<TextureComponent>()->setVerticalFlipped(true);
         enemy->remove<EnemyDestroyedComponent>();
         enemy->remove<AnimationComponent>();

         Entity* floatingText(world->create());
         floatingText->addComponent<CreateFloatingTextComponent>(enemy, std::to_string(100));

         Entity* destroyedSound(world->create());
         destroyedSound->addComponent<SoundComponent>(SoundID::KICK);
      }
   } else if (!enemy->hasComponent<ParticleComponent>() &&
              enemy->hasComponent<EnemyDestroyedComponent>() &&
              enemyComponent->enemyType == EnemyType::PIRANHA_PLANT) {  // Destroy Pirhanna
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
}

void EnemySystem::tick(World* world) {
   // Projectile bouncing
   world->find<PositionComponent, ProjectileComponent, MovingComponent>([&](Entity* entity) {
      switch (entity->getComponent<ProjectileComponent>()->projectileType) {
         case ProjectileType::FIREBALL:
            if (entity->hasComponent<BottomCollisionComponent>()) {
               entity->getComponent<MovingComponent>()->velocityY = -PROJECTILE_BOUNCE;
               entity->remove<BottomCollisionComponent>();
            }
            break;
         default:
            break;
      }
   });

   // Main enemy update loop
   world->find<EnemyComponent, PositionComponent>([&](Entity* enemy) {
      auto* position = enemy->getComponent<PositionComponent>();
      auto* move = enemy->getComponent<MovingComponent>();
      auto* enemyComponent = enemy->getComponent<EnemyComponent>();

      if (enemyComponent->enemyType == EnemyType::BOWSER) {
         performBowserActions(world, enemy);
      }

      // If the enemy is standing on a block and the block gets hit
      if (enemy->hasComponent<BottomCollisionComponent>()) {
         world->find<BlockBumpComponent>([&](Entity* block) {
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
             enemy->getComponent<EnemyComponent>()->enemyType == EnemyType::FIRE_BAR) {
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
             other->getComponent<MovingComponent>()->velocityX != 0) {
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

      // Moves Koopas in the opposite direction
      if (Camera::Get().inCameraRange(position) && enemyComponent->enemyType == EnemyType::KOOPA) {
         if (!enemy->hasAny<BottomCollisionComponent, DeadComponent>()) {
            move->velocityX *= -1;
            bool horizontalFlipped = enemy->getComponent<TextureComponent>()->isHorizontalFlipped();
            enemy->getComponent<TextureComponent>()->setHorizontalFlipped(!horizontalFlipped);
         }
      }

      if (enemyComponent->enemyType != EnemyType::PIRANHA_PLANT) {
         // Reverses the direction of the enemy when it hits a wall or another enemy
         if (enemy->hasComponent<LeftCollisionComponent>()) {
            if (enemyComponent->enemyType == EnemyType::KOOPA_SHELL) {
               move->velocityX = 6.0;
            } else {
               move->velocityX = ENEMY_SPEED;
            }
            enemy->getComponent<TextureComponent>()->setHorizontalFlipped(true);
            enemy->remove<LeftCollisionComponent>();
         } else if (enemy->hasComponent<RightCollisionComponent>()) {
            if (enemyComponent->enemyType == EnemyType::KOOPA_SHELL) {
               move->velocityX = -6.0;
            } else {
               move->velocityX = -ENEMY_SPEED;
            }
            enemy->getComponent<TextureComponent>()->setHorizontalFlipped(false);
            enemy->remove<RightCollisionComponent>();
         }
      }

      checkEnemyDestroyed(world, enemy);

      enemy->remove<TopCollisionComponent, BottomCollisionComponent, LeftCollisionComponent,
                    RightCollisionComponent>();
   });
}
