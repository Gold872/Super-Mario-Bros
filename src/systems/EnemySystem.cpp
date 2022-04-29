#include "systems/EnemySystem.h"

#include "AABBCollision.h"
#include "Camera.h"
#include "Constants.h"
#include "ECS/Components.h"
#include "ECS/ECS.h"

#include <iostream>

void EnemySystem::tick(World* world) {
   world->find<EnemyComponent, PositionComponent>([&](Entity* enemy) {
      auto* position = enemy->getComponent<PositionComponent>();
      auto* move = enemy->getComponent<MovingComponent>();
      auto* enemyComponent = enemy->getComponent<EnemyComponent>();

      // If the enemy is standing on a block and the block gets hit
      if (enemy->hasComponent<BottomCollisionComponent>()) {
         world->find<BlockBumpComponent>([&](Entity* block) {
            if (AABBCollision(enemy->getComponent<PositionComponent>(),
                              block->getComponent<PositionComponent>())) {
               enemy->addComponent<EnemyDestroyedComponent>();
            }
         });
      }

      world->find<ProjectileComponent, MovingComponent>([&](Entity* projectile) {
         auto* projectilePosition = projectile->getComponent<PositionComponent>();
         if (!AABBCollision(position, projectilePosition) ||
             enemy->hasComponent<ProjectileComponent>()) {
            return;
         }
         enemy->addComponent<EnemyDestroyedComponent>();
      });

      // Enemy + Enemy Collision (prevents to enemies from walking through each other
      world->find<EnemyComponent, MovingComponent>([&](Entity* other) {
         auto* otherPosition = other->getComponent<PositionComponent>();
         if (!AABBCollision(position, otherPosition) || enemy == other ||
             enemy->hasComponent<DeadComponent>() || other->hasComponent<ParticleComponent>()) {
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

      if (Camera::Get().inCameraRange(position) && enemyComponent->enemyType == EnemyType::KOOPA) {
         if (!enemy->hasAny<BottomCollisionComponent, DeadComponent>()) {
            move->velocityX *= -1;
            bool horizontalFlipped = enemy->getComponent<TextureComponent>()->isHorizontalFlipped();
            enemy->getComponent<TextureComponent>()->setHorizontalFlipped(!horizontalFlipped);
         }
      }

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

      if (enemy->hasComponent<CrushableComponent, CrushedComponent>()) {
         if (enemyComponent->enemyType != EnemyType::KOOPA) {
            enemy->addComponent<DeadComponent>();
         }
         enemy->getComponent<CrushableComponent>()->whenCrushed(enemy);
         enemy->remove<CrushableComponent>();
         enemy->remove<CrushedComponent>();
      }
      // Enemies that were destroyed through either a projectile or super star mario
      if (enemy->hasComponent<EnemyDestroyedComponent>()) {
         move->velocityY = -ENEMY_BOUNCE;
         enemy->addComponent<ParticleComponent>();
         enemy->addComponent<DeadComponent>();
         enemy->getComponent<TextureComponent>()->setVerticalFlipped(true);
         enemy->remove<EnemyDestroyedComponent>();
         enemy->remove<AnimationComponent>();
      }

      enemy->remove<TopCollisionComponent, BottomCollisionComponent, LeftCollisionComponent,
                    RightCollisionComponent>();
   });
}
