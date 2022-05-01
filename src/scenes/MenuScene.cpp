#include "scenes/MenuScene.h"

#include "Constants.h"
#include "ECS/Components.h"
#include "ECS/ECS.h"
#include "TextureManager.h"
#include "systems/Systems.h"

#include <SDL2/SDL.h>

#include <memory>

MenuScene::MenuScene() {
   Map::loadBlockIDS();

   menuSystem = world->registerSystem<MenuSystem>();
   world->registerSystem<RenderSystem>();

   createMenuEntities();
}

void MenuScene::handleEvents(SDL_Event& event) {
   if (event.type == SDL_KEYDOWN) {
      switch (event.key.keysym.scancode) {
         case SDL_SCANCODE_RETURN:
         case SDL_SCANCODE_SPACE:
            finished = true;
            break;
         default:
            break;
      }
   }
   world->handleEvent(event);
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

               entity->addComponent<TextureComponent>(
                   blockTexture, ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE, 1, 1, 1,
                   ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE, Map::BlockIDCoordinates.at(entityID),
                   false, false);

               entity->addComponent<BackgroundComponent>();
               break;
         }
      }
   }
}
