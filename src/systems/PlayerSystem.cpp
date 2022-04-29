#include "systems/PlayerSystem.h"

#include "AABBCollision.h"
#include "Camera.h"
#include "Constants.h"
#include "ECS/Components.h"
#include "Level.h"
#include "Map.h"
#include "Math.h"
#include "TextureManager.h"
#include "systems/FlagSystem.h"
#include "systems/WarpSystem.h"

#include <SDL2/SDL.h>

#include <cmath>
#include <iostream>
#include <memory>

bool PlayerSystem::inputEnabled = true;
bool PlayerSystem::inGameStart = false;

int running = false;

bool jumpHeld = 0;

bool gameOver = false;

bool underwater = false;

PIDController accelerationControllerY(0.4, 0, 0.02, 60);

PlayerSystem::PlayerSystem(GameScene* scene) {
   this->scene = scene;
}

Entity* PlayerSystem::createFloatingText(World* world, Entity* destroyedEnemy, std::string text) {
   auto* enemyPosition = destroyedEnemy->getComponent<PositionComponent>();

   Entity* scoreText(world->create());
   scoreText->addComponent<PositionComponent>(
       Vector2f(enemyPosition->getCenterX(), enemyPosition->getTop() - 4), Vector2i());
   scoreText->addComponent<MovingComponent>(0, -1, 0, 0);
   scoreText->addComponent<TextComponent>(text, 10, true);
   scoreText->addComponent<FloatingTextComponent>();
   scoreText->addComponent<DestroyDelayedComponent>(35);

   return scoreText;
}

void PlayerSystem::setUnderwater(bool val) {
   underwater = val;
}

void PlayerSystem::onGameOver(bool outOfBounds) {
   auto* position = mario->getComponent<PositionComponent>();
   auto* texture = mario->getComponent<TextureComponent>();

   if (outOfBounds && mario->hasComponent<SuperMarioComponent>()) {
      position->scale.y = position->hitbox.h = SCALED_CUBE_SIZE;
      texture->setEntityHeight(ORIGINAL_CUBE_SIZE);
      texture->setSpritesheetCoordinates(Map::PlayerIDCoordinates.at(1));

      mario->remove<SuperMarioComponent>();
   }

   if (!outOfBounds && mario->hasComponent<SuperMarioComponent>()) {
      shrink();
      return;
   }

   auto* move = mario->getComponent<MovingComponent>();

   move->velocityX = move->accelerationX = 0;
   move->accelerationY = 0;
   move->velocityY = -10.0f;

   texture->setSpritesheetCoordinates(Map::PlayerIDCoordinates.at(1));

   mario->addComponent<ParticleComponent>();
   currentState = GAMEOVER;

   scene->stopTimer();

   mario->addComponent<CallbackComponent>(
       [=](Entity* entity) {
          entity->remove<ParticleComponent>();
          scene->restartLevel();
       },
       90);
}

