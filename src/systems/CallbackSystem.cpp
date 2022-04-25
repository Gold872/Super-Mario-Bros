#include "systems/CallbackSystem.h"

#include "ECS/Components.h"
#include "ECS/ECS.h"

void CallbackSystem::tick(World* world) {
   world->find<CallbackComponent>([&](Entity* entity) {
      auto* callback = entity->getComponent<CallbackComponent>();

      callback->time--;

      if (callback->time == 0) {
         callback->callback(entity);
         entity->remove<CallbackComponent>();
      }
   });
   world->find<DestroyDelayedComponent>([&](Entity* entity) {
      auto* destroy = entity->getComponent<DestroyDelayedComponent>();

      if (destroy->time > 0) {
         destroy->time--;
      } else if (destroy->time <= 0) {
         world->destroy(entity);
      }
   });
   world->find<WaitUntilComponent>([&](Entity* entity) {
      auto* waitUntil = entity->getComponent<WaitUntilComponent>();

      if (waitUntil->condition(entity)) {
         waitUntil->doAfter(entity);
      }
   });
   world->find<TimerComponent>([&](Entity* entity) {
      auto* timer = entity->getComponent<TimerComponent>();

      timer->time--;
      if (timer->time == 0) {
         timer->onExecute(entity);
         timer->time = timer->delay;
      }
   });
}
