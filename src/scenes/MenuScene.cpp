#include "scenes/MenuScene.h"

#include "Camera.h"
#include "Constants.h"
#include "ECS/Components.h"
#include "ECS/ECS.h"
#include "Input.h"
#include "TextureManager.h"
#include "systems/Systems.h"

#include <SDL2/SDL.h>

#include <memory>

MenuScene::MenuScene() {
   Map::loadBlockIDS();

   TextureManager::Get().SetBackgroundColor(BackgroundColor::BLUE);

   Camera::Get().setCameraX(0);
   Camera::Get().setCameraY(0);

   menuSystem = world->registerSystem<MenuSystem>();
   world->registerSystem<RenderSystem>();

   createMenuEntities();
}

void MenuScene::handleInput() {
   world->handleInput();

   if (world->hasSystem<OptionsSystem>()) {
      if (world->getSystem<OptionsSystem>()->isFinished()) {
         world->unregisterSystem<OptionsSystem>();
         menuSystem->setEnabled(true);
         menuSystem->showMenuText();
      }
      return;
   }

   if (Input::Get().getKeyPressed(Key::MENU_ACCEPT)) {
      if (menuSystem->levelSelected()) {
         finished = true;
      } else if (menuSystem->optionsSelected()) {
         world->registerSystem<OptionsSystem>();
         menuSystem->setEnabled(false);
         menuSystem->hideMenuText();
      }
      return;
   }
}

bool MenuScene::isFinished() {
   return finished;
}

int MenuScene::getSelectedLevel() {
   return menuSystem->getSelectedLevel();
}

int MenuScene::getSelectedSublevel() {
   return menuSystem->getSelectedSublevel();
}

void MenuScene::createMenuEntities() {
   std::shared_ptr<SDL_Texture> blockTexture =
       TextureManager::Get().LoadSharedTexture("res/sprites/blocks/BlockTileSheet.png");

   backgroundMap.loadMap("res/data/MenuBackground/MenuBackground_Background.csv");

   for (unsigned int i = 0; i < backgroundMap.getLevelData().size(); i++) {
      for (unsigned int j = 0; j < backgroundMap.getLevelData()[i].size(); j++) {
         int entityID = backgroundMap.getLevelData()[i][j];
         switch (entityID) {
            case -1:
               break;
            default:
               Entity* entity(world->create());

               entity->addComponent<PositionComponent>(
                   Vector2f(j * SCALED_CUBE_SIZE, i * SCALED_CUBE_SIZE),
                   Vector2i(SCALED_CUBE_SIZE, SCALED_CUBE_SIZE));

               entity->addComponent<TextureComponent>(blockTexture, false, false);

               entity->addComponent<SpritesheetComponent>(
                   ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE, 1, 1, 1, ORIGINAL_CUBE_SIZE,
                   ORIGINAL_CUBE_SIZE, Map::BlockIDCoordinates.at(entityID));

               entity->addComponent<BackgroundComponent>();
               break;
         }
      }
   }
}