void PlayerSystem::setState(Animation_State newState) {
   auto* texture = mario->getComponent<TextureComponent>();

   switch (newState) {
      case STANDING:
         if (!mario->hasComponent<FrozenComponent>()) {
            if (mario->hasComponent<AnimationComponent>()) {
               mario->remove<AnimationComponent>();
            }
            if (mario->hasComponent<SuperMarioComponent>()) {
               texture->setSpritesheetCoordinates(Map::PlayerIDCoordinates.at(25));
            } else {
               texture->setSpritesheetCoordinates(Map::PlayerIDCoordinates.at(0));
            }
         }
         break;
      case WALKING:
         if (!mario->hasComponent<FrozenComponent>()) {
            std::vector<int> superFrameIDS = {27, 28, 29};
            std::vector<int> normalFrameIDS = {2, 3, 4};
            if (!mario->hasComponent<AnimationComponent>()) {
               if (mario->hasComponent<SuperMarioComponent>()) {
                  mario->addComponent<AnimationComponent>(superFrameIDS, 3, 8,
                                                          Map::PlayerIDCoordinates);
               } else {
                  mario->addComponent<AnimationComponent>(normalFrameIDS, 3, 8,
                                                          Map::PlayerIDCoordinates);
               }
               return;
            }
            if (mario->getComponent<AnimationComponent>()->frameIDS != superFrameIDS &&
                mario->getComponent<AnimationComponent>()->frameIDS != normalFrameIDS) {
               // If the player already has an animation but it is not the correct one
               auto* animation = mario->getComponent<AnimationComponent>();

               if (mario->hasComponent<SuperMarioComponent>()) {
                  animation->frameIDS = superFrameIDS;
                  animation->frameCount = 3;
               } else {
                  animation->frameIDS = normalFrameIDS;
                  animation->frameCount = 3;
               }
            }
            if (running) {
               mario->getComponent<AnimationComponent>()->setFramesPerSecond(12);
            } else {
               mario->getComponent<AnimationComponent>()->setFramesPerSecond(8);
            }
         }
         break;
      case DRIFTING:
         if (!mario->hasComponent<FrozenComponent>()) {
            if (mario->hasComponent<AnimationComponent>()) {
               mario->remove<AnimationComponent>();
            }
            if (mario->hasComponent<SuperMarioComponent>()) {
               texture->setSpritesheetCoordinates(Map::PlayerIDCoordinates.at(30));
            } else {
               texture->setSpritesheetCoordinates(Map::PlayerIDCoordinates.at(5));
            }
         }
         break;
      case JUMPING:
         if (!mario->hasComponent<FrozenComponent>()) {
            if (mario->hasComponent<AnimationComponent>()) {
               mario->remove<AnimationComponent>();
            }
            if (mario->hasComponent<SuperMarioComponent>()) {
               texture->setSpritesheetCoordinates(Map::PlayerIDCoordinates.at(31));
            } else {
               texture->setSpritesheetCoordinates(Map::PlayerIDCoordinates.at(6));
            }
         }
         break;
      case DUCKING:
         if (!mario->hasComponent<FrozenComponent>()) {
            if (mario->hasComponent<AnimationComponent>()) {
               mario->remove<AnimationComponent>();
            }
            texture->setSpritesheetCoordinates(Map::PlayerIDCoordinates.at(26));
         }
         break;
      case CLIMBING: {
         std::vector<int> superFrameIDS = {38, 39};
         std::vector<int> normalFrameIDS = {13, 14};
         if (!mario->hasComponent<AnimationComponent>()) {
            if (mario->hasComponent<SuperMarioComponent>()) {
               mario->addComponent<AnimationComponent>(superFrameIDS, 2, 8,
                                                       Map::PlayerIDCoordinates);
            } else {
               mario->addComponent<AnimationComponent>(normalFrameIDS, 2, 8,
                                                       Map::PlayerIDCoordinates);
            }
         } else if (mario->getComponent<AnimationComponent>()->frameIDS != superFrameIDS &&
                    mario->getComponent<AnimationComponent>()->frameIDS != normalFrameIDS) {
            // If the player already has an animation but it is not the correct one
            auto* animation = mario->getComponent<AnimationComponent>();

            if (mario->hasComponent<SuperMarioComponent>()) {
               animation->frameIDS = superFrameIDS;
               animation->frameCount = 2;
            } else {
               animation->frameIDS = normalFrameIDS;
               animation->frameCount = 2;
            }
         }
      } break;
      case SLIDING:
         if (mario->hasComponent<AnimationComponent>()) {
            mario->remove<AnimationComponent>();
         }
         if (mario->hasComponent<SuperMarioComponent>()) {
            texture->setSpritesheetCoordinates(Map::PlayerIDCoordinates.at(38));
         } else {
            texture->setSpritesheetCoordinates(Map::PlayerIDCoordinates.at(13));
         }
         break;
      case GAMEOVER:
         if (mario->hasComponent<AnimationComponent>()) {
            mario->remove<AnimationComponent>();
         }
         texture->setSpritesheetCoordinates(Map::PlayerIDCoordinates.at(1));
         break;
      default:
         break;
   }
}

