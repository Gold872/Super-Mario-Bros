#pragma once

#include "Constants.h"
#include "ECS.h"
//#include "Level.h"
#include "Map.h"
#include "Math.h"
#include "SoundManager.h"
#include "TextureManager.h"

#include <SDL2/SDL.h>

#include <cmath>
#include <functional>
#include <iostream>
#include <memory>
#include <unordered_map>
struct PositionComponent : public Component {
   PositionComponent(Vector2f position, Vector2i scale) : position{position}, scale{scale} {
      hitbox = {0, 0, scale.x, scale.y};
   };
   PositionComponent(Vector2f position, Vector2i scale, SDL_Rect hitbox)
       : position{position}, scale{scale}, hitbox{hitbox} {};

   Vector2f position;
   Vector2i scale;

   SDL_Rect hitbox;

   float getRight() {
      return position.x + scale.x;
   }

   float getLeft() {
      return position.x;
   }

   float getTop() {
      return position.y;
   }

   float getBottom() {
      return position.y + scale.y;
   }

   float getCenterX() {
      return position.x + scale.x / 2.0f;
   }

   float getCenterY() {
      return position.y + scale.y / 2.0f;
   }

   void setTop(float value) {
      position.y = value;
   }

   void setBottom(float value) {
      position.y = value - scale.y;
   }

   void setLeft(float value) {
      position.x = value;
   }

   void setRight(float value) {
      position.x = value - scale.x;
   }
};

struct TextureComponent : public Component {
   TextureComponent(std::shared_ptr<SDL_Texture> texture, int entityWidth, int entityHeight,
                    int xOffset, int yOffset, int gridGapWidth, int gridWidth, int gridHeight,
                    Vector2i spritesheetCoordinates, bool horizontalFlip = false,
                    bool verticalFlip = false)
       : texture{texture},
         entityWidth{entityWidth},
         entityHeight{entityHeight},
         xOffset{xOffset},
         yOffset{yOffset},
         gridGapWidth{gridGapWidth},
         gridWidth{gridWidth},
         gridHeight{gridHeight},
         spritesheetCoordinates{spritesheetCoordinates},
         horizontalFlipped{horizontalFlip},
         verticalFlipped{verticalFlip} {
      this->spritesheetCoordinates = spritesheetCoordinates;

      sourceRect.x = xOffset + ((spritesheetCoordinates.x) * gridGapWidth) +
                     ((spritesheetCoordinates.x) * gridWidth);
      sourceRect.y = yOffset + ((spritesheetCoordinates.y) * gridGapWidth) +
                     ((spritesheetCoordinates.y) * gridHeight);
      sourceRect.w = entityWidth;
      sourceRect.h = entityHeight;
   };

   std::shared_ptr<SDL_Texture> getTexture() {
      return texture;
   }

   void setSpritesheetCoordinates(Vector2i coords) {
      spritesheetCoordinates = coords;
      sourceRect.x = xOffset + ((spritesheetCoordinates.x) * gridGapWidth) +
                     ((spritesheetCoordinates.x) * gridWidth);
      sourceRect.y = yOffset + ((spritesheetCoordinates.y) * gridGapWidth) +
                     ((spritesheetCoordinates.y) * gridHeight);
   }

   void setSpritesheetXCoordinate(int xCoordinate) {
      spritesheetCoordinates.x = xCoordinate;

      sourceRect.x = xOffset + ((spritesheetCoordinates.x) * gridGapWidth) +
                     ((spritesheetCoordinates.x) * gridWidth);
      sourceRect.y = yOffset + ((spritesheetCoordinates.y) * gridGapWidth) +
                     ((spritesheetCoordinates.y) * gridHeight);
   }

   void setEntityHeight(int newEntityHeight) {
      entityHeight = newEntityHeight;

      sourceRect.h = entityHeight;
   }

   void setGridHeight(int newGridHeight) {
      gridHeight = newGridHeight;

      sourceRect.y = yOffset + ((spritesheetCoordinates.y) * gridGapWidth) +
                     ((spritesheetCoordinates.y) * gridHeight);
   }

   Vector2i getSpritesheetCoordinates() {
      return spritesheetCoordinates;
   }

   SDL_Rect getSourceRect() {
      return sourceRect;
   }

   void setHorizontalFlipped(bool val) {
      horizontalFlipped = val;
   }

   void setVerticalFlipped(bool val) {
      verticalFlipped = val;
   }

