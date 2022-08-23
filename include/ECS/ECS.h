#pragma once

#include <SDL2/SDL.h>

#include <algorithm>
#include <array>
#include <bitset>
#include <cassert>
#include <iostream>
#include <iterator>
#include <memory>
#include <string>
#include <vector>
/*
 * This program uses an Entity Component System.
 *
 * An Entity Component system doesn't use inheritance to make entities,
 * an entity is composed of Components, which contain data of things such
 * as position and rotation. This saves a lot of memory and makes everything
 * more organized
 *
 * The game is controlled by Systems, which is where it performs actions based
 * on which entities have certain components and the data in the components.
 *
 * The world is made up these Systems and the tick() function performs the
 * actions for the Systems
 * */

struct Component;

class Entity;

class System;

class World;

using ComponentID = std::uint8_t;

inline ComponentID getNewComponentTypeID() {
   static ComponentID lastID = 0;
   std::cout << std::to_string(lastID) << '\n';
   return lastID++;
}

template <typename T>
inline ComponentID getComponentTypeID() noexcept {
   static ComponentID typeID = getNewComponentTypeID();
   return typeID;
}

constexpr std::uint8_t maxComponents = 64;

using ComponentBitSet = std::bitset<maxComponents>;
using ComponentArray = std::array<Component*, maxComponents>;

struct Component {
   virtual ~Component() = default;
};

class Entity {
  public:
   Entity() {}

   Entity(const Entity& other) = delete;

   ~Entity() {}

   // Adds a component to the Entity, along with the arguments for the component
   template <typename ComponentType, typename... Args>
   inline ComponentType* addComponent(Args&&... arguments) {
      auto component = std::make_unique<ComponentType>(std::forward<Args>(arguments)...);
      auto* ptr = component.get();

      components.emplace_back(std::move(component));

      componentArray[getComponentTypeID<ComponentType>()] = ptr;
      componentBitset[getComponentTypeID<ComponentType>()] = true;

      return ptr;
   }

   // Removes all of a certain component type from the entity
   template <typename ComponentType>
   void remove() {
      if (!hasComponent<ComponentType>()) {
         return;
      }

      componentBitset[getComponentTypeID<ComponentType>()] = false;

      auto* toRemove = componentArray[getComponentTypeID<ComponentType>()];

      components.erase(std::remove_if(components.begin(), components.end(), [toRemove](auto& ptr) {
         return ptr.get() == toRemove;
      }));
   }

   template <typename A, typename B, typename... OTHERS>
   void remove() {
      if (!hasComponent<A>()) {
         remove<B, OTHERS...>();
         return;
      }

      remove<A>();

      remove<B, OTHERS...>();
   }

   // Removes all of the components from the entity
   bool clearComponents() {
      components.clear();
      return true;
   }

   // Returns the component it needs to get
   template <typename C>
   inline C* getComponent() {
      auto ptr(componentArray[getComponentTypeID<C>()]);
      return static_cast<C*>(ptr);
   }

   // Returns true if it has the one component
   template <typename C>
   inline bool hasComponent() const {
      return componentBitset[getComponentTypeID<C>()];
   }

   // Returns true if it has all of the specified components
   template <typename A, typename B, typename... OTHERS>
   inline bool hasComponent() const {
      return hasComponent<A>() && hasComponent<B, OTHERS...>();
   }

   // This is only here to allow the hasAny() to work with one template argument
   template <typename A>
   inline bool hasAny() const {
      return hasComponent<A>();
   }

   // Returns true if it has at least one of the specified components
   template <typename A, typename B, typename... OTHERS>
   inline bool hasAny() const {
      return hasComponent<A>() || hasAny<B, OTHERS...>();
   }

  private:
   std::vector<std::unique_ptr<Component>> components;
   ComponentArray componentArray;
   ComponentBitSet componentBitset;
};

class System {
   friend class World;

  public:
   virtual ~System() = default;

   virtual void onAddedToWorld(World* world) {}

   virtual void tick(World* world) = 0;

   virtual void handleInput(SDL_Event& event) {}

   virtual void handleInput(const Uint8* keystates) {}

   virtual void onRemovedFromWorld(World* world) {}

   void setEnabled(bool val) {
      enabled = val;
   }

   bool isEnabled() {
      return enabled;
   }

  private:
   bool enabled = true;
};

using SystemID = std::uint8_t;