void PlayerSystem::grow(World* world, Grow_Type growType) {
   switch (growType) {
      case ONEUP:
         break;
      case MUSHROOM: {
         Entity* addScore(world->create());
         addScore->addComponent<AddScoreComponent>(1000);

         if (mario->hasComponent<SuperMarioComponent>()) {
            return;
         }

         mario->addComponent<SuperMarioComponent>();

         auto position = mario->getComponent<PositionComponent>();

         auto texture = mario->getComponent<TextureComponent>();

         position->setTop(position->getTop() - position->scale.y);  // Makes the player taller

         position->scale.y = position->hitbox.h = SCALED_CUBE_SIZE * 2;

         texture->setEntityHeight(ORIGINAL_CUBE_SIZE * 2);

         mario->addComponent<AnimationComponent>(
             std::vector<int>{46, 45, 25, 46, 45, 25, 46, 45, 25}, 9, 12, Map::PlayerIDCoordinates,
             false);

         mario->addComponent<FrozenComponent>();

         mario->addComponent<CallbackComponent>(
             [&](Entity* mario) {
                mario->remove<FrozenComponent>();
             },
             45);
      } break;
      default:
         break;
   }
}

void PlayerSystem::shrink() {
   mario->remove<SuperMarioComponent>();

   mario->addComponent<AnimationComponent>(std::vector<int>{25, 45, 46, 25, 45, 46, 25, 45, 46}, 9,
                                           12, Map::PlayerIDCoordinates, false);

   mario->addComponent<FrozenComponent>();

   mario->addComponent<CallbackComponent>(
       [&](Entity* mario) {
          auto* position = mario->getComponent<PositionComponent>();
          auto* texture = mario->getComponent<TextureComponent>();

          // Shortens the player
          position->scale.y = position->hitbox.h = SCALED_CUBE_SIZE;
          texture->setEntityHeight(ORIGINAL_CUBE_SIZE);
          texture->setSpritesheetCoordinates(Map::PlayerIDCoordinates.at(0));
          mario->remove<FrozenComponent>();
          mario->addComponent<EndingBlinkComponent>(10, 150);
       },
       45);
}

void PlayerSystem::createBlockDebris(World* world, Entity* block) {
   auto* blockPosition = block->getComponent<PositionComponent>();
   auto* blockTexture = block->getComponent<TextureComponent>();

   // Getting the texture from the block prevents memory leaks
   std::shared_ptr<SDL_Texture> debrisTexture = blockTexture->getTexture();

   Entity* debris1(world->create());  // Top Left

   debris1->addComponent<PositionComponent>(
       Vector2f(blockPosition->getLeft(), blockPosition->getTop() - SCALED_CUBE_SIZE),
       Vector2i(SCALED_CUBE_SIZE, SCALED_CUBE_SIZE));

   debris1->addComponent<GravityComponent>();

   debris1->addComponent<MovingComponent>(-8.0f, -2.0f);

   debris1->addComponent<TextureComponent>(
       debrisTexture, ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE, 1, 1, 1, ORIGINAL_CUBE_SIZE,
       ORIGINAL_CUBE_SIZE, block->getComponent<DestructibleComponent>()->debrisCoordinates, true);

   debris1->addComponent<ParticleComponent>();
   /* ******************************************************************* */
   Entity* debris2(world->create());  // Top Right

   debris2->addComponent<PositionComponent>(
       Vector2f(blockPosition->getLeft(), blockPosition->getTop() - SCALED_CUBE_SIZE),
       Vector2i(SCALED_CUBE_SIZE, SCALED_CUBE_SIZE));

   debris2->addComponent<GravityComponent>();

   debris2->addComponent<MovingComponent>(8.0f, -2.0f);

   debris2->addComponent<TextureComponent>(
       debrisTexture, ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE, 1, 1, 1, ORIGINAL_CUBE_SIZE,
       ORIGINAL_CUBE_SIZE, block->getComponent<DestructibleComponent>()->debrisCoordinates);

   debris2->addComponent<ParticleComponent>();
   /* ******************************************************************* */
   Entity* debris3(world->create());  // Bottom Left

   debris3->addComponent<PositionComponent>(
       Vector2f(blockPosition->getLeft(), blockPosition->getTop()),
       Vector2i(SCALED_CUBE_SIZE, SCALED_CUBE_SIZE));

   debris3->addComponent<GravityComponent>();

   debris3->addComponent<MovingComponent>(-8.0f, -2.0f);

   debris3->addComponent<TextureComponent>(
       debrisTexture, ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE, 1, 1, 1, ORIGINAL_CUBE_SIZE,
       ORIGINAL_CUBE_SIZE, block->getComponent<DestructibleComponent>()->debrisCoordinates, true);

   debris3->addComponent<ParticleComponent>();
   /* ******************************************************************* */
   Entity* debris4(world->create());  // Bottom Right

   debris4->addComponent<PositionComponent>(
       Vector2f(blockPosition->getLeft(), blockPosition->getTop()),
       Vector2i(SCALED_CUBE_SIZE, SCALED_CUBE_SIZE));

   debris4->addComponent<GravityComponent>();

   debris4->addComponent<MovingComponent>(8.0f, -2.0f);

   debris4->addComponent<TextureComponent>(
       debrisTexture, ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE, 1, 1, 1, ORIGINAL_CUBE_SIZE,
       ORIGINAL_CUBE_SIZE, block->getComponent<DestructibleComponent>()->debrisCoordinates);

   debris4->addComponent<ParticleComponent>();
}

