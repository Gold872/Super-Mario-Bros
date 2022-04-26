#pragma once

#include "Scene.h"

#include <SDL2/SDL.h>

class MenuScene : public Scene {
  public:
   MenuScene();

   //   void update() override;
   //
   //   bool isFinished() override;
   //
   //   void handleEvents(SDL_Event& event) override;

  private:
   void createMenuEntities();
};
