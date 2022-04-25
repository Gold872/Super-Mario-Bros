#include "scenes/MenuScene.h"

#include "ECS/Components.h"
#include "ECS/ECS.h"
#include "systems/Systems.h"

MenuScene::MenuScene() {
   world->registerSystem<RenderSystem>();
}

void MenuScene::createMenuEntities() {

}