void PlayerSystem::onAddedToWorld(World* world) {
   if (scene->getLevelData().levelType == LevelType::UNDERWATER) {
      underwater = true;
   }

   // Creates the player's entity
   std::shared_ptr<SDL_Texture> playerTexture =
       TextureManager::Get().LoadSharedTexture("res/sprites/characters/PlayerSpriteSheet.png");

   mario = world->create();
   Vector2i startCoordinates = scene->getLevelData().playerStart;

   mario->addComponent<PositionComponent>(
       Vector2f(startCoordinates.x * SCALED_CUBE_SIZE, startCoordinates.y * SCALED_CUBE_SIZE),
       Vector2i(SCALED_CUBE_SIZE, SCALED_CUBE_SIZE));
   mario->addComponent<TextureComponent>(playerTexture, ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE, 1,
                                         9, 0, ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE,
                                         Map::PlayerIDCoordinates.at(0), false, false);
   mario->addComponent<MovingComponent>(0, 0, 0, 0);
   mario->addComponent<GravityComponent>();
   mario->addComponent<PlayerComponent>();
}

void PlayerSystem::reset() {
   Vector2i startCoordinates = scene->getLevelData().playerStart;

   auto* position = mario->getComponent<PositionComponent>();
   auto* move = mario->getComponent<MovingComponent>();

   position->position.x = startCoordinates.x * SCALED_CUBE_SIZE;
   position->position.y =
       (mario->getComponent<PositionComponent>()->scale.y == SCALED_CUBE_SIZE * 2)
           ? (startCoordinates.y - 1) * SCALED_CUBE_SIZE
           : startCoordinates.y * SCALED_CUBE_SIZE;

   move->velocityY = move->accelerationY = 0;
   move->velocityX = move->accelerationX = 0;

   mario->getComponent<TextureComponent>()->setVisible(true);

   if (scene->getLevelData().levelType != LevelType::START_UNDERGROUND) {
      mario->addComponent<GravityComponent>();

      mario->remove<FrictionExemptComponent>();
      mario->remove<CollisionExemptComponent>();

      Camera::Get().setCameraFrozen(false);
      Camera::Get().setCameraX(0);
      Camera::Get().setCameraY(0);

      PlayerSystem::enableInput(true);
   } else {
      Camera::Get().setCameraFrozen(true);
      Camera::Get().setCameraX(0);
      Camera::Get().setCameraY(0);

      PlayerSystem::setGameStart(true);
      PlayerSystem::enableInput(false);

      mario->addComponent<FrictionExemptComponent>();
      mario->addComponent<CollisionExemptComponent>();

      mario->remove<GravityComponent>();

      move->velocityX = 2.0;
   }

   currentState = STANDING;
}