inline SystemID getNewSystemTypeID() {
   static SystemID lastID = 0;
   return lastID++;
}

template <typename T>
inline SystemID getSystemTypeID() {
   static SystemID typeID = getNewSystemTypeID();
   return typeID;
}

constexpr std::uint8_t maxSystems = 16;

using SystemArray = std::array<System*, maxSystems>;

class World {
  public:
   World() = default;

   World(const World& other) = delete;

   ~World() {
      for (auto& system : systems) {
         system->onRemovedFromWorld(this);
      }
      systems.clear();

      for (auto entity : entities) {
         delete entity;
      }
      entities.clear();
   }

   Entity* create() {
      auto* entity = new Entity();
      entities.push_back(entity);
      return entity;
   }

   void destroy(Entity* entity) {
      assert(entity);

      destroyQueue.push_back(entity);
   }

   void emptyDestroyQueue() {
      for (Entity* entity : destroyQueue) {
         entities.erase(std::remove_if(entities.begin(), entities.end(),
                                       [entity](Entity* other) {
                                          return entity == other;
                                       }),
                        entities.end());

         delete entity;
      }

      destroyQueue.clear();
   }

   template <typename... Components>
   void destroyAll() {
      for (Entity* entity : entities) {
         if (entity->hasComponent<Components...>()) {
            destroy(entity);
         }
      }
   }

   template <typename S, typename... Args>
   S* registerSystem(Args&&... arguments) {
      std::unique_ptr<S> system = std::make_unique<S>(std::forward<Args>(arguments)...);

      auto* ptr = system.get();

      systemArray[getSystemTypeID<S>()] = ptr;
      systems.emplace_back(std::move(system));

      ptr->onAddedToWorld(this);

      return ptr;
   }

   template <typename T>
   void unregisterSystem() {
      auto* toRemove = systemArray[getSystemTypeID<T>()];

      systems.erase(std::remove_if(systems.begin(), systems.end(), [toRemove, this](auto& ptr) {
         if (ptr.get() == toRemove) {
            ptr->onRemovedFromWorld(this);
            return true;
         }
         return false;
      }));
   }

   template <typename T>
   T* getSystem() {
      auto ptr(systemArray[getSystemTypeID<T>()]);
      return static_cast<T*>(ptr);
   }

   template <typename T>
   void disableSystem() {
      getSystem<T>()->setEnabled(false);
   }

   template <typename A, typename B, typename... OTHERS>
   void disableSystem() {
      disableSystem<A>();

      disableSystem<B, OTHERS...>();
   }

   template <typename T>
   void enableSystem() {
      getSystem<T>()->setEnabled(true);
   }

   template <typename A, typename B, typename... OTHERS>
   void enableSystem() {
      enableSystem<A>();

      enableSystem<B, OTHERS...>();
   }

   bool hasEntity(Entity* entity) {
      auto it = std::find(entities.begin(), entities.end(), entity);

      return it != entities.end();
   }

   template <typename... Components>
   Entity* findFirst() {
      auto found = std::find_if(entities.begin(), entities.end(), [&](const Entity* entity) {
         return entity->hasComponent<Components...>();
      });
      return found != entities.end() ? *found : nullptr;
   }

   template <typename... Components, typename Function>
   void find(Function callback) {
      for (Entity* entity : entities) {
         if (entity->hasComponent<Components...>()) {
            callback(entity);
         }
      }
   }

   template <typename... Components>
   std::vector<Entity*> findAny() {
      std::vector<Entity*> result;
      std::copy_if(entities.begin(), entities.end(), std::back_inserter(result),
                   [&](const Entity* s) {
                      return s->hasAny<Components...>();
                   });
      return result;
   }

   void tick() {
      for (auto& system : systems) {
         if (system->isEnabled()) {
            system->tick(this);
            emptyDestroyQueue();
         }
      }
   }

   void handleInput(SDL_Event& event) {
      for (auto& system : systems) {
         if (system->isEnabled()) {
            system->handleInput(event);
         }
      }
   }

   void handleInput(const Uint8* keystates) {
      for (auto& system : systems) {
         if (system->isEnabled()) {
            system->handleInput(keystates);
         }
      }
   }

   std::vector<Entity*>& getEntities() {
      return entities;
   }

  private:
   std::vector<Entity*> entities;
   std::vector<Entity*> destroyQueue;

   SystemArray systemArray;
   std::vector<std::unique_ptr<System>> systems;
};
