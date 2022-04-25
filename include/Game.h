#pragma once

#include "ECS/Components.h"
#include "ECS/ECS.h"
#include "Map.h"
#include "scenes/GameScene.h"
#include "scenes/Scene.h"

#include <SDL2/SDL.h>

#include <memory>

/*
 * The World class is where all of the game logic happens, with things
 * such as collision detection, and input handling.
 * */

class Core;

class Game {
  public:
   Game();

   void init();

   void handleInput();
   void loadLevel(int level, int subLevel);
   void loadSubRoom(int roomNumber);
   void loadEntities();
   void update();

   void setCore(Core* core);

  private:
   Core* core;

   std::unique_ptr<Scene> scene;

   Map entitiesMap;
   Map foregroundMap;
   Map undergroundMap;
   Map backgroundMap;
};