   void setVisible(bool val) {
      visible = val;
   }

   bool isHorizontalFlipped() {
      return horizontalFlipped;
   }

   bool isVerticalFlipped() {
      return verticalFlipped;
   }

   bool isVisible() {
      return visible;
   }

  private:
   SDL_Rect sourceRect;

   std::shared_ptr<SDL_Texture> texture;
   int entityWidth;
   int entityHeight;
   int xOffset;
   int yOffset;
   int gridGapWidth;
   int gridWidth;
   int gridHeight;
   Vector2i spritesheetCoordinates;
   bool horizontalFlipped = false;
   bool verticalFlipped = false;
   bool visible = true;
};

struct TextComponent : public Component {
   TextComponent(std::string text, unsigned int fontSize, bool followCamera = false,
                 bool visible = true)
       : text{text}, fontSize{fontSize}, followCamera{followCamera}, visible{visible} {}

   ~TextComponent() {
      if (texture != nullptr) {
         SDL_DestroyTexture(texture);
      }
   }

   void destroyTexture() {
      SDL_DestroyTexture(texture);
      texture = nullptr;
   }

   bool isVisible() {
      return visible;
   }

   void setVisible(bool val) {
      visible = val;
   }

   std::string text;
   unsigned int fontSize;
   bool followCamera;
   bool visible;
   SDL_Texture* texture = nullptr;
};

/* SOUND COMPONENTS */
struct SoundComponent : public Component {
   SoundComponent(SoundID sound) : soundID{sound} {}
   SoundID soundID;
};

struct MusicComponent : public Component {
   MusicComponent(MusicID music) : musicID{music} {}

   MusicID musicID;
};

/* ANIMATION COMPONENTS */
struct AnimationComponent : public Component {
   AnimationComponent(std::vector<int> frameIDS, int frameCount, int framesPerSecond,
                      std::unordered_map<int, Vector2i>& coordinateSupplier, bool repeated = true)
       : frameIDS{frameIDS},
         frameCount{frameCount},
         framesPerSecond{framesPerSecond},
         coordinateSupplier{coordinateSupplier},
         repeated{repeated} {
      playing = true;
      frameDelay = (int)std::round((float)MAX_FPS / (float)framesPerSecond);
   }

   void setPlaying(bool val) {
      playing = val;
   }

   void setFramesPerSecond(int fps) {
      framesPerSecond = fps;
      frameDelay = (int)std::round((float)MAX_FPS / (float)framesPerSecond);
   }

   bool playing;
   std::vector<int> frameIDS;
   int frameCount;
   int framesPerSecond;
   int frameDelay;
   int frameTimer = 0;
   std::unordered_map<int, Vector2i>& coordinateSupplier;
   bool repeated;
   int currentFrame = 0;
};

struct PausedAnimationComponent : public Component {
   PausedAnimationComponent(int pauseFrame, int pauseLength)
       : pauseFrame{pauseFrame}, pauseLength{pauseLength} {}

   void pause(int pauseLength) {
      pauseTimer = pauseLength;
   }

   int pauseFrame;
   int pauseLength;
   int pauseTimer;
};

struct EndingBlinkComponent : public Component {
   EndingBlinkComponent(int blinkSpeed, int time) : blinkSpeed{blinkSpeed}, time{time} {}

   const int blinkSpeed;
   int current = 0;
   int time;
};

/* CALLBACK COMPONENTS */
struct CallbackComponent : public Component {
   CallbackComponent() = default;
   CallbackComponent(std::function<void(Entity*)> callback, int time)
       : callback{callback}, time{time} {}

   std::function<void(Entity*)> callback;
   int time;
};

struct DestroyDelayedComponent : public Component {
   DestroyDelayedComponent(int time) : time{time} {}

   int time;
};

struct TimerComponent : public Component {
   TimerComponent(std::function<void(Entity*)> onExecute, int delay)
       : onExecute{onExecute}, delay{delay} {
      time = delay;
   }

   std::function<void(Entity*)> onExecute;
   int delay;
   int time;
};

struct WaitUntilComponent : public Component {
   WaitUntilComponent() = default;
   WaitUntilComponent(std::function<bool(Entity*)> condition, std::function<void(Entity*)> doAfter)
       : condition{condition}, doAfter{doAfter} {}

   std::function<bool(Entity*)> condition;
   std::function<void(Entity*)> doAfter;
};

