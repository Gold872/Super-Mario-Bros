#pragma once

#include "ECS/ECS.h"

class MenuSystem : public System {
  public:
   void onAddedToWorld(World* world) override;

   void tick(World* world) override;

   void handleInput() override;

   int getSelectedLevel();

   int getSelectedSublevel();

   bool levelSelected() {
      return levelSelect && currentFocus == 1;
   }

   bool optionsSelected() {
      return currentOption == 1;
   }

   void enterLevelSelect();
   void exitLevelSelect();

   void hideMenuText();
   void showMenuText();

  private:
   bool levelChange = false;
   bool underlineChange = false;

   bool levelSelect = false;  // If done selecting a level

   int selectedLevel = 1;
   int selectedSublevel = 1;
   const int maxLevel = 8;
   const int maxSublevel = 4;

   int currentFocus = 0;       // 0 is to select, 1 selecting level
   int currentOption = 0;      // 0 is level select, 1 is options
   int currentLevelFocus = 0;  // 0 is level, 1 is sublevel

   Entity* logo;
   Entity* aboutText;
   Entity* selectText;  // "Level Select"
   Entity* optionsText;

   Entity* cursor;

   Entity* levelSelectBackground;
   Entity* levelSelectText;  // "Select a Level"
   Entity* levelNumber;
   Entity* underline;
};
