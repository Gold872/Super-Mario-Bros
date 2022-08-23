#pragma once

#include "ECS/ECS.h"

class MenuSystem : public System {
  public:
   void onAddedToWorld(World* world) override;

   void tick(World* world) override;

   void handleInput(SDL_Event& event) override;

   int getSelectedLevel();

   int getSelectedSublevel();

  private:
   bool levelChange = false;
   bool underlineChange = false;

   int selectedLevel = 1;
   int selectedSublevel = 1;
   const int maxLevel = 8;
   const int maxSublevel = 4;

   int currentFocus = 0;  // 0 is level, 1 is sublevel

   Entity* logo;
   Entity* aboutText;

   Entity* levelNumber;
   Entity* underline;
};
