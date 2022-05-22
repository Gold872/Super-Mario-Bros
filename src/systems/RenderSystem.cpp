#include "systems/RenderSystem.h"

#include "Camera.h"
#include "Constants.h"
#include "ECS/Components.h"
#include "TextureManager.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <cmath>
#include <cstring>
#include <iostream>

void RenderSystem::onAddedToWorld(World* world) {
   normalFont = TextureManager::Get().LoadSharedFont("res/fonts/press-start-2p.ttf", 25);
}

void RenderSystem::tick(World* world) {
   TextureManager::Get().Clear();
   // This is to render the entities in the correct order
   world->find<PositionComponent, TextureComponent, BackgroundComponent>([&](Entity* entity) {
      if (Camera::Get().inCameraRange(entity->getComponent<PositionComponent>())) {
         renderEntity(entity);
      }
   });
   world->find<PositionComponent, TextureComponent, ForegroundComponent>([&](Entity* entity) {
      if (Camera::Get().inCameraRange(entity->getComponent<PositionComponent>())) {
         renderEntity(entity);
      }
   });
   world->find<PositionComponent, TextureComponent, ProjectileComponent>([&](Entity* entity) {
      renderEntity(entity);
   });
   world->find<PositionComponent, TextureComponent, CollectibleComponent>([&](Entity* entity) {
      if (Camera::Get().inCameraRange(entity->getComponent<PositionComponent>())) {
         renderEntity(entity);
      }
   });
   world->find<PositionComponent, TextureComponent, EnemyComponent>([&](Entity* entity) {
      if (Camera::Get().inCameraRange(entity->getComponent<PositionComponent>())) {
         renderEntity(entity);
      }
   });
   world->find<PositionComponent, TextComponent, FloatingTextComponent>([&](Entity* entity) {
      renderText(entity, entity->getComponent<TextComponent>()->followCamera);
   });
   world->find<PositionComponent, TextureComponent, PlayerComponent>([&](Entity* entity) {
      renderEntity(entity);
   });
   world->find<PositionComponent, TextureComponent, AboveForegroundComponent>([&](Entity* entity) {
      if (Camera::Get().inCameraRange(entity->getComponent<PositionComponent>())) {
         renderEntity(entity);
      }
   });
   world->find<PositionComponent, TextureComponent, ParticleComponent>([&](Entity* entity) {
      renderEntity(entity);
   });

   world->find<PositionComponent, TextureComponent, IconComponent>([&](Entity* entity) {
      renderEntity(entity, false);
   });
   world->find<PositionComponent, TextComponent>([&](Entity* entity) {
      if (!entity->hasComponent<FloatingTextComponent>()) {
         renderText(entity, entity->getComponent<TextComponent>()->followCamera);
      }
   });

   TextureManager::Get().Display();
}

void RenderSystem::renderEntity(Entity* entity, bool cameraBound) {
   auto* position = entity->getComponent<PositionComponent>();
   auto* texture = entity->getComponent<TextureComponent>();

   float screenPositionX =
       (cameraBound) ? position->position.x - Camera::Get().getCameraX() : position->position.x;
   float screenPositionY =
       (cameraBound) ? position->position.y - Camera::Get().getCameraY() : position->position.y;

   SDL_Rect destinationRect = {(int)std::round(screenPositionX), (int)std::round(screenPositionY),
                               position->scale.x, position->scale.y};

   if (texture->isVisible()) {
      TextureManager::Get().Draw(texture->getTexture(), texture->getSourceRect(), destinationRect,
                                 texture->isHorizontalFlipped(), texture->isVerticalFlipped());
   }
}

void RenderSystem::renderText(Entity* entity, bool followCamera) {
   auto* position = entity->getComponent<PositionComponent>();
   auto* textComponent = entity->getComponent<TextComponent>();

   if (textComponent->texture == nullptr || textComponent->texture == NULL) {
      SDL_Color color = {255, 255, 255};
      SDL_Surface* textSurface =
          TTF_RenderText_Blended(normalFont.get(), textComponent->text.c_str(), color);

      SDL_Texture* texture =
          SDL_CreateTextureFromSurface(TextureManager::Get().getRenderer(), textSurface);

      textComponent->texture = texture;
      SDL_FreeSurface(textSurface);
   }
   const char* message = textComponent->text.c_str();

   int messageWidth = strlen(message) * textComponent->fontSize;
   int messageHeight = (int)std::round(textComponent->fontSize * (23.0 / 21.0));

   float screenPositionX =
       (followCamera) ? position->position.x - Camera::Get().getCameraX() : position->position.x;
   float screenPositionY =
       (followCamera) ? position->position.y - Camera::Get().getCameraY() : position->position.y;

   if (textComponent->isVisible()) {
      TextureManager::Get().Draw(
          textComponent->texture,
          (SDL_Rect){(int)std::round(screenPositionX), (int)std::round(screenPositionY),
                     messageWidth, messageHeight});
   }
}
