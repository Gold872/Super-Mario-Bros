#include "systems/ScoreSystem.h"

#include "Constants.h"
#include "ECS/Components.h"
#include "Map.h"

#include <cmath>
#include <memory>
#include <string>

ScoreSystem::ScoreSystem(GameScene* scene) {
   this->scene = scene;
}

Entity* ScoreSystem::createFloatingText(World* world, Entity* originalEntity, std::string text) {
   auto* originalPosition = originalEntity->getComponent<PositionComponent>();

   Entity* scoreText(world->create());
   scoreText->addComponent<PositionComponent>(
       Vector2f(originalPosition->getCenterX(), originalPosition->getTop() - 4), Vector2i());
   scoreText->addComponent<MovingComponent>(0, -1, 0, 0);
   scoreText->addComponent<TextComponent>(text, 10, true);
   scoreText->addComponent<FloatingTextComponent>();
   scoreText->addComponent<DestroyDelayedComponent>(35);

   return scoreText;
}

void ScoreSystem::onAddedToWorld(World* world) {
   int paddingW = 44;
   int paddingH = 16;
   int spacingH = 4;
   int textHeight = 16;

   int availableWidth = SCREEN_WIDTH - paddingW;
   int columns = 4;
   float columnWidth = (float)availableWidth / (float)columns;

   {
      Entity* marioText(world->create());

      marioText->addComponent<PositionComponent>(Vector2f(paddingW, paddingH), Vector2i());

      marioText->addComponent<TextComponent>("MARIO", 16);
   }
   /* ************************************************************** */
   {
      scoreEntity = world->create();

      scoreEntity->addComponent<PositionComponent>(
          Vector2f(paddingW, paddingH + textHeight + spacingH), Vector2i());

      scoreEntity->addComponent<TextComponent>("000000", 16);
   }
   /* ************************************************************** */
   {
      Entity* coinIcon(world->create());

      coinIcon->addComponent<PositionComponent>(
          Vector2f(paddingW + columnWidth, paddingH + textHeight + spacingH + 2),
          Vector2i(SCALED_CUBE_SIZE, SCALED_CUBE_SIZE));

      coinIcon->addComponent<TextureComponent>(
          scene->blockTexture, ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE, 1, 1, 1, ORIGINAL_CUBE_SIZE,
          ORIGINAL_CUBE_SIZE, Map::BlockIDCoordinates.at(754), false, false);

      coinIcon->addComponent<AnimationComponent>(std::vector<int>{754, 755, 756, 757}, 4, 8,
                                                 Map::BlockIDCoordinates);

      coinIcon->addComponent<PausedAnimationComponent>(0, 25);

      coinIcon->addComponent<IconComponent>();
   }
   /* ************************************************************** */
   {
      coinsEntity = world->create();

      coinsEntity->addComponent<PositionComponent>(
          Vector2f(paddingW + columnWidth + 18, paddingH + textHeight + spacingH), Vector2i());

      coinsEntity->addComponent<TextComponent>("x00", 16);
   }
   /* ************************************************************** */
   {
      Entity* worldEntity(world->create());
      worldEntity->addComponent<PositionComponent>(Vector2f(paddingW + (2 * columnWidth), paddingH),
                                                   Vector2i());
      worldEntity->addComponent<TextComponent>("WORLD", 16);
   }
   /* ************************************************************** */
   {
      worldNumberEntity = world->create();

      worldNumberEntity->addComponent<PositionComponent>(
          Vector2f(paddingW + (2 * columnWidth), paddingH + textHeight + spacingH), Vector2i());

      worldNumberEntity->addComponent<TextComponent>(
          std::to_string(scene->level) + "-" + std::to_string(scene->subLevel), 16);
   }
   /* ************************************************************** */
   {
      Entity* timeEntity(world->create());

      timeEntity->addComponent<PositionComponent>(Vector2f(paddingW + (3 * columnWidth), paddingH),
                                                  Vector2i());

      timeEntity->addComponent<TextComponent>("TIME", 16);
   }
   /* ************************************************************** */
   {
      timerEntity = world->create();

      timerEntity->addComponent<PositionComponent>(
          Vector2f(paddingW + (3 * columnWidth) + 4, paddingH + textHeight + spacingH), Vector2i());

      timerEntity->addComponent<TextComponent>(std::to_string(gameTime), 16);
   }
   /* ************************************************************** */
   {
      worldNumberTransition = world->create();

      worldNumberTransition->addComponent<PositionComponent>(
          Vector2f(paddingW + (columnWidth * 1.5), paddingH * 10 + textHeight + spacingH),
          Vector2i());

      auto* worldNumberText = worldNumberTransition->addComponent<TextComponent>(
          "WORLD " + std::to_string(scene->level) + "-" + std::to_string(scene->subLevel), 16);

      worldNumberText->setVisible(false);
   }
   /* ************************************************************** */
   {
      marioIcon = world->create();

      marioIcon->addComponent<PositionComponent>(
          Vector2f(paddingW + (columnWidth * 1.5), paddingH * 12 + textHeight + spacingH),
          Vector2i(SCALED_CUBE_SIZE, SCALED_CUBE_SIZE));

      std::shared_ptr<SDL_Texture> playerTexture =
          world->findFirst<PlayerComponent, TextureComponent>()
              ->getComponent<TextureComponent>()
              ->getTexture();

      auto* marioIconTexture = marioIcon->addComponent<TextureComponent>(
          playerTexture, ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE, 1, 9, 0, ORIGINAL_CUBE_SIZE,
          ORIGINAL_CUBE_SIZE, Map::PlayerIDCoordinates.at(0), false, false);

      marioIcon->addComponent<IconComponent>();

      marioIconTexture->setVisible(false);
   }
   /* ************************************************************** */
   {
      livesText = world->create();

      livesText->addComponent<PositionComponent>(
          Vector2f(paddingW * 2 + (columnWidth * 1.5), paddingH * 12 + textHeight + spacingH * 3),
          Vector2i());

      auto* livesNumberText =
          livesText->addComponent<TextComponent>(" x  " + std::to_string(lives), 16);

      livesNumberText->setVisible(false);
   }
}

