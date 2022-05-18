#pragma once

#include "ECS/ECS.h"
#include "scenes/GameScene.h"

#include <memory>
#include <tuple>

class GameScene;

class MapSystem : public System {
  public:
   MapSystem(GameScene* scene);

   void tick(World* world) override{};

   void loadEntities(World* world);

  private:
   GameScene* scene;

   Entity* createBlockEntity(World* world, int coordinateX, int coordinateY, int entityID);
   Entity* createPlatformEntity(
       World* world, int coordinateX, int coordinateY, int entityID,
       std::tuple<PlatformMotionType, Direction, Vector2i, int> platformCoordinate);

   int getReferenceBlockID(int entityID);
   int getReferenceBlockIDAsEntity(int entityID, int referenceID);

   int getReferenceEnemyID(int entityID);
   int getReferenceEnemyIDAsEntity(int entityID, int referenceID);

   void addItemDispenser(World* world, Entity* entity, int entityID, int referenceID);

   void createForegroundEntities(World* world, int coordinateX, int coordinateY, int entityID,
                                 int referenceID, bool createInvisibleBlocks = false);

   void createEnemyEntities(World* world, int coordinateX, int coordinateY, int entityID,
                            int referenceID);

   void createFireBarEntities(World* world);
};
