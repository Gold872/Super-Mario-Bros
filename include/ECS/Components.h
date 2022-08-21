#pragma once

#include "Constants.h"
#include "ECS.h"
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

   void setCenterX(float value) {
      position.x = value - scale.x / 2.0f;
   }

   void setCenterY(float value) {
      position.y = value - scale.y / 2.0f;
   }
};

struct TextureComponent : public Component {
   TextureComponent(std::shared_ptr<SDL_Texture> texture, bool horizontalFlip = false,
                    bool verticalFlip = false)
       : texture{texture}, horizontalFlipped{horizontalFlip}, verticalFlipped{verticalFlip} {};

   std::shared_ptr<SDL_Texture> getTexture() {
      return texture;
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
   bool horizontalFlipped = false;
   bool verticalFlipped = false;
   bool visible = true;
};

struct SpritesheetComponent : public Component {
   SpritesheetComponent(int entityWidth, int entityHeight, int xOffset, int yOffset,
                        int gridGapWidth, int gridWidth, int gridHeight,
                        Vector2i spritesheetCoordinates)
       : entityWidth{entityWidth},
         entityHeight{entityHeight},
         xOffset{xOffset},
         yOffset{yOffset},
         gridGapWidth{gridGapWidth},
         gridWidth{gridWidth},
         gridHeight{gridHeight},
         spritesheetCoordinates{spritesheetCoordinates} {
      this->spritesheetCoordinates = spritesheetCoordinates;

      sourceRect.x = xOffset + ((spritesheetCoordinates.x) * gridGapWidth) +
                     ((spritesheetCoordinates.x) * gridWidth);
      sourceRect.y = yOffset + ((spritesheetCoordinates.y) * gridGapWidth) +
                     ((spritesheetCoordinates.y) * gridHeight);
      sourceRect.w = entityWidth;
      sourceRect.h = entityHeight;
   };

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

   SDL_Rect& getSourceRect() {
      return sourceRect;
   }

  private:
   SDL_Rect sourceRect;

   int entityWidth;
   int entityHeight;
   int xOffset;
   int yOffset;
   int gridGapWidth;
   int gridWidth;
   int gridHeight;
   Vector2i spritesheetCoordinates;
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
   AnimationComponent(std::vector<int> frameIDS, int framesPerSecond,
                      std::unordered_map<int, Vector2i>& coordinateSupplier, bool repeated = true)
       : frameIDS{frameIDS},
         frameCount{(int)frameIDS.size()},
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

   void reset() {
      time = delay;
   }
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

struct InvisibleBlockComponent : public Component {};

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
   ONE_DIRECTION_REPEATED,    // Moves in one direction, but goes to min point when it reaches max
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

   float calculateVelocity(float position, float distanceTravel) {
      return 2 * std::exp(-((std::pow(position - (1.9 * distanceTravel), 2)) /
                            (2 * std::pow(distanceTravel, 2))));
   }
};

struct PlatformLevelComponent : public Component {
   PlatformLevelComponent(Entity* other, Entity* pulleyLine, int pulleyHeight)
       : otherPlatform{other}, pulleyLine{pulleyLine}, pulleyHeight{pulleyHeight} {}

   Entity* getOtherPlatform() {
      return otherPlatform;
   }

   Entity* otherPlatform;

   Entity* pulleyLine;
   std::vector<Entity*> pulleyLines;

   int pulleyHeight;
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

   float calculateYPosition(float angle) {
      float angleRadians = angle * (M_PI / 180);
      return std::sin(angleRadians) * barPosition;
   }

   float calculateXPosition(float angle) {
      float angleRadians = angle * (M_PI / 180);
      return std::cos(angleRadians) * barPosition;
   }
};

struct BridgeComponent : public Component {
   BridgeComponent() = default;

   std::vector<Entity*> connectedBridgeParts;
};

struct BridgeChainComponent : public Component {};

struct TrampolineComponent : public Component {
   TrampolineComponent(Entity* bottomEntity, int topIDS[], int bottomIDS[])
       : bottomEntity{bottomEntity} {
      topExtendedID = topIDS[0];
      topMediumRetractedID = topIDS[1];
      topRetractedID = topIDS[2];

      bottomExtendedID = bottomIDS[0];
      bottomMediumRetractedID = bottomIDS[1];
      bottomRetractedID = bottomIDS[2];
   }

