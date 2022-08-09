#pragma once

#include "ECS/ECS.h"
#include "scenes/GameScene.h"

#include <SDL2/SDL.h>

class GameScene;

class WarpSystem : public System {
  public:
   WarpSystem(GameScene* scene);

   void onAddedToWorld(World* world) override;

   void tick(World* world) override;

   void handleEvent(SDL_Event& event) override;

   void warp(World* world, Entity* pipe, Entity* player);

   void climb(World* world, Entity* vine, Entity* player);

   static bool isWarping() {
      return warping;
   }

   static bool isClimbing() {
      return climbing;
   }

   static bool hasClimbed() {
      return climbed;
   }

   static void setWarping(bool val) {
      warping = val;
   }

   static void setClimbing(bool val) {
      climbing = val;
   }

   static void setClimbed(bool val) {
      climbed = val;
   }

   void setTeleportLevelY(int levelY) {
      teleportLevelY = levelY;
   }

   void setTeleportPlayerCoordinates(Vector2i playerCoordinates) {
      teleportPlayerCoordinates = playerCoordinates;
   }

   void setTeleportCameraCoordinates(Vector2i cameraCoordinates) {
      teleportCameraCoordinates = cameraCoordinates;
   }

   int up = 0;
   int down = 0;
   int left = 0;
   int right = 0;

  private:
   static bool warping;
   static bool climbing;
   static bool climbed;

   GameScene* scene;

   int teleportLevelY;
   Vector2i teleportPlayerCoordinates;
   Vector2i teleportCameraCoordinates;
};
