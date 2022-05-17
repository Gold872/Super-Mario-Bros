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

   world->find<SequenceComponent>([&](Entity* entity) {
      auto* sequence = entity->getComponent<SequenceComponent>();

      sequence->currentDelay--;
      if (sequence->currentDelay <= 0) {
         sequence->currentSequenceIndex++;

         if (sequence->currentSequenceIndex == sequence->sequenceLength) {
            if (sequence->repeated) {
               sequence->currentSequenceIndex = 0;
            } else {
               entity->remove<SequenceComponent>();
               return;
            }
         }

         sequence->currentDelay = sequence->delays[sequence->currentSequenceIndex];

      } else if (sequence->currentDelay + 1 == sequence->delays[sequence->currentSequenceIndex]) {
         sequence->commands[sequence->currentSequenceIndex](entity);
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
}
