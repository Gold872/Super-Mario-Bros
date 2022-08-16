#include "systems/PlayerSystem.h"

#include "AABBCollision.h"
#include "Camera.h"
#include "Constants.h"
#include "ECS/Components.h"
#include "Level.h"
#include "Map.h"
#include "Math.h"
#include "SoundManager.h"
#include "TextureManager.h"
#include "command/CommandScheduler.h"
#include "command/Commands.h"
#include "systems/FlagSystem.h"
#include "systems/WarpSystem.h"

#include <SDL2/SDL.h>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <memory>

bool PlayerSystem::inputEnabled = true;
bool PlayerSystem::inGameStart = false;

int running = false;

bool jumpHeld = 0;

bool gameOver = false;

bool underwater = false;

PIDController underwaterControllerX(0.20, 0, 0.02, 60);

PlayerSystem::PlayerSystem(GameScene* scene) {
   this->scene = scene;
}

bool PlayerSystem::isSmallMario() {
   return mario->getComponent<PlayerComponent>()->playerState == PlayerState::SMALL_MARIO;
}

bool PlayerSystem::isSuperMario() {
   return mario->getComponent<PlayerComponent>()->playerState == PlayerState::SUPER_MARIO;
}

bool PlayerSystem::isFireMario() {
   return mario->getComponent<PlayerComponent>()->playerState == PlayerState::FIRE_MARIO;
}

bool PlayerSystem::isSuperStar() {
   return mario->getComponent<PlayerComponent>()->superStar;
}

Entity* PlayerSystem::createFireball(World* world) {
   holdFireballTexture = true;

   currentState = LAUNCH_FIREBALL;

   Entity* tempCallback(world->create());

   tempCallback->addComponent<CallbackComponent>(
       [&](Entity* entity) {
          holdFireballTexture = false;

          // For some reason world->destroy(entity) caused a bunch of weird stuff to happen
          entity->addComponent<DestroyDelayedComponent>(1);
       },
       6);

   Entity* fireball(world->create());

   auto* position = fireball->addComponent<PositionComponent>(
       Vector2f(), Vector2i(SCALED_CUBE_SIZE / 2, SCALED_CUBE_SIZE / 2), SDL_Rect{0, 0, 16, 16});

   auto playerTexture = mario->getComponent<TextureComponent>()->getTexture();

   fireball->addComponent<TextureComponent>(playerTexture, false, false);

   fireball->addComponent<SpritesheetComponent>(ORIGINAL_CUBE_SIZE / 2, ORIGINAL_CUBE_SIZE / 2, 1,
                                                9, 0, ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE,
                                                Map::PlayerIDCoordinates.at(246));

   auto* move = fireball->addComponent<MovingComponent>(Vector2f(0, 5), Vector2f(0, 0));

   fireball->addComponent<FrictionExemptComponent>();

   fireball->addComponent<GravityComponent>();

   fireball->addComponent<DestroyOutsideCameraComponent>();

   auto* marioTexture = mario->getComponent<TextureComponent>();

   if (marioTexture->isHorizontalFlipped()) {
      position->setRight(mario->getComponent<PositionComponent>()->getLeft());
      move->velocity.x = -PROJECTILE_SPEED;
   } else {
      position->setLeft(mario->getComponent<PositionComponent>()->getRight());
      move->velocity.x = PROJECTILE_SPEED;
   }

   position->setTop(mario->getComponent<PositionComponent>()->getTop() + 4);

   fireball->addComponent<WaitUntilComponent>(
       [=](Entity* entity) {
          return entity->hasAny<LeftCollisionComponent, RightCollisionComponent>() ||
                 !Camera::Get().inCameraRange(entity->getComponent<PositionComponent>());
       },
       [=](Entity* entity) {
          entity->remove<WaitUntilComponent>();
          if (entity->hasAny<LeftCollisionComponent, RightCollisionComponent>()) {
             entity->getComponent<SpritesheetComponent>()->setSpritesheetCoordinates(
                 Map::PlayerIDCoordinates.at(247));
             entity->addComponent<DestroyDelayedComponent>(4);
             entity->remove<MovingComponent, GravityComponent, FrictionExemptComponent>();

             Entity* fireballHitSound(world->create());
             fireballHitSound->addComponent<SoundComponent>(SoundID::BLOCK_HIT);
          } else {
             world->destroy(entity);
          }
       });

   fireball->addComponent<ProjectileComponent>(ProjectileType::FIREBALL);

   return fireball;
}

void PlayerSystem::setUnderwater(bool val) {
   underwater = val;
}

void PlayerSystem::onGameOver(World* world, bool outOfBounds) {
   auto* position = mario->getComponent<PositionComponent>();
   auto* spritesheet = mario->getComponent<SpritesheetComponent>();

   if (outOfBounds && isSuperMario()) {
      position->scale.y = position->hitbox.h = SCALED_CUBE_SIZE;
      spritesheet->setEntityHeight(ORIGINAL_CUBE_SIZE);
      spritesheet->setSpritesheetCoordinates(Map::PlayerIDCoordinates.at(1));

      mario->getComponent<PlayerComponent>()->playerState = PlayerState::SMALL_MARIO;
   }

   if (!outOfBounds && (isSuperMario() || isFireMario())) {
      shrink(world);
      return;
   }

   auto* move = mario->getComponent<MovingComponent>();

   move->velocity.x = move->acceleration.x = 0;
   move->acceleration.y = 0;
   move->velocity.y = -12.5f;

   mario->addComponent<ParticleComponent>();

   // Freezes every non-player entity
   world->find<MovingComponent>([](Entity* entity) {
      if (entity->hasComponent<PlayerComponent>()) {
         return;
      }

      entity->getComponent<MovingComponent>()->velocity = Vector2f(0, 0);
      entity->getComponent<MovingComponent>()->acceleration = Vector2f(0, 0);
   });

   world->find<AnimationComponent>([](Entity* entity) {
      if (entity->hasComponent<IconComponent>()) {
         return;
      }

      entity->getComponent<AnimationComponent>()->setPlaying(false);

      if (entity->hasComponent<PausedAnimationComponent>()) {
         entity->remove<PausedAnimationComponent>();
      }
   });

   spritesheet->setSpritesheetCoordinates(Map::PlayerIDCoordinates.at(1));

   currentState = GAMEOVER;

   mario->addComponent<CallbackComponent>(
       [=](Entity* entity) {
          entity->remove<ParticleComponent>();
          scene->restartLevel();
       },
       180);

   scene->stopTimer();
   scene->stopMusic();

   Entity* deathSound(world->create());
   deathSound->addComponent<SoundComponent>(SoundID::DEATH);
}

