#include "Core.h"

#include "Constants.h"
#include "SoundManager.h"
#include "TextureManager.h"

#include <iostream>
#include <stdlib.h>

bool Core::running;

Core::Core() {
   running = true;
   game.setCore(this);
}

void Core::limitFPS(Uint64 startTick) {
   if (1000 / MAX_FPS > SDL_GetTicks64() - startTick) {
      SDL_Delay(1000 / MAX_FPS - (SDL_GetTicks64() - startTick));
   }
}

int Core::init() {
   if (TextureManager::Get().Init() != 0) {
      std::cerr << "Error Initializing Texture Manager" << std::endl;
      return -1;
   }
   if (SoundManager::Get().Init() != 0) {
      std::cerr << "Error Initializing Sound Manager" << std::endl;
      return -1;
   }
   game.init();
   return 0;
}

void Core::run() {
   srand(time(NULL));  // Generates a random time seed for the game to generate random numbers
   while (running) {
      mainLoop();
   }
   TextureManager::Get().Quit();
   SoundManager::Get().Quit();
}

void Core::mainLoop() {
   Uint64 startTicks = SDL_GetTicks64();
   game.handleInput();
   game.update();
   limitFPS(startTicks);
   std::cout << std::flush;
}

void Core::setRunning(bool val) {
   running = val;
}
