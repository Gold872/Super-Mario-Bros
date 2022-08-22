#include "scenes/GameOverScene.h"

#include "Camera.h"
#include "Constants.h"
#include "ECS/Components.h"
#include "SoundManager.h"
#include "TextureManager.h"
#include "systems/RenderSystem.h"
#include "systems/SoundSystem.h"

GameOverScene::GameOverScene() {
   TextureManager::Get().SetBackgroundColor(BackgroundColor::BLACK);
   Camera::Get().setCameraX(0);
   Camera::Get().setCameraY(0);

   world->registerSystem<SoundSystem>();
   world->registerSystem<RenderSystem>();

   gameOverText = world->create();

   gameOverText->addComponent<PositionComponent>(
       Vector2f(10 * SCALED_CUBE_SIZE, 6.5 * SCALED_CUBE_SIZE), Vector2i());

   gameOverText->addComponent<TextComponent>("GAME OVER", 20);

   Entity* gameOverSound(world->create());
   gameOverSound->addComponent<SoundComponent>(SoundID::GAME_OVER);
}

void GameOverScene::update() {
   world->tick();
   timer++;
}

bool GameOverScene::isFinished() {
   return timer == MAX_FPS * 4.5;
}
