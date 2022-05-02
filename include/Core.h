#pragma once

#include "Game.h"

class Core {
  public:
   Core();

   int init();

   void run();

   void mainLoop();

   void limitFPS(Uint64 startTick);

   void setRunning(bool val);

  private:
   Game game;
   bool running;
};
