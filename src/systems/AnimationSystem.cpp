#include "systems/AnimationSystem.h"

#include "Camera.h"
#include "ECS/Components.h"
#include "ECS/ECS.h"
#include "Map.h"

void AnimationSystem::onAddedToWorld(World* world) {
   System::onAddedToWorld(world);
}

void AnimationSystem::tick(World* world) {
   // Deals with Blinking
   world->find<EndingBlinkComponent, TextureComponent>([&](Entity* entity) {
      auto blink = entity->getComponent<EndingBlinkComponent>();
      blink->current++;
      blink->time--;
      if ((blink->current / blink->blinkSpeed) % 2 == 1) {
         entity->getComponent<TextureComponent>()->setVisible(false);
      } else {
         entity->getComponent<TextureComponent>()->setVisible(true);
      }
      if (blink->time == 0) {
         entity->remove<EndingBlinkComponent>();
         entity->getComponent<TextureComponent>()->setVisible(true);
      }
   });

   // Non-Paused animations
   world->find<AnimationComponent, TextureComponent>([&](Entity* entity) {
      if (!entity->hasComponent<PositionComponent>()) {
         return;
      }
      if (!Camera::inCameraRange(entity->getComponent<PositionComponent>()) &&
          !entity->hasComponent<IconComponent>()) {
         return;
      }
      if (!entity->hasComponent<PausedAnimationComponent>()) {
         auto* animation = entity->getComponent<AnimationComponent>();
         auto* texture = entity->getComponent<TextureComponent>();

         animation->frameTimer--;

         if (animation->frameTimer <= 0 && animation->playing) {
            animation->frameTimer = animation->frameDelay;

            animation->currentFrame++;
            if (animation->currentFrame == animation->frameCount) {
               if (animation->repeated) {
                  animation->currentFrame = 0;
               } else {
                  //               	animation->currentFrame = 0;
                  //
                  //                  int animationFrameID =
                  //                  animation->frameIDS[animation->currentFrame];
                  //
                  //                  Vector2i frameCoordinates;
                  //
                  //                  if (entity->hasComponent<ForegroundComponent>()) {
                  //                     frameCoordinates =
                  //                     Map::BlockIDCoordinates.at(animationFrameID);
                  //                  } else if (entity->hasComponent<PlayerComponent>()) {
                  //                     frameCoordinates =
                  //                     Map::PlayerIDCoordinates.at(animationFrameID);
                  //                  } else if (entity->hasComponent<EnemyComponent>()) {
                  //                     frameCoordinates =
                  //                     Map::EnemyIDCoordinates.at(animationFrameID);
                  //                  }
                  //
                  //                  // Sets the texture sprite sheets coordinates to the animation
                  //                  frame coordinates
                  //                  texture->setSpritesheetCoordinates(frameCoordinates);
                  entity->remove<AnimationComponent>();
                  return;
               }
            }

            int animationFrameID = animation->frameIDS[animation->currentFrame];

            Vector2i frameCoordinates;

            frameCoordinates = animation->coordinateSupplier.at(animationFrameID);

            // Sets the texture sprite sheets coordinates to the animation frame coordinates
            texture->setSpritesheetCoordinates(frameCoordinates);
         }
      }
   });

   world->find<AnimationComponent, PausedAnimationComponent, TextureComponent>([&](Entity* entity) {
      if (!entity->hasComponent<PositionComponent>()) {
         return;
      }
      if (!Camera::inCameraRange(entity->getComponent<PositionComponent>()) &&
          !entity->hasComponent<IconComponent>()) {
         return;
      }
      auto animation = entity->getComponent<AnimationComponent>();
      auto texture = entity->getComponent<TextureComponent>();
      auto pause = entity->getComponent<PausedAnimationComponent>();

      // If it is playing then it increases the frame, and it also checks if it should pause
      if (animation->playing) {
         animation->frameTimer--;

         if (animation->frameTimer <= 0) {
            animation->frameTimer = animation->frameDelay;

            animation->currentFrame++;

            if (animation->currentFrame == animation->frameCount) {
               if (animation->repeated) {
                  animation->currentFrame = 0;

               } else {
                  int animationFrameID = animation->frameIDS[animation->currentFrame];

                  Vector2i frameCoordinates;

                  frameCoordinates = animation->coordinateSupplier.at(animationFrameID);

                  // Sets the texture sprite sheets coordinates to the animation frame coordinates
                  texture->setSpritesheetCoordinates(frameCoordinates);
                  entity->remove<AnimationComponent>();
                  return;
               }
            }
            if (animation->currentFrame == pause->pauseFrame) {
               pause->pause(pause->pauseLength);
               animation->playing = false;
            }
         }
      } else {
         pause->pauseTimer--;

         if (pause->pauseTimer == 0) {
            animation->playing = true;
         }
      }

      int animationFrameID = animation->frameIDS[animation->currentFrame];

      Vector2i frameCoordinates;

      frameCoordinates = animation->coordinateSupplier.at(animationFrameID);

      // Sets the texture sprite sheets coordinates to the animation frame coordinates
      texture->setSpritesheetCoordinates(frameCoordinates);
   });
}
