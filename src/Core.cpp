#include "Core.h"

#include "Constants.h"
#include "TextureManager.h"

#include <iostream>

bool Core::running;

Core::Core() {
   running = true;
   game.setCore(this);
}

void Core::limitFPS(int startTick) {
   if (1000 / MAX_FPS > SDL_GetTicks() - startTick) {
      SDL_Delay(1000 / MAX_FPS - (SDL_GetTicks() - startTick));
   }
}

int Core::init() {
   if (TextureManager::Init() != 0) {
      std::cerr << "Error Initializing Texture Manager" << std::endl;
      return -1;
   }
   game.init();
   return 0;
}

void Core::run() {
   while (running) {
      mainLoop();
   }
   TextureManager::Quit();
}

void Core::mainLoop() {
   unsigned int ticksNow = SDL_GetTicks();
   game.handleInput();
   game.update();
   limitFPS(ticksNow);
   std::cout << std::flush;
}

void Core::setRunning(bool val) {
   running = val;
}
