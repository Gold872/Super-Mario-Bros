#include "Game.h"

#include "Core.h"
#include "TextureManager.h"

#include <SDL2/SDL.h>

Game::Game() {}

Game::Game(Core* core) {
   this->core = core;
}

void Game::init() {
   currentScene = Scenes::MENU;

   scene = std::make_unique<MenuScene>();
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
      scene->handleInput(event);
   }
   scene->handleInput(keystates);
}

void Game::update() {
   scene->update();

   if (scene->isFinished()) {
      Scene* exitingScene = scene.get();
      switch (currentScene) {
         case Scenes::MENU: {
            MenuScene* menuScene = static_cast<MenuScene*>(exitingScene);

            int startLevel = menuScene->getSelectedLevel();
            int startSublevel = menuScene->getSelectedSublevel();

            scene = std::make_unique<GameScene>(startLevel, startSublevel);

            currentScene = Scenes::GAME;
         } break;
         case Scenes::GAME: {
            GameScene* gameScene = static_cast<GameScene*>(exitingScene);

            if (gameScene->gameWin()) {
               // Go back to menu
               scene = std::make_unique<MenuScene>();

               currentScene = Scenes::MENU;
            } else {
               scene = std::make_unique<GameOverScene>();

               currentScene = Scenes::GAME_OVER;
            }
         } break;
         case Scenes::GAME_OVER:
            scene = std::make_unique<MenuScene>();

            currentScene = Scenes::MENU;
            break;
         default:
            break;
      }
   }
}

void Game::setCore(Core* core) {
   this->core = core;
}
