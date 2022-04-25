#pragma once

#include <SDL2/SDL.h>

#include <algorithm>
#include <array>
#include <bitset>
#include <iostream>
#include <iterator>
#include <memory>
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

      components.erase(std::find(
          components.begin(), components.end(),
          std::unique_ptr<Component>{componentArray[getComponentTypeID<ComponentType>()]}));

      delete componentArray[getComponentTypeID<ComponentType>()];
   }

   template <typename A, typename B, typename... OTHERS>
   void remove() {
      if (!hasComponent<A>()) {
         remove<B, OTHERS...>();
         return;
      }

      componentBitset[getComponentTypeID<A>()] = false;

      components.erase(
          std::find(components.begin(), components.end(),
                    std::unique_ptr<Component>{componentArray[getComponentTypeID<A>()]}));

      delete componentArray[getComponentTypeID<A>()];

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

   virtual void handleEvent(SDL_Event& event) {}

   virtual void handleEvent(const Uint8* keystates) {}

   virtual void onRemovedFromWorld(World* world) {}

   void setActive(bool val) {
      active = val;
   }

   bool isActive() {
      return active;
   }

  private:
   bool active = true;
};

class World {
  public:
   World() = default;

   World(const World& other) = delete;

   ~World() {
      for (auto system : systems) {
         system->onRemovedFromWorld(this);
         delete system;
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
      if (!entity) {
         return;
      }
      entities.erase(std::remove(entities.begin(), entities.end(), entity), entities.end());
      delete entity;
   }

   template <typename S, typename... Args>
   S* registerSystem(Args&&... arguments) {
      auto* system = new S(std::forward<Args>(arguments)...);
      systems.push_back(system);
      system->onAddedToWorld(this);
      return system;
   }

   void unregisterSystem(System* system) {
      if (!system) {
         return;
      }
      systems.erase(std::remove(systems.begin(), systems.end(), system), systems.end());
      system->onRemovedFromWorld(this);
      delete system;
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
      for (auto system : systems) {
         if (system->isActive()) {
            system->tick(this);
         }
      }
   }

   void handleEvent(SDL_Event& event) {
      for (auto system : systems) {
         if (system->isActive()) {
            system->handleEvent(event);
         }
      }
   }

   void handleEvent(const Uint8* keystates) {
      for (auto system : systems) {
         if (system->isActive()) {
            system->handleEvent(keystates);
         }
      }
   }

   std::vector<Entity*> getEntities() {
      return entities;
   }

  private:
   std::vector<Entity*> entities;
   std::vector<System*> systems;
};
