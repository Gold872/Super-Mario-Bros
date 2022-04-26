#include "Camera.h"
#include "Core.h"
#include "Game.h"
#include "TextureManager.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

Game::Game() {}

void Game::init() {
   scene = std::make_unique<GameScene>(1, 1);
}

void Game::handleInput() {
   const Uint8* keystates = SDL_GetKeyboardState(nullptr);
   SDL_Event event;
   while (SDL_PollEvent(&event)) {
      switch (event.type) {
         case SDL_QUIT:
            core->setRunning(false);
            break;
         case SDL_WINDOWEVENT:
            switch (event.window.event) {
               case SDL_WINDOWEVENT_RESIZED:
                  TextureManager::Get().ResizeWindow();
                  break;
               default:
                  break;
            }
            break;
         default:
            break;
      }
      scene->handleEvents(event);
   }
   scene->handleEvents(keystates);
}

void Game::update() {
   scene->update();
}

void Game::setCore(Core* core) {
   this->core = core;
}
