#pragma once

#include "ECS/Components.h"
#include "ECS/ECS.h"
#include "scenes/GameScene.h"

class GameScene;

class PlayerSystem : public System {
  public:
   enum Animation_State
   {
      STANDING,
      WALKING,
      RUNNING,
      DRIFTING,
      DUCKING,
      JUMPING,
      SWIMMING,
      LAUNCH_FIREBALL,
      CLIMBING,  // Climbing a vine
      SLIDING,   // Sliding down a flag
      GAMEOVER
   };

   enum class GrowType
   {
      ONEUP,
      MUSHROOM,
      SUPER_STAR,
      FIRE_FLOWER
   };

   Animation_State currentState = STANDING;

   PlayerSystem(GameScene* scene);

   ~PlayerSystem() override = default;

   void tick(World* world) override;

   void handleEvent(SDL_Event& event) override;

   void handleEvent(const Uint8* keystates) override;

   void onAddedToWorld(World* world) override;

   void setUnderwater(bool val);

   void reset();

   static bool isInputEnabled() {
      return inputEnabled;
   }

   static bool isGameStart() {
      return inGameStart;
   }

   static void enableInput(bool val) {
      inputEnabled = val;
   }

   static void setGameStart(bool val) {
      inGameStart = val;
   }

  private:
   bool isSmallMario();

   bool isSuperMario();

   bool isFireMario();

   void onGameOver(World* world, bool outOfBounds = false);

   void setState(Animation_State newState);

   void grow(World* world, GrowType growType);

   void shrink(World* world);

   void createBlockDebris(World* world, Entity* block);

   void updateGroundVelocity(World* world);

   void updateAirVelocity();

   void updateWaterVelocity(World* world);

   void updateCamera();

   void checkEnemyCollisions(World* world);

   Entity* createFireball(World* world);

   int xDir = 0;
   int left = 0;
   int right = 0;
   int jump = 0;
   int duck = 0;
   int launchFireball = 0;

   int holdFireballTexture = 0;
   int jumpHeldTime = 0;

   static bool inputEnabled;
   static bool inGameStart;

   Entity* mario;
   GameScene* scene;
};