void PlayerSystem::setState(Animation_State newState) {
   auto* spritesheet = mario->getComponent<SpritesheetComponent>();
   auto* position = mario->getComponent<PositionComponent>();

   if (mario->hasComponent<FrozenComponent>()) {
      return;
   }

   switch (newState) {
      case STANDING:
         if (mario->hasComponent<AnimationComponent>()) {
            mario->remove<AnimationComponent>();
         }
         if (isFireMario()) {
            spritesheet->setSpritesheetCoordinates(Map::PlayerIDCoordinates.at(225));
         } else if (isSuperMario()) {
            spritesheet->setSpritesheetCoordinates(Map::PlayerIDCoordinates.at(25));
         } else {
            spritesheet->setSpritesheetCoordinates(Map::PlayerIDCoordinates.at(0));
         }

         break;
      case WALKING: {
         std::vector<int> fireFrameIDS = {227, 228, 229};
         std::vector<int> superFrameIDS = {27, 28, 29};
         std::vector<int> normalFrameIDS = {2, 3, 4};
         if (!mario->hasComponent<AnimationComponent>()) {
            if (isFireMario()) {
               mario->addComponent<AnimationComponent>(fireFrameIDS, 8, Map::PlayerIDCoordinates);
            } else if (isSuperMario()) {
               mario->addComponent<AnimationComponent>(superFrameIDS, 8, Map::PlayerIDCoordinates);
            } else {
               mario->addComponent<AnimationComponent>(normalFrameIDS, 8, Map::PlayerIDCoordinates);
            }
            return;
         }
         if (mario->getComponent<AnimationComponent>()->frameIDS != superFrameIDS &&
             mario->getComponent<AnimationComponent>()->frameIDS != normalFrameIDS &&
             mario->getComponent<AnimationComponent>()->frameIDS != fireFrameIDS) {
            // If the player already has an animation but it is not the correct one
            auto* animation = mario->getComponent<AnimationComponent>();

            if (isFireMario()) {
               animation->frameIDS = fireFrameIDS;
               animation->frameCount = 3;
            } else if (isSuperMario()) {
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

      } break;
      case SWIMMING: {
         std::vector<int> fireFrameIDS = {232, 233};
         std::vector<int> superFrameIDS = {32, 33};
         std::vector<int> normalFrameIDS = {7, 8};
         if (!mario->hasComponent<AnimationComponent>()) {
            if (isFireMario()) {
               mario->addComponent<AnimationComponent>(fireFrameIDS, 16, Map::PlayerIDCoordinates);
            } else if (isSuperMario()) {
               mario->addComponent<AnimationComponent>(superFrameIDS, 16, Map::PlayerIDCoordinates);
            } else {
               mario->addComponent<AnimationComponent>(normalFrameIDS, 16,
                                                       Map::PlayerIDCoordinates);
            }
         } else if (mario->getComponent<AnimationComponent>()->frameIDS != superFrameIDS &&
                    mario->getComponent<AnimationComponent>()->frameIDS != normalFrameIDS &&
                    mario->getComponent<AnimationComponent>()->frameIDS != fireFrameIDS) {
            // If the player already has an animation but it is not the correct one
            auto* animation = mario->getComponent<AnimationComponent>();
            if (isFireMario()) {
               animation->frameIDS = fireFrameIDS;
               animation->frameCount = 2;
               animation->setFramesPerSecond(16);
            } else if (isSuperMario()) {
               animation->frameIDS = superFrameIDS;
               animation->frameCount = 2;
               animation->setFramesPerSecond(16);
            } else {
               animation->frameIDS = normalFrameIDS;
               animation->frameCount = 2;
               animation->setFramesPerSecond(16);
            }
         }
      } break;
      case SWIMMING_JUMP: {
         std::vector<int> fireFrameIDS = {232, 233, 234, 235, 236, 237};
         std::vector<int> superFrameIDS = {32, 33, 34, 35, 36, 37};
         std::vector<int> normalFrameIDS = {7, 8, 9, 10, 11};
         if (!mario->hasComponent<AnimationComponent>()) {
            if (isFireMario()) {
               mario->addComponent<AnimationComponent>(fireFrameIDS, 16, Map::PlayerIDCoordinates,
                                                       false);
            } else if (isSuperMario()) {
               mario->addComponent<AnimationComponent>(superFrameIDS, 16, Map::PlayerIDCoordinates,
                                                       false);
            } else {
               mario->addComponent<AnimationComponent>(normalFrameIDS, 16, Map::PlayerIDCoordinates,
                                                       false);
            }
         } else if (mario->getComponent<AnimationComponent>()->frameIDS != superFrameIDS &&
                    mario->getComponent<AnimationComponent>()->frameIDS != normalFrameIDS &&
                    mario->getComponent<AnimationComponent>()->frameIDS != fireFrameIDS) {
            // If the player already has an animation but it is not the correct one
            auto* animation = mario->getComponent<AnimationComponent>();
            if (isFireMario()) {
               animation->frameIDS = fireFrameIDS;
               animation->frameCount = 6;
               animation->setFramesPerSecond(16);
               animation->repeated = false;
            } else if (isSuperMario()) {
               animation->frameIDS = superFrameIDS;
               animation->frameCount = 6;
               animation->setFramesPerSecond(16);
               animation->repeated = false;
            } else {
               animation->frameIDS = normalFrameIDS;
               animation->frameCount = 5;
               animation->setFramesPerSecond(16);
               animation->repeated = false;
            }
         }
      } break;
      case SWIMMING_WALK: {
         std::vector<int> fireFrameIDS = {227, 228, 229};
         std::vector<int> superFrameIDS = {27, 28, 29};
         std::vector<int> normalFrameIDS = {2, 3, 4};
         if (!mario->hasComponent<AnimationComponent>()) {
            if (isFireMario()) {
               mario->addComponent<AnimationComponent>(fireFrameIDS, 4, Map::PlayerIDCoordinates);
            } else if (isSuperMario()) {
               mario->addComponent<AnimationComponent>(superFrameIDS, 4, Map::PlayerIDCoordinates);
            } else {
               mario->addComponent<AnimationComponent>(normalFrameIDS, 4, Map::PlayerIDCoordinates);
            }
            return;
         }
         if (mario->getComponent<AnimationComponent>()->frameIDS != superFrameIDS &&
             mario->getComponent<AnimationComponent>()->frameIDS != normalFrameIDS &&
             mario->getComponent<AnimationComponent>()->frameIDS != fireFrameIDS) {
            // If the player already has an animation but it is not the correct one
            auto* animation = mario->getComponent<AnimationComponent>();

            if (isFireMario()) {
               animation->frameIDS = fireFrameIDS;
               animation->frameCount = 3;
               animation->setFramesPerSecond(4);
            } else if (isSuperMario()) {
               animation->frameIDS = superFrameIDS;
               animation->frameCount = 3;
               animation->setFramesPerSecond(4);
            } else {
               animation->frameIDS = normalFrameIDS;
               animation->frameCount = 3;
               animation->setFramesPerSecond(4);
            }
         }
      } break;
      case DRIFTING:
         if (mario->hasComponent<AnimationComponent>()) {
            mario->remove<AnimationComponent>();
         }
         if (isFireMario()) {
            spritesheet->setSpritesheetCoordinates(Map::PlayerIDCoordinates.at(230));
         } else if (isSuperMario()) {
            spritesheet->setSpritesheetCoordinates(Map::PlayerIDCoordinates.at(30));
         } else {
            spritesheet->setSpritesheetCoordinates(Map::PlayerIDCoordinates.at(5));
         }

         break;
      case JUMPING:
         if (mario->hasComponent<AnimationComponent>()) {
            mario->remove<AnimationComponent>();
         }
         if (isFireMario()) {
            spritesheet->setSpritesheetCoordinates(Map::PlayerIDCoordinates.at(231));
         } else if (isSuperMario()) {
            spritesheet->setSpritesheetCoordinates(Map::PlayerIDCoordinates.at(31));
         } else {
            spritesheet->setSpritesheetCoordinates(Map::PlayerIDCoordinates.at(6));
         }

         break;
      case DUCKING:
         if (mario->hasComponent<AnimationComponent>()) {
            mario->remove<AnimationComponent>();
         }
         if (isFireMario()) {
            spritesheet->setSpritesheetCoordinates(Map::PlayerIDCoordinates.at(226));
         } else {
            spritesheet->setSpritesheetCoordinates(Map::PlayerIDCoordinates.at(26));
         }
         break;
      case LAUNCH_FIREBALL: {
         if (mario->hasComponent<AnimationComponent>()) {
            mario->remove<AnimationComponent>();
         }
         if (mario->hasComponent<BottomCollisionComponent>()) {
            spritesheet->setSpritesheetCoordinates(Map::PlayerIDCoordinates.at(240));
         } else {
            spritesheet->setSpritesheetCoordinates(Map::PlayerIDCoordinates.at(243));
         }
      } break;
      case CLIMBING: {
         std::vector<int> fireFrameIDS = {238, 239};
         std::vector<int> superFrameIDS = {38, 39};
         std::vector<int> normalFrameIDS = {13, 14};
         if (!mario->hasComponent<AnimationComponent>()) {
            if (isFireMario()) {
               mario->addComponent<AnimationComponent>(fireFrameIDS, 8, Map::PlayerIDCoordinates);
            } else if (isSuperMario()) {
               mario->addComponent<AnimationComponent>(superFrameIDS, 8, Map::PlayerIDCoordinates);
            } else {
               mario->addComponent<AnimationComponent>(normalFrameIDS, 8, Map::PlayerIDCoordinates);
            }
         } else if (mario->getComponent<AnimationComponent>()->frameIDS != superFrameIDS &&
                    mario->getComponent<AnimationComponent>()->frameIDS != normalFrameIDS &&
                    mario->getComponent<AnimationComponent>()->frameIDS != fireFrameIDS) {
            // If the player already has an animation but it is not the correct one
            auto* animation = mario->getComponent<AnimationComponent>();
            if (isFireMario()) {
               animation->frameIDS = fireFrameIDS;
               animation->frameCount = 2;
               animation->setFramesPerSecond(8);
            } else if (isSuperMario()) {
               animation->frameIDS = superFrameIDS;
               animation->frameCount = 2;
               animation->setFramesPerSecond(8);
            } else {
               animation->frameIDS = normalFrameIDS;
               animation->frameCount = 2;
               animation->setFramesPerSecond(8);
            }
         }
      } break;
      case SLIDING:
         if (mario->hasComponent<AnimationComponent>()) {
            mario->remove<AnimationComponent>();
         }
         if (isFireMario()) {
            spritesheet->setSpritesheetCoordinates(Map::PlayerIDCoordinates.at(238));
         } else if (isSuperMario()) {
            spritesheet->setSpritesheetCoordinates(Map::PlayerIDCoordinates.at(38));
         } else {
            spritesheet->setSpritesheetCoordinates(Map::PlayerIDCoordinates.at(13));
         }
         break;
      case GAMEOVER:
         if (mario->hasComponent<AnimationComponent>()) {
            mario->remove<AnimationComponent>();
         }
         spritesheet->setSpritesheetCoordinates(Map::PlayerIDCoordinates.at(1));
         break;
      default:
         break;
   }

   if (newState == DUCKING) {
      position->hitbox.h = 32;
      position->hitbox.y = 32;
   } else {
      position->hitbox.h = position->scale.y;
      position->hitbox.y = 0;
   }
}

void PlayerSystem::grow(World* world, GrowType growType) {
   switch (growType) {
      case GrowType::ONEUP: {
         Entity* addLives(world->create());
         addLives->addComponent<AddLivesComponent>();

         Entity* floatingText(world->create());
         floatingText->addComponent<CreateFloatingTextComponent>(mario, "1-UP");

      } break;
      case GrowType::SUPER_STAR: {
         mario->getComponent<PlayerComponent>()->superStar = true;

         mario->addComponent<EndingBlinkComponent>(5, 600);
         mario->addComponent<CallbackComponent>(
             [=](Entity* entity) {
                entity->getComponent<PlayerComponent>()->superStar = false;
                scene->resumeLastPlayedMusic();
             },
             600);

         Entity* superStarMusic(world->create());
         superStarMusic->addComponent<MusicComponent>(MusicID::SUPER_STAR);
      } break;
      case GrowType::MUSHROOM: {
         Entity* addScore(world->create());
         addScore->addComponent<AddScoreComponent>(1000);

         Entity* floatingText(world->create());
         floatingText->addComponent<CreateFloatingTextComponent>(mario, std::to_string(1000));

         Entity* powerUpSound(world->create());
         powerUpSound->addComponent<SoundComponent>(SoundID::POWER_UP_COLLECT);

         if (isSuperMario() || isFireMario()) {
            return;
         }

         auto* position = mario->getComponent<PositionComponent>();
         auto* spritesheet = mario->getComponent<SpritesheetComponent>();

         position->setTop(position->getTop() - position->scale.y);  // Makes the player taller

         position->scale.y = position->hitbox.h = SCALED_CUBE_SIZE * 2;

         spritesheet->setEntityHeight(ORIGINAL_CUBE_SIZE * 2);

         mario->addComponent<AnimationComponent>(
             std::vector<int>{46, 45, 25, 46, 45, 25, 46, 45, 25}, 12, Map::PlayerIDCoordinates,
             false);

         mario->addComponent<FrozenComponent>();

         mario->addComponent<CallbackComponent>(
             [&](Entity* mario) {
                mario->getComponent<PlayerComponent>()->playerState = PlayerState::SUPER_MARIO;
                mario->remove<FrozenComponent>();
             },
             45);
      } break;
      case GrowType::FIRE_FLOWER: {
         if (!isSuperMario()) {
            grow(world, GrowType::MUSHROOM);
            return;
         }

         Entity* addScore(world->create());
         addScore->addComponent<AddScoreComponent>(1000);

         Entity* floatingText(world->create());
         floatingText->addComponent<CreateFloatingTextComponent>(mario, std::to_string(1000));

         Entity* powerUpSound(world->create());
         powerUpSound->addComponent<SoundComponent>(SoundID::POWER_UP_COLLECT);

         if (isFireMario()) {
            return;
         }

         mario->addComponent<AnimationComponent>(
             std::vector<int>{350, 351, 352, 353, 350, 351, 352, 353, 350, 351, 352, 353}, 12,
             Map::PlayerIDCoordinates, false);

         mario->addComponent<FrozenComponent>();

         mario->addComponent<CallbackComponent>(
             [&](Entity* mario) {
                mario->getComponent<PlayerComponent>()->playerState = PlayerState::FIRE_MARIO;
                mario->remove<FrozenComponent>();
             },
             60);
      } break;
      default:
         break;
   }
}

void PlayerSystem::shrink(World* world) {
   mario->getComponent<PlayerComponent>()->playerState = PlayerState::SMALL_MARIO;

   Entity* shrinkSound(world->create());
   shrinkSound->addComponent<SoundComponent>(SoundID::PIPE);

   mario->addComponent<AnimationComponent>(std::vector<int>{25, 45, 46, 25, 45, 46, 25, 45, 46}, 12,
                                           Map::PlayerIDCoordinates, false);

   mario->addComponent<FrozenComponent>();

   mario->addComponent<CallbackComponent>(
       [&](Entity* mario) {
          auto* position = mario->getComponent<PositionComponent>();
          auto* spritesheet = mario->getComponent<SpritesheetComponent>();

          // Shortens the player
          position->scale.y = position->hitbox.h = SCALED_CUBE_SIZE;
          spritesheet->setEntityHeight(ORIGINAL_CUBE_SIZE);
          spritesheet->setSpritesheetCoordinates(Map::PlayerIDCoordinates.at(0));
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

   debris1->addComponent<MovingComponent>(Vector2f(-8.0f, -2.0f), Vector2f(0, 0));

   debris1->addComponent<TextureComponent>(debrisTexture, true);

   debris1->addComponent<SpritesheetComponent>(
       ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE, 1, 1, 1, ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE,
       block->getComponent<DestructibleComponent>()->debrisCoordinates);

   debris1->addComponent<ParticleComponent>();

   debris1->addComponent<DestroyOutsideCameraComponent>();
   /* ******************************************************************* */
   Entity* debris2(world->create());  // Top Right

   debris2->addComponent<PositionComponent>(
       Vector2f(blockPosition->getLeft(), blockPosition->getTop() - SCALED_CUBE_SIZE),
       Vector2i(SCALED_CUBE_SIZE, SCALED_CUBE_SIZE));

   debris2->addComponent<GravityComponent>();

   debris2->addComponent<MovingComponent>(Vector2f(8.0f, -2.0f), Vector2f(0, 0));

   debris2->addComponent<TextureComponent>(debrisTexture);

   debris2->addComponent<SpritesheetComponent>(
       ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE, 1, 1, 1, ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE,
       block->getComponent<DestructibleComponent>()->debrisCoordinates);

   debris2->addComponent<ParticleComponent>();

   debris2->addComponent<DestroyOutsideCameraComponent>();
   /* ******************************************************************* */
   Entity* debris3(world->create());  // Bottom Left

   debris3->addComponent<PositionComponent>(
       Vector2f(blockPosition->getLeft(), blockPosition->getTop()),
       Vector2i(SCALED_CUBE_SIZE, SCALED_CUBE_SIZE));

   debris3->addComponent<GravityComponent>();

   debris3->addComponent<MovingComponent>(Vector2f(-8.0f, -2.0f), Vector2f(0, 0));

   debris3->addComponent<TextureComponent>(debrisTexture, true);

   debris3->addComponent<SpritesheetComponent>(
       ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE, 1, 1, 1, ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE,
       block->getComponent<DestructibleComponent>()->debrisCoordinates);

   debris3->addComponent<ParticleComponent>();

   debris3->addComponent<DestroyOutsideCameraComponent>();
   /* ******************************************************************* */
   Entity* debris4(world->create());  // Bottom Right

   debris4->addComponent<PositionComponent>(
       Vector2f(blockPosition->getLeft(), blockPosition->getTop()),
       Vector2i(SCALED_CUBE_SIZE, SCALED_CUBE_SIZE));

   debris4->addComponent<GravityComponent>();

   debris4->addComponent<MovingComponent>(Vector2f(8.0f, -2.0f), Vector2f(0, 0));

   debris4->addComponent<TextureComponent>(debrisTexture);

   debris4->addComponent<SpritesheetComponent>(
       ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE, 1, 1, 1, ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE,
       block->getComponent<DestructibleComponent>()->debrisCoordinates);

   debris4->addComponent<ParticleComponent>();

   debris4->addComponent<DestroyOutsideCameraComponent>();
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

   mario->addComponent<TextureComponent>(playerTexture, false, false);

   mario->addComponent<SpritesheetComponent>(ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE, 1, 9, 0,
                                             ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE,
                                             Map::PlayerIDCoordinates.at(0));

   mario->getComponent<TextureComponent>()->setVisible(false);

   mario->addComponent<MovingComponent>(Vector2f(0, 0), Vector2f(0, 0));

   mario->addComponent<PlayerComponent>();

   mario->addComponent<FrozenComponent>();
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

   Camera::Get().setCameraX(scene->getLevelData().cameraStart.x * SCALED_CUBE_SIZE);
   Camera::Get().setCameraY(scene->getLevelData().cameraStart.y * SCALED_CUBE_SIZE);

   move->velocity.y = move->acceleration.y = 0;
   move->velocity.x = move->acceleration.x = 0;

   mario->getComponent<TextureComponent>()->setVisible(true);
   mario->getComponent<TextureComponent>()->setHorizontalFlipped(false);

   mario->remove<FrozenComponent>();

   if (scene->getLevelData().levelType != LevelType::START_UNDERGROUND) {
      mario->addComponent<GravityComponent>();

      mario->remove<FrictionExemptComponent>();
      mario->remove<CollisionExemptComponent>();

      Camera::Get().setCameraFrozen(false);

      PlayerSystem::enableInput(true);
   } else {
      Camera::Get().setCameraFrozen(true);

      PlayerSystem::setGameStart(true);
      PlayerSystem::enableInput(false);

      mario->addComponent<FrictionExemptComponent>();
      mario->addComponent<CollisionExemptComponent>();

      mario->remove<GravityComponent>();

      move->velocity.x = 1.6;
   }

   currentState = STANDING;
}

void PlayerSystem::checkTrampolineCollisions(World* world) {
   auto* position = mario->getComponent<PositionComponent>();
   auto* move = mario->getComponent<MovingComponent>();

   world->find<TrampolineComponent>([&](Entity* entity) {
      auto* trampoline = entity->getComponent<TrampolineComponent>();
      auto* trampolinePosition = entity->getComponent<PositionComponent>();
      auto* trampolineTexture = entity->getComponent<SpritesheetComponent>();

      Entity* bottomEntity = trampoline->bottomEntity;
      auto* bottomTexture = bottomEntity->getComponent<SpritesheetComponent>();

      if (!AABBCollision(position, trampolinePosition) ||
          !Camera::Get().inCameraRange(trampolinePosition)) {
         trampoline->currentSequenceIndex = 0;

         trampolineCollided = false;
         return;
      }

      if (trampoline->currentSequenceIndex > 20) {
         return;
      }

      trampolineCollided = true;

      entity->remove<TileComponent>();

      switch (trampoline->currentSequenceIndex) {
         case 0:  // Currently extended, set to half retracted
            trampolineTexture->setSpritesheetCoordinates(
                Map::BlockIDCoordinates.at(trampoline->topMediumRetractedID));

            bottomTexture->setSpritesheetCoordinates(
                Map::BlockIDCoordinates.at(trampoline->bottomMediumRetractedID));

            trampolinePosition->hitbox = SDL_Rect{0, 16, SCALED_CUBE_SIZE, 16};
            break;
         case 1:  // Currently half retracted, set to retracted
            trampolineTexture->setSpritesheetCoordinates(
                Map::BlockIDCoordinates.at(trampoline->topRetractedID));

            bottomTexture->setSpritesheetCoordinates(
                Map::BlockIDCoordinates.at(trampoline->bottomRetractedID));

            trampolinePosition->hitbox = SDL_Rect{0, 32, SCALED_CUBE_SIZE, 0};
            break;
         case 2:  // Currently retracted, set to half retracted and launch the player
            trampolineTexture->setSpritesheetCoordinates(
                Map::BlockIDCoordinates.at(trampoline->topMediumRetractedID));

            bottomTexture->setSpritesheetCoordinates(
                Map::BlockIDCoordinates.at(trampoline->bottomMediumRetractedID));

            trampolinePosition->hitbox = SDL_Rect{0, 16, SCALED_CUBE_SIZE, 16};

            move->velocity.y = -11.0;
            break;
         case 3:  // Currently half retracted, set to extended
            trampolineTexture->setSpritesheetCoordinates(
                Map::BlockIDCoordinates.at(trampoline->topExtendedID));

            bottomTexture->setSpritesheetCoordinates(
                Map::BlockIDCoordinates.at(trampoline->bottomExtendedID));

            trampolinePosition->hitbox = SDL_Rect{0, 0, SCALED_CUBE_SIZE, SCALED_CUBE_SIZE};
            break;
         default:
            break;
      }

      trampoline->currentSequenceIndex++;

      if (position->getCenterY() > trampolinePosition->getBottom()) {
         position->setCenterY(trampolinePosition->getBottom());
      }
   });
}

// Updates the velocity and acceleration for when mario is on the ground
void PlayerSystem::updateGroundVelocity(World* world) {
   auto* texture = mario->getComponent<TextureComponent>();
   auto* move = mario->getComponent<MovingComponent>();

   // The textures only get flipped if the player is on the ground
   if (left) {
      texture->setHorizontalFlipped(true);
   } else if (right) {
      texture->setHorizontalFlipped(false);
   }
   // Updates the acceleration
   move->acceleration.x = (float)xDir * MARIO_ACCELERATION_X;
   //   if (mario->hasComponent<SuperStarComponent>()) {
   //      move->acceleration.x *= 1.5957446808510638297f;
   //   } else
   if (running) {
      // a weird number that will max the velocity at 5
      move->acceleration.x *= 1.3297872340425531914f;
   } else {
      // a weird number that will max the velocity to 3
      move->acceleration.x *= 0.7978723404255319148936f;
   }

   if (jump && !jumpHeld && !trampolineCollided) {
      jumpHeld = true;
      move->velocity.y = -7.3;

      Entity* jumpSound(world->create());
      jumpSound->addComponent<SoundComponent>(SoundID::JUMP);
   }
   if (duck && isSuperMario()) {
      currentState = DUCKING;
      move->acceleration.x = 0;
      // Slows the player down
      if (move->velocity.x > 1.5) {
         move->velocity.x -= 0.5;
      } else if (move->velocity.x < -1.5) {
         move->velocity.x += 0.5;
      }
   } else if ((bool)std::abs(move->velocity.x) || (bool)std::abs(move->acceleration.x)) {
      // If the player should be drifting
      if ((move->velocity.x > 0 && move->acceleration.x < 0) ||
          (move->velocity.x < 0 && move->acceleration.x > 0)) {
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

   move->acceleration.x = (float)xDir * MARIO_ACCELERATION_X;
   if (running) {
      if ((move->acceleration.x >= 0 && move->velocity.x >= 0) ||
          (move->acceleration.x <= 0 && move->velocity.x <= 0)) {
         // If the acceleration and velocity aren't in opposite directions
         //         if (mario->hasComponent<SuperStarComponent>()) {
         //            move->acceleration.x *= 1.5957446808510638297f;
         //         } else {
         //            move->acceleration.x *= 1.3297872340425531914f;
         //         }
         move->acceleration.x *= 1.3297872340425531914f;
      } else {
         move->acceleration.x *= 0.35f;
      }
   }
   // Changes mario's acceleration while in the air (the longer you jump the higher mario
   // will go)
   if (jump && move->velocity.y < -1.0) {
      if (running && std::abs(move->velocity.x) > 3.5) {
         move->acceleration.y = -0.414;
      } else {
         move->acceleration.y = -0.412;
      }
   } else {
      move->acceleration.y = 0;
   }
   if (duck && isSuperMario()) {
      currentState = DUCKING;
   } else {
      currentState = JUMPING;
   }
}

void PlayerSystem::updateWaterVelocity(World* world) {
   auto* move = mario->getComponent<MovingComponent>();
   auto* texture = mario->getComponent<TextureComponent>();

   if (!mario->hasComponent<FrictionExemptComponent>()) {
      mario->addComponent<FrictionExemptComponent>();
   }
   if (!mario->hasComponent<BottomCollisionComponent>() &&
       !mario->hasComponent<WaitUntilComponent>()) {
      currentState = SWIMMING;
   } else if (mario->hasComponent<BottomCollisionComponent>()) {
      if (move->velocity.x != 0) {
         currentState = SWIMMING_WALK;
      } else {
         currentState = STANDING;
      }
   }

   if (currentState == SWIMMING || currentState == SWIMMING_JUMP) {
      if (left) {
         move->velocity.x += underwaterControllerX.calculate(move->velocity.x, -3.0);
         texture->setHorizontalFlipped(true);
      } else if (right) {
         move->velocity.x += underwaterControllerX.calculate(move->velocity.x, 3.0);
         texture->setHorizontalFlipped(false);
      } else {
         move->velocity.x += underwaterControllerX.calculate(move->velocity.x, 0.0);
      }
   } else {
      if (left) {
         move->velocity.x += underwaterControllerX.calculate(move->velocity.x, -1.0);
         texture->setHorizontalFlipped(true);
      } else if (right) {
         move->velocity.x += underwaterControllerX.calculate(move->velocity.x, 1.0);
         texture->setHorizontalFlipped(false);
      } else {
         move->velocity.x += underwaterControllerX.calculate(move->velocity.x, 0.0);
      }
   }

   move->acceleration.y = -0.45480f;

   if (move->velocity.y > MAX_UNDERWATER_Y) {
      move->velocity.y = MAX_UNDERWATER_Y;
   }

   if (jump && !jumpHeld) {
      move->velocity.y = -3.53;
      jumpHeld = true;

      Entity* jumpSound(world->create());
      jumpSound->addComponent<SoundComponent>(SoundID::STOMP);

      currentState = SWIMMING_JUMP;

      mario->addComponent<WaitUntilComponent>(
          [](Entity* entity) {
             return !entity->hasComponent<AnimationComponent>();
          },
          [&](Entity* entity) {
             currentState = SWIMMING;
             entity->remove<WaitUntilComponent>();
          });
   }
}

void PlayerSystem::updateCamera() {
   auto* position = mario->getComponent<PositionComponent>();
   auto* move = mario->getComponent<MovingComponent>();

   if (!Camera::Get().isFrozen()) {
      if (position->position.x + 16 > Camera::Get().getCameraCenterX() && move->velocity.x > 0.0) {
         Camera::Get().increaseCameraX(move->velocity.x);
      }
      if (position->position.x <= Camera::Get().getCameraLeft()) {
         position->position.x = Camera::Get().getCameraLeft();
      }
      if (position->getRight() >= Camera::Get().getCameraMaxX()) {
         position->setRight(Camera::Get().getCameraMaxX());
      }
      if (Camera::Get().getCameraRight() >= Camera::Get().getCameraMaxX()) {
         Camera::Get().setCameraRight(Camera::Get().getCameraMaxX());
      }

      Camera::Get().updateCameraMin();
   }
}

void PlayerSystem::checkEnemyCollisions(World* world) {
   bool enemyCrushed = false;

   auto* position = mario->getComponent<PositionComponent>();
   auto* move = mario->getComponent<MovingComponent>();

   world->find<EnemyComponent, PositionComponent>([&](Entity* enemy) {
      if (!AABBTotalCollision(enemy->getComponent<PositionComponent>(), position) ||
          mario->hasComponent<FrozenComponent>() || enemy->hasComponent<DeadComponent>() ||
          currentState == GAMEOVER) {
         return;
      }
      auto* enemyMove = enemy->getComponent<MovingComponent>();
      auto* enemyPosition = enemy->getComponent<PositionComponent>();

      switch (enemy->getComponent<EnemyComponent>()->enemyType) {
         case EnemyType::KOOPA_SHELL:
            if (isSuperStar()) {
               enemy->getComponent<MovingComponent>()->velocity.x = 0;

               enemy->addComponent<EnemyDestroyedComponent>();

               Entity* score(world->create());
               score->addComponent<AddScoreComponent>(100);
               break;
            }

            if (move->velocity.y > 0.0) {
               if (enemyMove->velocity.x != 0) {
                  enemyMove->velocity.x = 0;
                  move->velocity.y = -ENEMY_BOUNCE;

                  enemyCrushed = true;
               } else {
                  enemyMove->velocity.x = 6.0;
               }
            } else if (position->getLeft() <= enemyPosition->getLeft() &&
                       position->getRight() < enemyPosition->getRight() &&
                       move->velocity.y <= 0.0) {  // Hit from left side
               enemyMove->velocity.x = 6.0;
            } else if (position->getLeft() > enemyPosition->getLeft() &&
                       position->getRight() > enemyPosition->getRight()) {
               enemyMove->velocity.x = -6.0;
            }

            break;
         case EnemyType::FIRE_BAR:
            if (!isSuperStar() && !mario->hasComponent<EndingBlinkComponent>()) {
               onGameOver(world);
            }
            break;
         default:
            if (isSuperStar()) {
               enemy->getComponent<MovingComponent>()->velocity.x = 0;

               enemy->addComponent<EnemyDestroyedComponent>();

               Entity* score(world->create());
               score->addComponent<AddScoreComponent>(100);
               return;
            }
            if (move->velocity.y > 0 && enemy->hasComponent<CrushableComponent>()) {
               enemy->addComponent<CrushedComponent>();
               enemy->getComponent<MovingComponent>()->velocity.x = 0;
               move->velocity.y = -MARIO_BOUNCE;

               enemyCrushed = true;

               Entity* score(world->create());
               score->addComponent<AddScoreComponent>(100);

            } else if (!enemyCrushed && move->velocity.y <= 0 &&
                       !mario->hasAny<FrozenComponent, EndingBlinkComponent>()) {
               onGameOver(world);
            } else if (enemyCrushed) {
               enemy->addComponent<CrushedComponent>();
               enemy->getComponent<MovingComponent>()->velocity.x = 0;
               move->velocity.y = -MARIO_BOUNCE;

               Entity* score(world->create());
               score->addComponent<AddScoreComponent>(100);
            }
            break;
      }
   });
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
      if (move->velocity.x != 0) {
         currentState = WALKING;
      }
      if (move->velocity.x == 0 || move->velocity.y != 0) {
         currentState = STANDING;
      }
      setState(currentState);
      updateCamera();
      return;
   }
   if (WarpSystem::isClimbing()) {
      currentState = (move->velocity.y != 0) ? CLIMBING : SLIDING;
      setState(currentState);
      updateCamera();
      return;
   }
   if (!PlayerSystem::isInputEnabled()) {
      if (scene->getLevelData().levelType == LevelType::START_UNDERGROUND &&
          PlayerSystem::isGameStart()) {
         move->velocity.x = 1.6;
      }

      if (move->velocity.x != 0 && move->velocity.y == 0) {
         currentState = WALKING;
      } else if (move->velocity.y != 0) {  // If moving in the air
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
         updateWaterVelocity(world);
      } else if (mario->hasComponent<BottomCollisionComponent>()) {
         updateGroundVelocity(world);
      } else {
         updateAirVelocity();
      }
   } else {
      // If game over
      currentState = GAMEOVER;
      return;
   }

   // Hold the launching texture
   if (holdFireballTexture) {
      currentState = LAUNCH_FIREBALL;
   }

   if (position->position.y >=
           Camera::Get().getCameraY() + SCREEN_HEIGHT + (1 * SCALED_CUBE_SIZE) &&
       !mario->hasComponent<FrozenComponent>() && !WarpSystem::hasClimbed()) {
      onGameOver(world, true);
      return;
   }

   bool platformMoved = false;

   // Move mario with the platforms
   world->find<MovingPlatformComponent, MovingComponent>([&](Entity* entity) {
      if (!AABBCollision(position, entity->getComponent<PositionComponent>()) || platformMoved) {
         return;
      }
      auto* platformMove = entity->getComponent<MovingComponent>();
      position->position.x += platformMove->velocity.x;
      position->position.y += platformMove->velocity.y;

      if (position->position.x + 16 > Camera::Get().getCameraCenterX() &&
          platformMove->velocity.x > 0) {
         Camera::Get().increaseCameraX(platformMove->velocity.x);
      }

      platformMoved = true;
   });

   checkTrampolineCollisions(world);

   // Launch fireballs
   if (isFireMario() && launchFireball) {
      createFireball(world);
      launchFireball = false;

      Entity* fireballSound(world->create());
      fireballSound->addComponent<SoundComponent>(SoundID::FIREBALL);
   }

   // Enemy collision
   checkEnemyCollisions(world);

   // Projectile Collision
   world->find<ProjectileComponent, PositionComponent>([&](Entity* projectile) {
      auto* projectilePosition = projectile->getComponent<PositionComponent>();
      if (!Camera::Get().inCameraRange(projectilePosition)) {
         return;
      }
      if (!AABBTotalCollision(position, projectilePosition) || isSuperStar() ||
          mario->hasAny<EndingBlinkComponent, FrozenComponent>()) {
         return;
      }
      if (projectile->getComponent<ProjectileComponent>()->projectileType !=
          ProjectileType::FIREBALL) {
         onGameOver(world);
      }
   });

   // Break blocks
   world->find<BumpableComponent, PositionComponent, BottomCollisionComponent>([&](Entity*
                                                                                       breakable) {
      if (move->velocity.y > 0) {
         return;
      }
      // Destroy the block if the player is Super Mario
      if (!isSmallMario()) {
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

                   Entity* breakSound(world->create());
                   breakSound->addComponent<SoundComponent>(SoundID::BLOCK_BREAK);
                },
                1);
            return;
         }
      }
      // If the player is in normal state, make the block bump
      if (!breakable->hasComponent<BlockBumpComponent>()) {
         breakable->addComponent<BlockBumpComponent>(std::vector<int>{-3, -3, -2, -1, 1, 2, 3, 3});

         Entity* bumpSound(world->create());
         bumpSound->addComponent<SoundComponent>(SoundID::BLOCK_HIT);
      }
      breakable->remove<BottomCollisionComponent>();

      if (breakable->hasComponent<MysteryBoxComponent>()) {
         auto mysteryBox = breakable->getComponent<MysteryBoxComponent>();

         if (breakable->hasComponent<InvisibleBlockComponent>()) {
            breakable->remove<InvisibleBlockComponent>();
            move->velocity.y = move->acceleration.y = 0;
         }

         breakable->getComponent<MysteryBoxComponent>()->whenDispensed(breakable);
         breakable->remove<AnimationComponent>();
         breakable->getComponent<SpritesheetComponent>()->setSpritesheetCoordinates(
             mysteryBox->deactivatedCoordinates);
         breakable->remove<BumpableComponent>();
      }
   });

   // Collect Power-Ups
   world->find<CollectibleComponent, PositionComponent>([&](Entity* collectible) {
      if (!Camera::Get().inCameraRange(collectible->getComponent<PositionComponent>())) {
         return;
      }
      if (!AABBTotalCollision(collectible->getComponent<PositionComponent>(), position)) {
         return;
      }
      auto collect = collectible->getComponent<CollectibleComponent>();

      switch (collect->collectibleType) {
         case CollectibleType::MUSHROOM: {
            grow(world, GrowType::MUSHROOM);

            world->destroy(collectible);
         } break;
         case CollectibleType::SUPER_STAR:
            grow(world, GrowType::SUPER_STAR);

            world->destroy(collectible);
            break;
         case CollectibleType::FIRE_FLOWER:
            grow(world, GrowType::FIRE_FLOWER);

            world->destroy(collectible);
            break;
         case CollectibleType::COIN: {
            Entity* coinScore(world->create());
            coinScore->addComponent<AddScoreComponent>(100, true);

            Entity* coinSound(world->create());
            coinSound->addComponent<SoundComponent>(SoundID::COIN);

            world->destroy(collectible);
         } break;
         case CollectibleType::ONE_UP: {
            grow(world, GrowType::ONEUP);

            Entity* oneUpSound(world->create());
            oneUpSound->addComponent<SoundComponent>(SoundID::ONE_UP);

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
            case SDL_SCANCODE_Q:
               if (event.key.repeat == 0) {
                  launchFireball = true;
               }
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
            case SDL_SCANCODE_Q:
               launchFireball = false;
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
   if (keystates[SDL_SCANCODE_W] || keystates[SDL_SCANCODE_SPACE] || keystates[SDL_SCANCODE_UP]) {
      jumpHeld = jump;
      jump = true;
   } else {
      jump = jumpHeld = false;
   }

   //   (keystates[SDL_SCANCODE_W] || keystates[SDL_SCANCODE_SPACE] || keystates[SDL_SCANCODE_UP])
   //       ? jump = true
   //       : jump = jumpHeld = false;

   //   if (keystates[SDL_SCANCODE_W] || keystates[SDL_SCANCODE_SPACE] ||
   //   keystates[SDL_SCANCODE_UP]) {
   //      jump = true;
   //      jumpHeldTime++;
   //   } else {
   //      jump = jumpHeld = false;
   //      jumpHeldTime = 0;
   //   }
}
