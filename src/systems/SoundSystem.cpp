#include "systems/SoundSystem.h"

#include "ECS/Components.h"
#include "SoundManager.h"

void SoundSystem::tick(World* world) {
   world->find<SoundComponent>([&](Entity* entity) {
      auto* sound = entity->getComponent<SoundComponent>();

      SoundManager::Get().playSound(sound->soundID);

      world->destroy(entity);
   });

   world->find<MusicComponent>([&](Entity* entity) {
      auto* music = entity->getComponent<MusicComponent>();

      SoundManager::Get().playMusic(music->musicID);

      world->destroy(entity);
   });
}
