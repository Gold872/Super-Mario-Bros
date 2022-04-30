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

   void limitFPS(Uint64 startTick);

   static void setRunning(bool val);

  private:
   Game game;
   static bool running;
};
