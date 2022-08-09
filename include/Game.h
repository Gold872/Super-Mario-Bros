#pragma once

#include "scenes/GameScene.h"
#include "scenes/MenuScene.h"
#include "scenes/Scene.h"

#include <SDL2/SDL.h>

#include <memory>

/*
 * The World class is where all of the game logic happens, with things
 * such as collision detection, and input handling.
 * */

class Core;

enum class Scenes
{
   MENU,
   GAME
};

class Game {
  public:
   Game();
   Game(Core* core);

   void init();

   void handleInput();

   void update();

   void setCore(Core* core);

  private:
   Core* core;

   Scenes currentScene;

   std::unique_ptr<Scene> scene;
};
