#include "systems/MenuSystem.h"

#include "Constants.h"
#include "ECS/Components.h"
#include "Input.h"
#include "SMBMath.h"
#include "TextureManager.h"

#include <string>

void MenuSystem::onAddedToWorld(World* world) {
   {  // LOGO
      logo = world->create();
      logo->addComponent<PositionComponent>(Vector2f(7 * SCALED_CUBE_SIZE, 2 * SCALED_CUBE_SIZE),
                                            Vector2i(11 * SCALED_CUBE_SIZE, 6 * SCALED_CUBE_SIZE));
      logo->addComponent<TextureComponent>(
          TextureManager::Get().LoadSharedTexture("res/sprites/icons/logo.png"));
      logo->addComponent<IconComponent>();
   }
   {  // LEVEL SELECT BACKGROUND
      levelSelectBackground = world->create();
      levelSelectBackground->addComponent<PositionComponent>(Vector2f(5.5, 9.5) * SCALED_CUBE_SIZE,
                                                             Vector2i(14, 3) * SCALED_CUBE_SIZE);
      levelSelectBackground
          ->addComponent<TextureComponent>(TextureManager::Get().LoadSharedTexture(
              "res/sprites/icons/optionsinfobackground.png", false))
          ->setVisible(false);
      levelSelectBackground->addComponent<IconComponent>();
   }
   {  // ABOUT TEXT
      aboutText = world->create();
      aboutText->addComponent<PositionComponent>(
          Vector2f(5.5 * SCALED_CUBE_SIZE, 8.5 * SCALED_CUBE_SIZE), Vector2i());
      aboutText->addComponent<TextComponent>("Recreated by Gold87 using C++ and SDL2", 12);
   }
   {  // SELECT TEXT
      selectText = world->create();
      selectText->addComponent<PositionComponent>(Vector2f(9, 10) * SCALED_CUBE_SIZE, Vector2i());
      selectText->addComponent<TextComponent>("Level Select", 15);
   }
   {  // OPTIONS TEXT
      optionsText = world->create();
      optionsText->addComponent<PositionComponent>(Vector2f(9, 11) * SCALED_CUBE_SIZE, Vector2i());
      optionsText->addComponent<TextComponent>("Options", 15);
   }
   {  // LEVEL SELECT TEXT
      levelSelectText = world->create();
      levelSelectText->addComponent<PositionComponent>(
          Vector2f(9.5 * SCALED_CUBE_SIZE, 10 * SCALED_CUBE_SIZE), Vector2i());
      levelSelectText->addComponent<TextComponent>("Select a Level", 15, false, false);
   }
   {  // LEVEL NUMBER
      levelNumber = world->create();
      levelNumber->addComponent<PositionComponent>(
          Vector2f(11.5 * SCALED_CUBE_SIZE, 11 * SCALED_CUBE_SIZE), Vector2i());
      levelNumber->addComponent<TextComponent>(
          std::to_string(selectedLevel) + " - " + std::to_string(selectedSublevel), 15, false,
          false);
   }
   {  // UNDERLINE
      underline = world->create();
      underline->addComponent<PositionComponent>(
          Vector2f(11.5 * SCALED_CUBE_SIZE, 11.2 * SCALED_CUBE_SIZE), Vector2i());
      underline->addComponent<TextComponent>("_", 15, false, false);
   }
   {  // CURSOR
      cursor = world->create();
      cursor->addComponent<PositionComponent>(Vector2f(8, 10) * SCALED_CUBE_SIZE, Vector2i());
      cursor->addComponent<TextComponent>(">", 15);
   }
}

void MenuSystem::tick(World* world) {
   if (levelChange) {
      levelNumber->getComponent<TextComponent>()->destroyTexture();
      levelNumber->getComponent<TextComponent>()->text =
          std::to_string(selectedLevel) + " - " + std::to_string(selectedSublevel);

      levelChange = false;
   }
   if (underlineChange) {
      if (currentLevelFocus == 0) {
         underline->getComponent<PositionComponent>()->position.x = 11.5 * SCALED_CUBE_SIZE;
      } else if (currentLevelFocus == 1) {
         underline->getComponent<PositionComponent>()->position.x = 13.4 * SCALED_CUBE_SIZE;
      }
      underlineChange = false;
   }
}

int MenuSystem::getSelectedLevel() {
   return selectedLevel;
}