/* UNCATEGORiZED */
struct BlockBumpComponent : public Component {
   BlockBumpComponent() = default;
   BlockBumpComponent(std::vector<int> yChanges) : yChanges{yChanges} {}

   std::vector<int> yChanges;
   unsigned yChangeIndex = 0;
};

/* TEXTURE CLASSIFICATION COMPONENTS */

struct AboveForegroundComponent : public Component {
};  // For warp pipes and things that have to hide the player

struct ForegroundComponent : public Component {};

struct BackgroundComponent : public Component {};

struct UndergroundComponent : public Component {};

struct IconComponent : public Component {};

struct FloatingTextComponent : public Component {};

/* BLOCK TYPES */

struct TileComponent : public Component {};

struct DestructibleComponent : public Component {
   DestructibleComponent() = default;
   DestructibleComponent(Vector2i debrisCoordinates) : debrisCoordinates{debrisCoordinates} {}

   Vector2i debrisCoordinates;
};

struct BumpableComponent : public Component {};

enum class Direction
{
   NONE,
   UP,
   DOWN,
   LEFT,
   RIGHT
};

enum class PlatformMotionType
{
   NONE,
   ONE_DIRECTION_REPEATED,
   ONE_DIRECTION_CONTINUOUS,  // Continuously moving in one direction
   BACK_AND_FORTH,            // Moves back and forth
   GRAVITY                    // Affected by Gravity when mario stands on it
};

enum class LevelType;

struct WarpPipeComponent : public Component {
   WarpPipeComponent(Vector2i playerLocation, Vector2i cameraLocation, Direction inDirection,
                     Direction outDirection, bool cameraFreeze, BackgroundColor backgroundColor,
                     LevelType levelType, Vector2i newLevel)
       : playerLocation{playerLocation},
         cameraLocation{cameraLocation},
         inDirection{inDirection},
         outDirection{outDirection},
         cameraFreeze{cameraFreeze},
         backgroundColor{backgroundColor},
         levelType{levelType},
         newLevel{newLevel} {}

   Vector2i playerLocation;
   Vector2i cameraLocation;

   Direction inDirection;
   Direction outDirection;

   bool cameraFreeze;

   BackgroundColor backgroundColor;

   LevelType levelType;

   Vector2i newLevel;
};

struct MovingPlatformComponent : public Component {
   MovingPlatformComponent(PlatformMotionType motionType, Direction movingDirection,
                           Vector2i minMax = Vector2i(0, 0))
       : motionType{motionType},
         movingDirection{movingDirection},
         minPoint{(float)minMax.x},
         maxPoint{(float)minMax.y} {}

   PlatformMotionType motionType;
   Direction movingDirection;

   float minPoint;
   float maxPoint;

   std::function<float(float position, float distanceTravel)> calculateVelocity =
       [](float position, float distanceTravel) {
          return 2 * std::pow(M_E, -((std::pow(position - (1.9 * distanceTravel), 2)) /
                                     (2 * std::pow(distanceTravel, 2))));
       };

   std::vector<Entity*> connectedParts;
};

enum class RotationDirection
{
   NONE,
   CLOCKWISE,
   COUNTER_CLOCKWISE,
};

struct FireBarComponent : public Component {
   FireBarComponent() = default;
   FireBarComponent(Vector2f rotationPoint, float barPosition, float startAngle,
                    RotationDirection direction)
       : pointOfRotation{rotationPoint},
         barPosition{barPosition},
         barAngle{startAngle},
         direction{direction} {}

   Vector2f pointOfRotation;

   float barPosition;  // Length away from the point of rotation

   float barAngle;

   RotationDirection direction;

   std::function<float(float angle)> calculateYPosition = [=](float angle) {
      float angleRadians = angle * (M_PI / 180);
      return std::sin(angleRadians) * barPosition;
   };

   std::function<float(float angle)> calculateXPosition = [=](float angle) {
      float angleRadians = angle * (M_PI / 180);
      return std::cos(angleRadians) * barPosition;
   };
};

struct BridgeComponent : public Component {
   BridgeComponent() = default;

   std::vector<Entity*> connectedBridgeParts;
};

struct BridgeChainComponent : public Component {};

struct FlagComponent : public Component {};

struct FlagPoleComponent : public Component {};

struct AxeComponent : public Component {};

struct ParticleComponent : public Component {};

enum class MysteryBoxType
{
   NONE,
   MUSHROOM,
   COINS,
   SUPER_STAR,
   RANDOM
};

