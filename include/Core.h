#pragma once

#include "Game.h"

class Core {
  public:
   Core();

   enum GameState
   {
      MAIN_MENU,
      GAME,
      TRANSITION
   };

   int init();

   void run();

   void mainLoop();

   void limitFPS(int startTick);

   static void setRunning(bool val);

  private:
   Game game;
   static bool running;
};
