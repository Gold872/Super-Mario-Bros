#pragma once

#include "Map.h"
#include "Scene.h"
#include "systems/MenuSystem.h"

#include <SDL2/SDL.h>

class MenuScene : public Scene {
  public:
   MenuScene();

   void handleInput() override;

   bool isFinished() override;

   int getSelectedLevel();

   int getSelectedSublevel();

  private:
   MenuSystem* menuSystem;

   bool finished = false;

   void createMenuEntities();

   Map backgroundMap;
};