// Updates the velocity and acceleration for when mario is on the ground
void PlayerSystem::updateGroundVelocity() {
   auto* texture = mario->getComponent<TextureComponent>();
   auto* move = mario->getComponent<MovingComponent>();

   // The textures only get flipped if the player is on the ground
   if (left) {
      texture->setHorizontalFlipped(true);
   } else if (right) {
      texture->setHorizontalFlipped(false);
   }
   // Updates the acceleration
   move->accelerationX = (float)xDir * MARIO_ACCELERATION_X;
   if (mario->hasComponent<SuperStarComponent>()) {
      move->accelerationX *= 1.5957446808510638297f;
   } else if (running) {
      move->accelerationX *= 1.3297872340425531914f;
   } else {
      move->accelerationX *= 0.7978723404255319148936f;
   }

   if (jump && !jumpHeld) {
      jumpHeld = true;
      move->velocityY = -10.3;
   }
   if (duck && mario->hasComponent<SuperMarioComponent>()) {
      currentState = DUCKING;
      move->accelerationX = 0;
      // Slows the player down
      if (move->velocityX > 1.5) {
         move->velocityX -= 0.5;
      } else if (move->velocityX < -1.5) {
         move->velocityX += 0.5;
      }
   } else if ((bool)std::abs(move->velocityX) || (bool)std::abs(move->accelerationX)) {
      // If the player should be drifting
      if ((move->velocityX > 0 && move->accelerationX < 0) ||
          (move->velocityX < 0 && move->accelerationX > 0)) {
         currentState = DRIFTING;
      } else {
         currentState = WALKING;
      }
   } else {
      currentState = STANDING;
   }
}

void PlayerSystem::updateAirVelocity() {
   auto* move = mario->getComponent<MovingComponent>();
   // If the player is in the air
   move->accelerationX = (float)xDir * MARIO_ACCELERATION_X;
   if (running) {
      if ((move->accelerationX >= 0 && move->velocityX >= 0) ||
          (move->accelerationX <= 0 && move->velocityX <= 0)) {
         // If the acceleration and velocity aren't in opposite directions
         if (mario->hasComponent<SuperStarComponent>()) {
            move->accelerationX *= 1.5957446808510638297f;
         } else {
            move->accelerationX *= 1.3297872340425531914f;
         }
      } else {
         move->accelerationX *= 0.35f;
      }
   }
   // Changes mario's acceleration while in the air (the longer you jump the higher mario
   // will go)
   if (jump && move->velocityY < 0.0) {
      move->accelerationY = (running) ? accelerationControllerY.calculate(
                                            move->accelerationY, -MARIO_JUMP_ACCELERATION / 1.9)
                                      : accelerationControllerY.calculate(
                                            move->accelerationY, -MARIO_JUMP_ACCELERATION / 2.5);
   } else {
      move->accelerationY = accelerationControllerY.calculate(move->accelerationY, 0);
   }
   currentState = JUMPING;
}

void PlayerSystem::updateWaterVelocity() {
   if (!mario->hasComponent<FrictionExemptComponent>()) {
      mario->addComponent<FrictionExemptComponent>();
   }
   if (!mario->hasComponent<BottomCollisionComponent>()) {
      currentState = SWIMMING;
   }
}

void PlayerSystem::updateCamera() {
   auto* position = mario->getComponent<PositionComponent>();
   auto* move = mario->getComponent<MovingComponent>();

   if (!Camera::Get().isFrozen()) {
      Camera::Get().updateCameraMin();
      if (position->position.x + 16 > Camera::Get().getCameraCenter() && move->velocityX > 0.0) {
         Camera::Get().increaseCameraX(move->velocityX);
      }
      if (position->position.x <= Camera::Get().getCameraMinX()) {
         position->position.x = Camera::Get().getCameraMinX();
      }
      if (Camera::Get().getCameraMaxX() >= scene->getLevelData().cameraMax * SCALED_CUBE_SIZE) {
         Camera::Get().setCameraMax(scene->getLevelData().cameraMax * SCALED_CUBE_SIZE);
         Camera::Get().setCameraMin(Camera::Get().getCameraX());
      }
   }
}

