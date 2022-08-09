#include "systems/MenuSystem.h"

#include "Constants.h"
#include "ECS/Components.h"
#include "Math.h"
#include "TextureManager.h"

#include <string>

void MenuSystem::onAddedToWorld(World* world) {
   {  // LOGO
      logo = world->create();
      logo->addComponent<PositionComponent>(Vector2f(7 * SCALED_CUBE_SIZE, 2 * SCALED_CUBE_SIZE),
                                            Vector2i(11 * SCALED_CUBE_SIZE, 6 * SCALED_CUBE_SIZE));
      logo->addComponent<TextureComponent>(
          TextureManager::Get().LoadSharedTexture("res/sprites/icons/logo.png"));
      logo->addComponent<SpritesheetComponent>(11 * ORIGINAL_CUBE_SIZE, 6 * ORIGINAL_CUBE_SIZE, 0,
                                               0, 0, 176, 96, Vector2i(0, 0));
      logo->addComponent<IconComponent>();
   }
   {  // ABOUT TEXT
      aboutText = world->create();
      aboutText->addComponent<PositionComponent>(
          Vector2f(5.5 * SCALED_CUBE_SIZE, 8.5 * SCALED_CUBE_SIZE), Vector2i());
      aboutText->addComponent<TextComponent>("Recreated by Gold87 using C++ and SDL2", 12);
   }
   {  // SELECT TEXT
      Entity* selectText(world->create());
      selectText->addComponent<PositionComponent>(
          Vector2f(9.5 * SCALED_CUBE_SIZE, 10 * SCALED_CUBE_SIZE), Vector2i());
      selectText->addComponent<TextComponent>("Select a Level", 15);
   }
   {  // LEVEL NUMBER
      levelNumber = world->create();
      levelNumber->addComponent<PositionComponent>(
          Vector2f(11.5 * SCALED_CUBE_SIZE, 11 * SCALED_CUBE_SIZE), Vector2i());
      levelNumber->addComponent<TextComponent>(
          std::to_string(selectedLevel) + " - " + std::to_string(selectedSublevel), 15);
   }
   {  // UNDERLINE
      underline = world->create();
      underline->addComponent<PositionComponent>(
          Vector2f(11.5 * SCALED_CUBE_SIZE, 11.2 * SCALED_CUBE_SIZE), Vector2i());
      underline->addComponent<TextComponent>("_", 15);
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
      if (currentFocus == 0) {
         underline->getComponent<PositionComponent>()->position.x = 11.5 * SCALED_CUBE_SIZE;
      } else if (currentFocus == 1) {
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

void MenuSystem::handleEvent(SDL_Event& event) {
   if (event.type != SDL_KEYDOWN) {
      return;
   }
   switch (event.key.keysym.scancode) {
      case SDL_SCANCODE_UP:
      case SDL_SCANCODE_W:
         if (event.key.repeat == 0) {
            if (currentFocus == 0) {
               if (selectedLevel < maxLevel) {
                  selectedLevel++;
                  levelChange = true;
               }
            } else if (currentFocus == 1) {
               if (selectedSublevel < maxSublevel) {
                  selectedSublevel++;
                  levelChange = true;
               }
            }
         }
         break;
      case SDL_SCANCODE_DOWN:
      case SDL_SCANCODE_S:
         if (event.key.repeat == 0) {
            if (currentFocus == 0) {
               if (selectedLevel > 1) {
                  selectedLevel--;
                  levelChange = true;
               }
            } else if (currentFocus == 1) {
               if (selectedSublevel > 1) {
                  selectedSublevel--;
                  levelChange = true;
               }
            }
         }
         break;
      case SDL_SCANCODE_LEFT:
      case SDL_SCANCODE_A:
         if (event.key.repeat == 0) {
            if (currentFocus != 0) {
               currentFocus = 0;
               underlineChange = true;
            }
         }
         break;
      case SDL_SCANCODE_RIGHT:
      case SDL_SCANCODE_D:
         if (event.key.repeat == 0) {
            if (currentFocus != 1) {
               currentFocus = 1;
               underlineChange = true;
            }
         }
         break;
      default:
         break;
   }
}