void ScoreSystem::tick(World* world) {
   bool changeScore = false;
   bool changeCoin = false;
   bool changeTime = false;

   world->find<CreateFloatingTextComponent>([=](Entity* entity) {
      auto* floatingText = entity->getComponent<CreateFloatingTextComponent>();

      createFloatingText(world, floatingText->originalEntity, floatingText->text);

      world->destroy(entity);
   });

   world->find<AddScoreComponent>([&](Entity* entity) {
      auto* score = entity->getComponent<AddScoreComponent>();

      if (score->score > 0) {
         totalScore += score->score;
         changeScore = true;
      }

      if (score->addCoin) {
         coins++;
         changeCoin = true;
      }
      world->destroy(entity);
   });

   if (timerRunning) {
      time -= 2;
      if (time % MAX_FPS == 0) {
         gameTime--;
         changeTime = true;
      }
   }

   if (changeScore) {
      scoreEntity->getComponent<TextComponent>()->destroyTexture();

      std::string scoreString = std::to_string(totalScore);
      std::string finalString = std::string{};

      for (int zeros = 6 - scoreString.length(); zeros > 0; zeros--) {
         finalString += '0';
      }

      finalString += scoreString;

      scoreEntity->getComponent<TextComponent>()->text = finalString;
   }

   if (changeCoin) {
      coinsEntity->getComponent<TextComponent>()->destroyTexture();

      std::string coinString = std::to_string(coins);
      std::string finalString = std::string{};

      for (int i = 2 - coinString.length(); i > 0; i--) {
         finalString += '0';
      }
      finalString += coinString;
      coinsEntity->getComponent<TextComponent>()->text = "x" + finalString;
   }

   if (changeTime) {
      timerEntity->getComponent<TextComponent>()->destroyTexture();

      std::string timeString = std::to_string(gameTime);
      std::string finalString = std::string{};

      for (int zeros = 3 - timeString.length(); zeros > 0; zeros--) {
         finalString += '0';
      }
      finalString += timeString;

      timerEntity->getComponent<TextComponent>()->text = finalString;
   }
}

void ScoreSystem::reset() {
   timerEntity->getComponent<TextComponent>()->destroyTexture();
   gameTime = 400;
   time = 400 * MAX_FPS;
   timerEntity->getComponent<TextComponent>()->text = std::to_string(gameTime);

   worldNumberEntity->getComponent<TextComponent>()->destroyTexture();
   worldNumberEntity->getComponent<TextComponent>()->text =
       std::to_string(scene->level) + "-" + std::to_string(scene->subLevel);
}

void ScoreSystem::startTimer() {
   timerEntity->getComponent<TextComponent>()->destroyTexture();
   timerRunning = true;
}

void ScoreSystem::stopTimer() {
   timerRunning = false;
}

void ScoreSystem::decreaseLives() {
   lives--;
   livesText->getComponent<TextComponent>()->destroyTexture();
   livesText->getComponent<TextComponent>()->text = " x  " + std::to_string(lives);
}

void ScoreSystem::scoreCountdown(World* world) {
   gameTime--;

   timerEntity->getComponent<TextComponent>()->destroyTexture();

   std::string timeString = std::to_string(gameTime);
   std::string finalString = std::string{};

   for (int zeros = 3 - timeString.length(); zeros > 0; zeros--) {
      finalString += '0';
   }
   finalString += timeString;

   timerEntity->getComponent<TextComponent>()->text = finalString;

   Entity* timerTickSound(world->create());
   timerTickSound->addComponent<SoundComponent>(SoundID::TIMER_TICK);

   Entity* addScore(world->create());
   addScore->addComponent<AddScoreComponent>(100);
}

bool ScoreSystem::scoreCountFinished() {
   return gameTime <= 0;
}

void ScoreSystem::showTransitionEntities() {
   worldNumberTransition->getComponent<TextComponent>()->destroyTexture();
   worldNumberTransition->getComponent<TextComponent>()->text =
       "WORLD " + std::to_string(scene->level) + "-" + std::to_string(scene->subLevel);
   worldNumberTransition->getComponent<TextComponent>()->setVisible(true);

   marioIcon->getComponent<TextureComponent>()->setVisible(true);

   livesText->getComponent<TextComponent>()->setVisible(true);
}

void ScoreSystem::hideTransitionEntities() {
   worldNumberTransition->getComponent<TextComponent>()->setVisible(false);

   marioIcon->getComponent<TextureComponent>()->setVisible(false);

   livesText->getComponent<TextComponent>()->setVisible(false);
}