int MenuSystem::getSelectedSublevel() {
   return selectedSublevel;
}

void MenuSystem::hideMenuText() {
   cursor->getComponent<TextComponent>()->setVisible(false);
   aboutText->getComponent<TextComponent>()->setVisible(false);
   selectText->getComponent<TextComponent>()->setVisible(false);
   optionsText->getComponent<TextComponent>()->setVisible(false);
   levelSelectText->getComponent<TextComponent>()->setVisible(false);
   levelNumber->getComponent<TextComponent>()->setVisible(false);
   underline->getComponent<TextComponent>()->setVisible(false);
}

void MenuSystem::showMenuText() {
   cursor->getComponent<TextComponent>()->setVisible(true);
   aboutText->getComponent<TextComponent>()->setVisible(true);
   selectText->getComponent<TextComponent>()->setVisible(true);
   optionsText->getComponent<TextComponent>()->setVisible(true);
   //   levelSelectText->getComponent<TextComponent>()->setVisible(true);
   //   levelNumber->getComponent<TextComponent>()->setVisible(true);
   //   underline->getComponent<TextComponent>()->setVisible(true);
}

void MenuSystem::enterLevelSelect() {
   currentFocus = 1;

   levelSelectBackground->getComponent<TextureComponent>()->setVisible(true);
   levelSelectText->getComponent<TextComponent>()->setVisible(true);
   levelNumber->getComponent<TextComponent>()->setVisible(true);
   underline->getComponent<TextComponent>()->setVisible(true);

   cursor->getComponent<TextComponent>()->setVisible(false);
   selectText->getComponent<TextComponent>()->setVisible(false);
   optionsText->getComponent<TextComponent>()->setVisible(false);
}

void MenuSystem::exitLevelSelect() {
   currentFocus = 0;

   levelSelectBackground->getComponent<TextureComponent>()->setVisible(false);
   levelSelectText->getComponent<TextComponent>()->setVisible(false);
   levelNumber->getComponent<TextComponent>()->setVisible(false);
   underline->getComponent<TextComponent>()->setVisible(false);

   cursor->getComponent<TextComponent>()->setVisible(true);
   selectText->getComponent<TextComponent>()->setVisible(true);
   optionsText->getComponent<TextComponent>()->setVisible(true);
}

void MenuSystem::handleInput() {
   Input& input = Input::Get();

   if (input.getKeyPressed(Key::MENU_ACCEPT)) {
      if (currentFocus == 0 && currentOption == 0) {
         enterLevelSelect();
      } else if (currentFocus == 1) {
         levelSelect = true;
      }
   }

   if (input.getKeyPressed(Key::MENU_ESCAPE)) {
      if (currentFocus == 1) {
         exitLevelSelect();
      }
   }

   if (input.getKeyPressed(Key::MENU_UP)) {
      switch (currentFocus) {
         case 0:
            if (currentOption > 0) {
               currentOption--;
               cursor->getComponent<PositionComponent>()->position.y -= SCALED_CUBE_SIZE;
            }
            break;
         case 1:
            if (currentLevelFocus == 0) {
               if (selectedLevel < maxLevel) {
                  selectedLevel++;
                  levelChange = true;
               }
            } else if (currentLevelFocus == 1) {
               if (selectedSublevel < maxSublevel) {
                  selectedSublevel++;
                  levelChange = true;
               }
            }
            break;
         default:
            break;
      }
   }

   if (input.getKeyPressed(Key::MENU_DOWN)) {
      switch (currentFocus) {
         case 0:
            if (currentOption < 1) {
               currentOption++;
               cursor->getComponent<PositionComponent>()->position.y += SCALED_CUBE_SIZE;
            }
            break;
         case 1:
            if (currentLevelFocus == 0) {
               if (selectedLevel > 1) {
                  selectedLevel--;
                  levelChange = true;
               }
            } else if (currentLevelFocus == 1) {
               if (selectedSublevel > 1) {
                  selectedSublevel--;
                  levelChange = true;
               }
            }
            break;
         default:
            break;
      }
   }

   if (input.getKeyPressed(Key::MENU_LEFT)) {
      if (currentLevelFocus != 0) {
         currentLevelFocus = 0;
         underlineChange = true;
      }
   }

   if (input.getKeyPressed(Key::MENU_RIGHT)) {
      if (currentLevelFocus != 1) {
         currentLevelFocus = 1;
         underlineChange = true;
      }
   }
}