   Entity* bottomEntity;

   int currentSequenceIndex = 0;

   int topExtendedID;
   int topMediumRetractedID;
   int topRetractedID;

   int bottomExtendedID;
   int bottomMediumRetractedID;
   int bottomRetractedID;
};

struct FlagComponent : public Component {};

struct FlagPoleComponent : public Component {};

struct VineComponent : public Component {
   VineComponent(Vector2i coordinates, Vector2i teleport, Vector2i camera, int resetValue,
                 Vector2i resetLocation, int newCameraMax, BackgroundColor newBackgroundColor,
                 LevelType newLevelType, std::vector<Entity*>& vineParts)
       : coordinates{coordinates},
         teleportCoordinates{teleport},
         cameraCoordinates{camera},
         resetYValue{resetValue},
         resetTeleportLocation{resetLocation},
         newCameraMax{newCameraMax},
         newBackgroundColor{newBackgroundColor},
         newLevelType{newLevelType},
         vineParts{vineParts} {}

   Vector2i coordinates;
   Vector2i teleportCoordinates;
   Vector2i cameraCoordinates;
   int resetYValue;
   Vector2i resetTeleportLocation;
   int newCameraMax;
   BackgroundColor newBackgroundColor;
   LevelType newLevelType;
   std::vector<Entity*>& vineParts;
};

struct AxeComponent : public Component {};

struct ParticleComponent : public Component {};

enum class MysteryBoxType
{
   NONE,
   MUSHROOM,
   COINS,
   SUPER_STAR,
   ONE_UP,
   VINES
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

enum class CollectibleType
{
   NONE,
   MUSHROOM,
   SUPER_STAR,
   FIRE_FLOWER,
   COIN,
   ONE_UP
};

struct CollectibleComponent : public Component {
   CollectibleComponent() = default;
   CollectibleComponent(CollectibleType type = CollectibleType::NONE) : collectibleType{type} {}

   CollectibleType collectibleType;
};

/* PHYSICS COMPONENTS */
struct MovingComponent : public Component {
   MovingComponent(Vector2f velocity, Vector2f acceleration)
       : velocity{velocity}, acceleration{acceleration} {}

   Vector2f velocity;
   Vector2f acceleration;
};

struct CollisionExemptComponent : public Component {};

struct FrictionExemptComponent : public Component {};

struct MoveOutsideCameraComponent : public Component {};

struct DestroyOutsideCameraComponent : public Component {};

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

/* PLAYER COMPONENTS */
enum class PlayerState
{
   SMALL_MARIO,
   SUPER_MARIO,
   FIRE_MARIO
};

struct PlayerComponent : public Component {
   PlayerState playerState = PlayerState::SMALL_MARIO;
   bool superStar = false;
};

struct FrozenComponent : public Component {};

/* ENEMY COMPONENTS */
enum class EnemyType
{
   NONE,
   GOOMBA,
   KOOPA,
   KOOPA_PARATROOPA,
   KOOPA_SHELL,
   PIRANHA_PLANT,
   CHEEP_CHEEP,
   BLOOPER,
   HAMMER_BRO,
   BOWSER,
   LAKITU,
   SPINE,
   LAVA_BUBBLE,
   BULLET_BILL,
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

struct PiranhaPlantComponent : public Component {
   Vector2f pipeCoordinates;
   bool inPipe = false;
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

struct HammerBroComponent : public Component {
   HammerBroComponent(std::function<void(Entity*)> throwHammer) : throwHammer{throwHammer} {}

   int lastJumpTime = 0;
   int lastThrowTime = 0;
   int lastMoveTime = 0;

   Entity* hammer = nullptr;

   Direction lastMoveDirection = Direction::NONE;

   std::function<void(Entity*)> throwHammer;
};

struct LakituComponent : public Component {
   int sideChangeTimer = 0;
   Direction lakituSide = Direction::LEFT;
   PIDController speedController = PIDController(0.06, 0, 0);
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

struct AddLivesComponent : public Component {
   AddLivesComponent(int livesNumber = 1) : livesNumber{livesNumber} {}

   int livesNumber;
};

struct CreateFloatingTextComponent : public Component {
   CreateFloatingTextComponent(Entity* originalEntity, std::string text)
       : originalEntity{originalEntity}, text{text} {}

   Entity* originalEntity;
   std::string text;
};