struct MysteryBoxComponent : public Component {
   MysteryBoxComponent() = default;
   MysteryBoxComponent(MysteryBoxType type = MysteryBoxType::NONE) : boxType{type} {}
   MysteryBoxComponent(MysteryBoxType type, std::function<void(Entity*)> dispensed)
       : boxType{type}, whenDispensed{dispensed} {}

   MysteryBoxType boxType;
   Vector2i deactivatedCoordinates;
   std::function<void(Entity*)> whenDispensed = [&](Entity* entity) {};
};

struct CollectibleDispenserComponent : public Component {};

enum class CollectibleType
{
   NONE,
   MUSHROOM,
   SUPER_STAR,
   FIRE_FLOWER,
   COIN,
};

struct CollectibleComponent : public Component {
   CollectibleComponent() = default;
   CollectibleComponent(CollectibleType type = CollectibleType::NONE) : collectibleType{type} {}

   CollectibleType collectibleType;
};

/* PHYSICS COMPONENTS */
struct MovingComponent : public Component {
   MovingComponent(float velocityX, float velocityY, float accelerationX = 0.000,
                   float accelerationY = 0.000)
       : velocityX{velocityX},
         velocityY{velocityY},
         accelerationX{accelerationX},
         accelerationY{accelerationY} {}

   float velocityX, velocityY;
   float accelerationX, accelerationY;
};

struct CollisionExemptComponent : public Component {};

struct FrictionExemptComponent : public Component {};

struct MoveOutsideCameraComponent : public Component {};

struct GravityComponent : public Component {};

/* COLLISION COMPONENTS */

enum class CollisionDirection
{
   NONE,
   TOP,
   BOTTOM,
   LEFT,
   RIGHT
};

struct TopCollisionComponent : public Component {};

struct BottomCollisionComponent : public Component {};

struct LeftCollisionComponent : public Component {};

struct RightCollisionComponent : public Component {};

struct EnemyCollisionComponent : public Component {};

/* PLAYER COMPONENTS */
struct PlayerComponent : public Component {};

struct SuperMarioComponent : public Component {};

struct FireMarioComponent : public Component {};

struct FrozenComponent : public Component {};

struct SuperStarComponent : public Component {};

/* ENEMY COMPONENTS */
enum class EnemyType
{
   NONE,
   GOOMBA,
   KOOPA,
   FLYING_KOOPA,
   KOOPA_SHELL,
   PIRANHA_PLANT,
   BOWSER,
   FIRE_BAR,
};

enum class ProjectileType
{
   NONE,
   FIREBALL,
   OTHER
};

struct EnemyComponent : public Component {
   EnemyComponent(EnemyType type) : enemyType{type} {}

   EnemyType enemyType;
};

struct BowserComponent : public Component {
   BowserComponent(std::vector<std::function<void(Entity*, int number)>> attacks,
                   std::vector<std::function<void(Entity*)>> movements)
       : bowserAttacks{attacks}, bowserMovements{movements} {}

   std::vector<std::function<void(Entity*, int number)>> bowserAttacks;
   std::vector<std::function<void(Entity*)>> bowserMovements;

   float distanceMoved = 0;

   int lastAttackTime = 0;
   int lastMoveTime = 0;
   int lastStopTime = 0;
   int lastJumpTime = 0;

   int currentMoveIndex = 0;

   Direction lastMoveDirection = Direction::NONE;
};

struct DeadComponent : public Component {};

struct CrushableComponent : public Component {
   CrushableComponent(std::function<void(Entity*)> whenCrushed) : whenCrushed{whenCrushed} {}

   std::function<void(Entity*)> whenCrushed;
};

struct CrushedComponent : public Component {};

struct ProjectileComponent : public Component {
   ProjectileComponent(ProjectileType type) : projectileType{type} {}

   ProjectileType projectileType;
};

// When the enemy is killed from being above a destroyed block or hit by the player with superstar
struct EnemyDestroyedComponent : public Component {};

/* SCORING */
struct AddScoreComponent : public Component {
   AddScoreComponent(int score, bool addCoin = false) : score{score}, addCoin{addCoin} {}

   int score;
   bool addCoin;
};

struct CreateFloatingTextComponent : public Component {
   CreateFloatingTextComponent(Entity* originalEntity, std::string text)
       : originalEntity{originalEntity}, text{text} {}

   Entity* originalEntity;
   std::string text;
};