void PlayerSystem::tick(World* world) {
   auto* position = mario->getComponent<PositionComponent>();
   auto* move = mario->getComponent<MovingComponent>();

   if (FlagSystem::isClimbing()) {
      currentState = SLIDING;
      setState(currentState);
      updateCamera();
      return;
   }
   if (WarpSystem::isWarping()) {
      if (move->velocityX != 0) {
         currentState = WALKING;
      }
      if (move->velocityX == 0 || move->velocityY != 0) {
         currentState = STANDING;
      }
      setState(currentState);
      updateCamera();
      return;
   }
   if (!PlayerSystem::isInputEnabled()) {
      if (scene->getLevelData().levelType == LevelType::START_UNDERGROUND &&
          PlayerSystem::isGameStart()) {
         move->velocityX = 2.0;
      }

      if (move->velocityX != 0 && move->velocityY == 0) {
         currentState = WALKING;
      } else if (move->velocityY != 0) {  // If moving in the air
         currentState = JUMPING;
      } else {
         currentState = STANDING;
      }
      setState(currentState);
      updateCamera();
      return;
   }

   if (currentState != GAMEOVER) {  // If the player isn't dead
      if (underwater) {
         updateWaterVelocity();
      } else if (mario->hasComponent<BottomCollisionComponent>()) {
         updateGroundVelocity();
      } else {
         updateAirVelocity();
      }
   } else {
      // If game over
      currentState = GAMEOVER;
      return;
   }

   if (position->position.y >=
       Camera::Get().getCameraY() + SCREEN_HEIGHT + (1 * SCALED_CUBE_SIZE)) {
      onGameOver(true);
      return;
   }

   // Enemy collision
   world->find<EnemyComponent, PositionComponent>([&](Entity* enemy) {
      if (!AABBTotalCollision(enemy->getComponent<PositionComponent>(), position) ||
          enemy->hasAny<ParticleComponent, DeadComponent>() || currentState == GAMEOVER) {
         return;
      }
      auto* enemyMove = enemy->getComponent<MovingComponent>();
      auto* enemyPosition = enemy->getComponent<PositionComponent>();

      switch (enemy->getComponent<EnemyComponent>()->enemyType) {
         case EnemyType::KOOPA_SHELL:
            if (mario->hasComponent<SuperStarComponent>()) {
               enemy->getComponent<MovingComponent>()->velocityX = 0;

               enemy->addComponent<ParticleComponent>();
               enemy->addComponent<EnemyDestroyedComponent>();

               Entity* score(world->create());
               score->addComponent<AddScoreComponent>(100);

               createFloatingText(world, enemy, std::to_string(100));
               break;
            }

            if (move->velocityY > 0.0) {
               if (enemyMove->velocityX != 0) {
                  enemyMove->velocityX = 0;
                  move->velocityY = -ENEMY_BOUNCE;
               } else {
                  enemyMove->velocityX = 6.0;
               }
            } else if (position->getLeft() <= enemyPosition->getLeft() &&
                       position->getRight() < enemyPosition->getRight() &&
                       move->velocityY <= 0.0) {  // Hit from left side
               enemyMove->velocityX = 6.0;
            } else if (position->getLeft() > enemyPosition->getLeft() &&
                       position->getRight() > enemyPosition->getRight()) {
               enemyMove->velocityX = -6.0;
            }

            break;
         case EnemyType::KOOPA:
         case EnemyType::FLYING_KOOPA:
            if (mario->hasComponent<SuperStarComponent>()) {
               enemy->getComponent<MovingComponent>()->velocityX = 0;

               enemy->addComponent<DeadComponent>();
               enemy->addComponent<ParticleComponent>();
               enemy->addComponent<EnemyDestroyedComponent>();

               Entity* score(world->create());
               score->addComponent<AddScoreComponent>(100);

               createFloatingText(world, enemy, std::to_string(100));
               return;
            }
            if (move->velocityY > 0 && enemy->hasComponent<CrushableComponent>()) {
               enemy->addComponent<CrushedComponent>();
               enemy->getComponent<MovingComponent>()->velocityX = 0;
               move->velocityY = -MARIO_BOUNCE;

               Entity* score(world->create());
               score->addComponent<AddScoreComponent>(100);

               createFloatingText(world, enemy, std::to_string(100));
            } else if (move->velocityY <= 0 && !mario->hasComponent<FrozenComponent>() &&
                       !mario->hasComponent<EndingBlinkComponent>()) {
               onGameOver();
            }
            break;
         case EnemyType::GOOMBA:
            if (mario->hasComponent<SuperStarComponent>()) {
               enemy->getComponent<MovingComponent>()->velocityX = 0;

               enemy->addComponent<DeadComponent>();
               enemy->addComponent<ParticleComponent>();
               enemy->addComponent<EnemyDestroyedComponent>();

               Entity* score(world->create());
               score->addComponent<AddScoreComponent>(100);

               createFloatingText(world, enemy, std::to_string(100));
               return;
            }
            if (move->velocityY > 0 && enemy->hasComponent<CrushableComponent>()) {
               enemy->addComponent<CrushedComponent>();
               enemy->getComponent<MovingComponent>()->velocityX = 0;
               move->velocityY = -MARIO_BOUNCE;

               Entity* score(world->create());
               score->addComponent<AddScoreComponent>(100);

               createFloatingText(world, enemy, std::to_string(100));
            } else if (move->velocityY <= 0 && !mario->hasComponent<FrozenComponent>() &&
                       !mario->hasComponent<EndingBlinkComponent>()) {
               onGameOver();
            }
            break;
         case EnemyType::FIRE_BAR:
            if (!mario->hasComponent<SuperMarioComponent>()) {
               onGameOver();
            }
            break;
         default:
            break;
      }
   });

   // Projectile Collision
   world->find<ProjectileComponent, PositionComponent>([&](Entity* projectile) {
      auto* projectilePosition = projectile->getComponent<PositionComponent>();
      if (!AABBTotalCollision(position, projectilePosition) ||
          mario->hasAny<SuperStarComponent, EndingBlinkComponent, FrozenComponent>()) {
         return;
      }
      onGameOver();
   });

   // Break blocks
   world->find<BumpableComponent, PositionComponent, BottomCollisionComponent>([&](Entity*
                                                                                       breakable) {
      if (move->velocityY > 0) {
         return;
      }
      // Destroy the block if the player is Super Mario
      if (mario->hasComponent<SuperMarioComponent>()) {
         if (!breakable->hasComponent<MysteryBoxComponent>() &&
             breakable->hasComponent<DestructibleComponent>() &&
             AABBCollision(breakable->getComponent<PositionComponent>(), position)) {
            // This allows the enemy system to that the enemy should be destroyed, otherwise
            // the enemy will fall as normal
            breakable->addComponent<BlockBumpComponent>(std::vector<int>{0});
            breakable->addComponent<CallbackComponent>(
                [&](Entity* breakable) {
                   createBlockDebris(world, breakable);
                   world->destroy(breakable);
                },
                1);
            return;
         }
      }
      // If the player is in normal state, make the block bump
      if (!breakable->hasComponent<BlockBumpComponent>()) {
         breakable->addComponent<BlockBumpComponent>(std::vector<int>{-3, -3, -2, -1, 1, 2, 3, 3});
      }
      breakable->remove<BottomCollisionComponent>();

      if (breakable->hasComponent<MysteryBoxComponent>()) {
         auto mysteryBox = breakable->getComponent<MysteryBoxComponent>();

         breakable->getComponent<MysteryBoxComponent>()->whenDispensed(breakable);
         breakable->remove<AnimationComponent>();
         breakable->getComponent<TextureComponent>()->setSpritesheetCoordinates(
             mysteryBox->deactivatedCoordinates);
         breakable->remove<BumpableComponent>();
      }
   });

   // Collect Power-Ups
   world->find<CollectibleComponent>([&](Entity* collectible) {
      if (!AABBTotalCollision(collectible->getComponent<PositionComponent>(), position)) {
         return;
      }
      auto collect = collectible->getComponent<CollectibleComponent>();

      switch (collect->collectibleType) {
         case CollectibleType::MUSHROOM: {
            grow(world, Grow_Type::MUSHROOM);
            world->destroy(collectible);
         } break;
         case CollectibleType::SUPER_STAR:
            world->destroy(collectible);
            mario->addComponent<EndingBlinkComponent>(5, 600);
            mario->addComponent<SuperStarComponent>();
            mario->addComponent<CallbackComponent>(
                [](Entity* entity) {
                   entity->remove<SuperStarComponent>();
                },
                600);
            break;
         case CollectibleType::COIN: {
            Entity* coinScore(world->create());
            coinScore->addComponent<AddScoreComponent>(100, true);

            world->destroy(collectible);
         } break;
         default:
            break;
      }
   });

   // Updates the camera's position
   updateCamera();

   // Updates the textures for whichever state the player is currently in
   setState(currentState);

   // This resets the collision/jumping states to avoid conflicts during the next game tick
   //   jump = false;
   if (mario->hasComponent<TopCollisionComponent>()) {
      mario->remove<TopCollisionComponent>();
   }
   if (mario->hasComponent<BottomCollisionComponent>()) {
      mario->remove<BottomCollisionComponent>();
   }
   if (mario->hasComponent<LeftCollisionComponent>()) {
      mario->remove<LeftCollisionComponent>();
   }
   if (mario->hasComponent<RightCollisionComponent>()) {
      mario->remove<RightCollisionComponent>();
   }
}

void PlayerSystem::handleEvent(SDL_Event& event) {
   if (event.type != SDL_KEYDOWN && event.type != SDL_KEYUP) {
      return;
   }

   if (!PlayerSystem::isInputEnabled()) {
      left = right = running = duck = xDir = 0;
      return;
   }

   switch (event.type) {
      case SDL_KEYDOWN:
         switch (event.key.keysym.scancode) {
            case SDL_SCANCODE_A:
            case SDL_SCANCODE_LEFT:
               left = true;
               break;
            case SDL_SCANCODE_S:
            case SDL_SCANCODE_DOWN:
               duck = true;
               break;
            case SDL_SCANCODE_D:
            case SDL_SCANCODE_RIGHT:
               right = true;
               break;
            case SDL_SCANCODE_LSHIFT:
            case SDL_SCANCODE_LCTRL:
               running = true;
               break;
            default:
               break;
         }
         break;
      case SDL_KEYUP:
         switch (event.key.keysym.scancode) {
            case SDL_SCANCODE_A:
            case SDL_SCANCODE_LEFT:
               left = false;
               break;
            case SDL_SCANCODE_S:
            case SDL_SCANCODE_DOWN:
               duck = false;
               break;
            case SDL_SCANCODE_D:
            case SDL_SCANCODE_RIGHT:
               right = false;
               break;
            case SDL_SCANCODE_LSHIFT:
            case SDL_SCANCODE_LCTRL:
               running = false;
               break;
            default:
               break;
         }
         break;
   }
   xDir = right - left;
}

void PlayerSystem::handleEvent(const Uint8* keystates) {
   if (!PlayerSystem::isInputEnabled()) {
      jump = jumpHeld = false;
      return;
   }
   //   if(keystates[SDL_SCANCODE_W] || keystates[SDL_SCANCODE_UP]) {
   //   	jumpHeld = jump;
   //   	jump = true;
   //   } else {
   //   	jump = jumpHeld = false;
   //   }

   (keystates[SDL_SCANCODE_W] || keystates[SDL_SCANCODE_SPACE] || keystates[SDL_SCANCODE_UP])
       ? jump = true
       : jump = jumpHeld = false;
   //   (keystates[SDL_SCANCODE_A]) ? left = true : left = false;
   //   (keystates[SDL_SCANCODE_D]) ? right = true : right = false;
   //
   //   xDir = right - left;
}
