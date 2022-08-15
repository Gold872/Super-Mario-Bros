#include "AABBCollision.h"
#include "Camera.h"
#include "Constants.h"
#include "ECS/Components.h"
#include "Map.h"
#include "SoundManager.h"
#include "command/CommandScheduler.h"
#include "command/Commands.h"
#include "systems/MapSystem.h"

#include <functional>
#include <iostream>
#include <time.h>
#include <tuple>
#include <vector>

template <typename T>
std::tuple<Vector2i, T> getDataCoordinate(std::vector<std::tuple<Vector2i, T>> levelData,
                                          Vector2i coordinate) {
   for (auto tupledData : levelData) {
      if (std::get<0>(tupledData) == coordinate) {
         return tupledData;
      }
   }

   return std::tuple<Vector2i, T>({0, 0}, T::NONE);
}

auto getPlatformCoordinate(
    std::vector<std::tuple<Vector2i, PlatformMotionType, Direction, Vector2i, bool>> levelData,
    Vector2i coordinate) {
   for (auto tupledData : levelData) {
      if (std::get<0>(tupledData) == coordinate) {
         return std::tuple<PlatformMotionType, Direction, Vector2i, bool>(
             std::get<1>(tupledData), std::get<2>(tupledData), std::get<3>(tupledData),
             std::get<4>(tupledData));
      }
   }
   return std::tuple<PlatformMotionType, Direction, Vector2i, bool>(
       PlatformMotionType::NONE, Direction::NONE, Vector2i(0, 0), false);
}

auto getPlatformLevelCoordinate(std::vector<std::tuple<Vector2i, Vector2i, int>> levelData,
                                Vector2i coordinate) {
   for (auto tupledData : levelData) {
      if (std::get<0>(tupledData) == coordinate) {
         return std::tuple<Vector2i, Vector2i, int>(tupledData);
      }
   }
   return std::tuple<Vector2i, Vector2i, int>(Vector2i(0, 0), Vector2i(0, 0), 0);
}

auto getPipeCoordinate(std::vector<std::tuple<Vector2i, Vector2i, Vector2i, Direction, Direction,
                                              bool, BackgroundColor, LevelType, Vector2i>>
                           levelData,
                       Vector2i coordinate) {
   for (auto tupledData : levelData) {
      if (std::get<0>(tupledData) == coordinate) {
         return std::tuple<Vector2i, Vector2i, Direction, Direction, bool, BackgroundColor,
                           LevelType, Vector2i>(std::get<1>(tupledData), std::get<2>(tupledData),
                                                std::get<3>(tupledData), std::get<4>(tupledData),
                                                std::get<5>(tupledData), std::get<6>(tupledData),
                                                std::get<7>(tupledData), std::get<8>(tupledData));
      }
   }

   return std::tuple<Vector2i, Vector2i, Direction, Direction, bool, BackgroundColor, LevelType,
                     Vector2i>({0, 0}, {0, 0}, Direction::NONE, Direction::NONE, false,
                               BackgroundColor::BLACK, LevelType::NONE, Vector2i(0, 0));
}

auto getFireBarCoordinate(std::vector<std::tuple<Vector2i, int, RotationDirection, int>> levelData,
                          Vector2i coordinate) {
   for (auto tupledData : levelData) {
      if (std::get<0>(tupledData) == coordinate) {
         return tupledData;
      }
   }
   return std::tuple<Vector2i, int, RotationDirection, int>(Vector2i(0, 0), 0.0,
                                                            RotationDirection::NONE, 0);
}

auto getVineCoordinate(std::vector<std::tuple<Vector2i, Vector2i, Vector2i, int, Vector2i, int,
                                              BackgroundColor, LevelType>>
                           levelData,
                       Vector2i coordinate) {
   for (auto tupledData : levelData) {
      if (std::get<0>(tupledData) == coordinate) {
         return tupledData;
      }
   }

   return std::tuple<Vector2i, Vector2i, Vector2i, int, Vector2i, int, BackgroundColor, LevelType>(
       Vector2i(0), Vector2i(0), Vector2i(0), 0, Vector2i(0), 0, BackgroundColor::BLACK,
       LevelType::NONE);
}

MapSystem::MapSystem(GameScene* scene) {
   this->scene = scene;
}

float MapSystem::generateRandomNumber(float min, float max) {
   return ((float)rand() / (float)RAND_MAX) * ((max - min) + 1) + min;
}

// Gets the Block ID that is equivalent to its ID in the Overworld
int MapSystem::getReferenceBlockID(int entityID) {
   if (entityID == -1) {
      return -1;
   }

   if (Map::IrregularBlockReferences.find(entityID) != Map::IrregularBlockReferences.end()) {
      return Map::IrregularBlockReferences.at(entityID);
   }

   Vector2i blockCoordinates = Map::BlockIDCoordinates.at(entityID);

   int coordinateX = blockCoordinates.x;
   int coordinateY = blockCoordinates.y;

   if (coordinateY > 10 && blockCoordinates.x < 32) {
      coordinateY -= 11;
   }

   if (coordinateX > 15 && coordinateX < 32) {
      coordinateX -= 16;
   } else if (coordinateX >= 32 && blockCoordinates.y < 10) {
      coordinateX -= 32;
   }

   for (auto it = Map::BlockIDCoordinates.begin(); it != Map::BlockIDCoordinates.end(); it++) {
      if (it->second == Vector2i(coordinateX, coordinateY)) {
         return it->first;
      }
   }

   return -1;
}

int MapSystem::getReferenceBlockIDAsEntity(int entityID, int referenceID) {
   if (entityID == -1) {
      return -1;
   }

   Vector2i entityCoordinates = Map::BlockIDCoordinates.at(entityID);
   Vector2i referenceCoordinates = Map::BlockIDCoordinates.at(referenceID);

   int entityCoordinateX = entityCoordinates.x;
   int entityCoordinateY = entityCoordinates.y;

   int referenceCoordinateX = referenceCoordinates.x;
   int referenceCoordinateY = referenceCoordinates.y;

   if (entityCoordinateY > 10) {
      referenceCoordinateY += 11;
   }

   if (entityCoordinateX > 15 && entityCoordinateX < 32) {
      referenceCoordinateX += 16;
   } else if (entityCoordinateX >= 32) {
      referenceCoordinateX -= 32;
   }

   for (auto it = Map::BlockIDCoordinates.begin(); it != Map::BlockIDCoordinates.end(); it++) {
      if (it->second == Vector2i(referenceCoordinateX, referenceCoordinateY)) {
         return it->first;
      }
   }

   return -1;
}

int MapSystem::getReferenceEnemyID(int entityID) {
   if (entityID == -1) {
      return -1;
   }
   Vector2i entityCoordinates = Map::EnemyIDCoordinates.at(entityID);

   int coordinateX = entityCoordinates.x;
   int coordinateY = entityCoordinates.y;

   if (coordinateY > 2 && coordinateY < 12) {
      coordinateY -= (coordinateY - (coordinateY % 3));
   }

   for (auto it = Map::EnemyIDCoordinates.begin(); it != Map::EnemyIDCoordinates.end(); it++) {
      if (it->second == Vector2i(coordinateX, coordinateY)) {
         return it->first;
      }
   }

   return -1;
}

int MapSystem::getReferenceEnemyIDAsEntity(int entityID, int referenceID) {
   if (entityID == -1 || referenceID == -1) {
      return -1;
   }

   Vector2i entityCoordinates = Map::EnemyIDCoordinates.at(entityID);
   Vector2i referenceCoordinates = Map::EnemyIDCoordinates.at(referenceID);

   int entityCoordinateY = entityCoordinates.y;

   int referenceCoordinateX = referenceCoordinates.x;
   int referenceCoordinateY = referenceCoordinates.y;

   if (entityCoordinateY > 2 && entityCoordinateY < 12) {
      referenceCoordinateY += (entityCoordinateY - (entityCoordinateY % 3));
   }

   for (auto it = Map::EnemyIDCoordinates.begin(); it != Map::EnemyIDCoordinates.end(); it++) {
      if (it->second == Vector2i(referenceCoordinateX, referenceCoordinateY)) {
         return it->first;
      }
   }

   return -1;
}

Entity* MapSystem::createBlockEntity(World* world, int coordinateX, int coordinateY, int entityID) {
   Entity* entity(world->create());

   entity->addComponent<PositionComponent>(Vector2f(coordinateX, coordinateY) * SCALED_CUBE_SIZE,
                                           Vector2i(SCALED_CUBE_SIZE));

   entity->addComponent<TextureComponent>(scene->blockTexture, false, false);

   entity->addComponent<SpritesheetComponent>(ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE, 1, 1, 1,
                                              ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE,
                                              Map::BlockIDCoordinates.at(entityID));

   entity->addComponent<ForegroundComponent>();

   entity->addComponent<TileComponent>();

   return entity;
}

Entity* MapSystem::createBackgroundEntity(World* world, int coordinateX, int coordinateY,
                                          int entityID) {
   Entity* entity(world->create());

   entity->addComponent<PositionComponent>(Vector2f(coordinateX, coordinateY) * SCALED_CUBE_SIZE,
                                           Vector2i(SCALED_CUBE_SIZE));

   entity->addComponent<TextureComponent>(scene->blockTexture, false, false);

   entity->addComponent<SpritesheetComponent>(ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE, 1, 1, 1,
                                              ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE,
                                              Map::BlockIDCoordinates.at(entityID));

   entity->addComponent<BackgroundComponent>();

   return entity;
}

// Creates a moving platform entity
Entity* MapSystem::createPlatformEntity(
    World* world, int coordinateX, int coordinateY, int entityID, int platformLength,
    std::tuple<PlatformMotionType, Direction, Vector2i, bool> platformData) {
   Entity* platform(world->create());

   platform->addComponent<PositionComponent>(Vector2f(coordinateX, coordinateY) * SCALED_CUBE_SIZE,
                                             Vector2i(platformLength, 1) * SCALED_CUBE_SIZE);

   platform->addComponent<TextureComponent>(scene->blockTexture);

   platform->addComponent<SpritesheetComponent>(
       ORIGINAL_CUBE_SIZE * platformLength, ORIGINAL_CUBE_SIZE, 1, 1, 1, ORIGINAL_CUBE_SIZE,
       ORIGINAL_CUBE_SIZE, Map::BlockIDCoordinates.at(entityID));

   platform->addComponent<ForegroundComponent>();

   platform->addComponent<TileComponent>();

   if (std::get<3>(platformData)) {
      platform->getComponent<PositionComponent>()->position.x += SCALED_CUBE_SIZE / 2;
   }

   platform->addComponent<FrictionExemptComponent>();

   platform->addComponent<CollisionExemptComponent>();

   auto* move = platform->addComponent<MovingComponent>(Vector2f(0, 0), Vector2f(0, 0));

   switch (std::get<0>(platformData)) {
      case PlatformMotionType::ONE_DIRECTION_REPEATED: {
         Direction movingDirection = std::get<1>(platformData);
         Vector2i minMax = std::get<2>(platformData);

         minMax += Vector2i(0, 1);

         minMax *= SCALED_CUBE_SIZE;

         platform->addComponent<MovingPlatformComponent>(PlatformMotionType::ONE_DIRECTION_REPEATED,
                                                         movingDirection, minMax);

         switch (movingDirection) {
            case Direction::LEFT:
               move->velocity.x = -2.0;
               break;
            case Direction::RIGHT:
               move->velocity.x = 2.0;
               break;
            case Direction::UP:
               move->velocity.y = -2.0;
               break;
            case Direction::DOWN:
               move->velocity.y = 2.0;
               break;
            default:
               break;
         }
      } break;
      case PlatformMotionType::ONE_DIRECTION_CONTINUOUS: {
         Direction movingDirection = std::get<1>(platformData);

         platform->addComponent<MovingPlatformComponent>(
             PlatformMotionType::ONE_DIRECTION_CONTINUOUS, movingDirection, Vector2i(0, 0));

         switch (movingDirection) {
            case Direction::LEFT:
               move->velocity.x = -2.0;
               break;
            case Direction::RIGHT:
               move->velocity.x = 2.0;
               break;
            case Direction::UP:
               move->velocity.y = -2.0;
               break;
            case Direction::DOWN:
               move->velocity.y = 2.0;
               break;
            default:
               break;
         }
      } break;
      case PlatformMotionType::BACK_AND_FORTH: {
         Direction movingDirection = std::get<1>(platformData);
         Vector2i minMax = std::get<2>(platformData);

         minMax += Vector2i(0, 1);

         minMax *= SCALED_CUBE_SIZE;

         platform->addComponent<MovingPlatformComponent>(PlatformMotionType::BACK_AND_FORTH,
                                                         movingDirection, minMax);
      } break;
      case PlatformMotionType::GRAVITY:
         platform->addComponent<MovingPlatformComponent>(PlatformMotionType::GRAVITY,
                                                         Direction::NONE);
         break;
      default:
         break;
   }

   return platform;
}

Entity* MapSystem::createPlatformLevelEntity(
    World* world, std::tuple<Vector2i, Vector2i, int> platformLevelData) {
   Vector2i leftCoordinate = std::get<0>(platformLevelData);
   Vector2i rightCoordinate = std::get<1>(platformLevelData);
   int pulleyHeight = std::get<2>(platformLevelData) + 1;

   // The X coordinates of the pulley lines
   int leftLineX = leftCoordinate.x + 1;
   int rightLineX = rightCoordinate.x + 1;

   Entity* leftPulleyLine = createBackgroundEntity(world, leftLineX, pulleyHeight, 391);
   Entity* rightPulleyLine = createBackgroundEntity(world, rightLineX, pulleyHeight, 391);

   Entity* leftPlatform(world->create());
   Entity* rightPlatform(world->create());

   leftPlatform->addComponent<PositionComponent>(
       leftCoordinate.convertTo<float>() * SCALED_CUBE_SIZE, Vector2i(3, 1) * SCALED_CUBE_SIZE,
       SDL_Rect{0, 0, 3 * SCALED_CUBE_SIZE, SCALED_CUBE_SIZE / 2});

   leftPlatform->addComponent<TextureComponent>(scene->blockTexture);

   leftPlatform->addComponent<SpritesheetComponent>(ORIGINAL_CUBE_SIZE * 3, ORIGINAL_CUBE_SIZE, 1,
                                                    1, 1, ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE,
                                                    Map::BlockIDCoordinates.at(809));

   leftPlatform->addComponent<MovingComponent>(Vector2f(0, 0), Vector2f(0, 0));

   leftPlatform->addComponent<PlatformLevelComponent>(rightPlatform, leftPulleyLine,
                                                      pulleyHeight * SCALED_CUBE_SIZE);

   leftPlatform->addComponent<ForegroundComponent>();

   leftPlatform->addComponent<TileComponent>();

   rightPlatform->addComponent<PositionComponent>(
       rightCoordinate.convertTo<float>() * SCALED_CUBE_SIZE, Vector2i(3, 1) * SCALED_CUBE_SIZE,
       SDL_Rect{0, 0, 3 * SCALED_CUBE_SIZE, SCALED_CUBE_SIZE / 2});

   rightPlatform->addComponent<TextureComponent>(scene->blockTexture);

   rightPlatform->addComponent<SpritesheetComponent>(ORIGINAL_CUBE_SIZE * 3, ORIGINAL_CUBE_SIZE, 1,
                                                     1, 1, ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE,
                                                     Map::BlockIDCoordinates.at(809));

   rightPlatform->addComponent<MovingComponent>(Vector2f(0, 0), Vector2f(0, 0));

   rightPlatform->addComponent<PlatformLevelComponent>(leftPlatform, rightPulleyLine,
                                                       pulleyHeight * SCALED_CUBE_SIZE);

   rightPlatform->addComponent<ForegroundComponent>();

   rightPlatform->addComponent<TileComponent>();

   return leftPlatform;
}

void MapSystem::addItemDispenser(World* world, Entity* entity, int entityID, int referenceID) {
   auto* mysteryBox = entity->getComponent<MysteryBoxComponent>();
   auto blockTexture = entity->getComponent<TextureComponent>()->getTexture();

   int deactivatedID = getReferenceBlockIDAsEntity(entityID, 196);

   mysteryBox->deactivatedCoordinates = Map::BlockIDCoordinates.at(deactivatedID);

   switch (mysteryBox->boxType) {
      case MysteryBoxType::MUSHROOM:
         mysteryBox->whenDispensed = [=](Entity* originalBlock) {
            Entity* dispenseSound(world->create());
            dispenseSound->addComponent<SoundComponent>(SoundID::POWER_UP_APPEAR);

            if (world->findFirst<PlayerComponent>()->getComponent<PlayerComponent>()->playerState !=
                PlayerState::SMALL_MARIO) {
               Entity* fireFlower(world->create());

               auto* position = originalBlock->getComponent<PositionComponent>();

               fireFlower->addComponent<PositionComponent>(position->position,
                                                           Vector2i(SCALED_CUBE_SIZE));

               int flowerID = getReferenceBlockIDAsEntity(entityID, 48);

               fireFlower->addComponent<TextureComponent>(blockTexture);

               fireFlower->addComponent<SpritesheetComponent>(
                   ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE, 1, 1, 1, ORIGINAL_CUBE_SIZE,
                   ORIGINAL_CUBE_SIZE, Map::BlockIDCoordinates.at(flowerID));

               fireFlower->addComponent<AnimationComponent>(
                   std::vector<int>{flowerID, flowerID + 1, flowerID + 2, flowerID + 3}, 8,
                   Map::BlockIDCoordinates);

               fireFlower->addComponent<CollectibleComponent>(CollectibleType::FIRE_FLOWER);

               fireFlower->addComponent<MovingComponent>(Vector2f(0, -1.0f), Vector2f(0, 0));

               fireFlower->addComponent<CollisionExemptComponent>();

               fireFlower->addComponent<WaitUntilComponent>(
                   [=](Entity* entity) {
                      return position->getTop() >
                             entity->getComponent<PositionComponent>()->getBottom();
                   },
                   [](Entity* entity) {
                      entity->addComponent<GravityComponent>();
                      entity->getComponent<MovingComponent>()->velocity.y = 0;
                      entity->remove<CollisionExemptComponent>();
                      entity->remove<WaitUntilComponent>();
                   });
            } else {
               Entity* mushroom(world->create());

               auto* position = originalBlock->getComponent<PositionComponent>();

               mushroom->addComponent<PositionComponent>(position->position,
                                                         Vector2i(SCALED_CUBE_SIZE));

               mushroom->addComponent<TextureComponent>(blockTexture);

               mushroom->addComponent<SpritesheetComponent>(
                   ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE, 1, 1, 1, ORIGINAL_CUBE_SIZE,
                   ORIGINAL_CUBE_SIZE, Map::BlockIDCoordinates.at(608));

               mushroom->addComponent<CollectibleComponent>(CollectibleType::MUSHROOM);

               mushroom->addComponent<MovingComponent>(Vector2f(0, -1.0f), Vector2f(0, 0));

               mushroom->addComponent<CollisionExemptComponent>();

               mushroom->addComponent<WaitUntilComponent>(
                   [=](Entity* entity) {
                      return position->getTop() >
                             entity->getComponent<PositionComponent>()->getBottom();
                   },
                   [](Entity* entity) {
                      entity->addComponent<GravityComponent>();
                      entity->getComponent<MovingComponent>()->velocity.x = COLLECTIBLE_SPEED;
                      entity->remove<CollisionExemptComponent>();
                      entity->remove<WaitUntilComponent>();
                   });
            }
         };
         break;
      case MysteryBoxType::COINS:
         mysteryBox->whenDispensed = [=](Entity* originalBlock) {
            Entity* coinSound(world->create());
            coinSound->addComponent<SoundComponent>(SoundID::COIN);

            Entity* addScore(world->create());
            addScore->addComponent<AddScoreComponent>(100, true);

            Entity* floatingText(world->create());
            floatingText->addComponent<CreateFloatingTextComponent>(originalBlock,
                                                                    std::to_string(100));

            Entity* coin(world->create());

            auto* position = originalBlock->getComponent<PositionComponent>();

            coin->addComponent<PositionComponent>(position->position, Vector2i(SCALED_CUBE_SIZE));

            coin->addComponent<TextureComponent>(blockTexture);

            coin->addComponent<SpritesheetComponent>(ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE, 1, 1,
                                                     1, ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE,
                                                     Map::BlockIDCoordinates.at(656));

            coin->addComponent<ForegroundComponent>();

            coin->addComponent<AnimationComponent>(std::vector<int>{656, 657, 658, 659}, 8,
                                                   Map::BlockIDCoordinates);

            coin->addComponent<MovingComponent>(Vector2f(0, -10.00), Vector2f(0, 0.3));

            coin->addComponent<GravityComponent>();

            coin->addComponent<ParticleComponent>();

            coin->addComponent<WaitUntilComponent>(
                [=](Entity* entity) {
                   return AABBTotalCollision(position, entity->getComponent<PositionComponent>()) &&
                          entity->getComponent<MovingComponent>()->velocity.y >= 0;
                },
                [=](Entity* entity) {
                   world->destroy(entity);
                });
         };
         break;
      case MysteryBoxType::SUPER_STAR: {
         mysteryBox->whenDispensed = [=](Entity* originalBlock) {
            Entity* dispenseSound(world->create());
            dispenseSound->addComponent<SoundComponent>(SoundID::POWER_UP_APPEAR);

            Entity* star(world->create());

            auto* position = originalBlock->getComponent<PositionComponent>();

            star->addComponent<PositionComponent>(position->position, Vector2i(SCALED_CUBE_SIZE));

            int starID = getReferenceBlockIDAsEntity(entityID, 96);

            star->addComponent<TextureComponent>(blockTexture);

            star->addComponent<SpritesheetComponent>(ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE, 1, 1,
                                                     1, ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE,
                                                     Map::BlockIDCoordinates.at(starID));

            star->addComponent<AnimationComponent>(
                std::vector<int>{starID, starID + 1, starID + 2, starID + 3}, 8,
                Map::BlockIDCoordinates);

            star->addComponent<CollectibleComponent>(CollectibleType::SUPER_STAR);

            star->addComponent<MovingComponent>(Vector2f(0, -1.0f), Vector2f(0, 0));

            star->addComponent<CollisionExemptComponent>();

            star->addComponent<WaitUntilComponent>(
                [=](Entity* entity) {
                   return position->getTop() >
                          entity->getComponent<PositionComponent>()->getBottom();
                },
                [&](Entity* entity) {
                   entity->addComponent<GravityComponent>();
                   entity->getComponent<MovingComponent>()->velocity.x = COLLECTIBLE_SPEED;
                   entity->remove<CollisionExemptComponent>();
                   entity->remove<WaitUntilComponent>();
                });
         };
      } break;
      case MysteryBoxType::ONE_UP: {
         mysteryBox->whenDispensed = [=](Entity* originalBlock) {
            Entity* dispenseSound(world->create());
            dispenseSound->addComponent<SoundComponent>(SoundID::POWER_UP_APPEAR);

            Entity* oneup(world->create());

            auto* position = originalBlock->getComponent<PositionComponent>();

            oneup->addComponent<PositionComponent>(position->position, Vector2i(SCALED_CUBE_SIZE));

            int oneupID = getReferenceBlockIDAsEntity(entityID, 52);

            oneup->addComponent<TextureComponent>(blockTexture);

            oneup->addComponent<SpritesheetComponent>(ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE, 1, 1,
                                                      1, ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE,
                                                      Map::BlockIDCoordinates.at(oneupID));

            oneup->addComponent<CollectibleComponent>(CollectibleType::ONE_UP);

            oneup->addComponent<MovingComponent>(Vector2f(0, -1.0f), Vector2f(0, 0));

            oneup->addComponent<CollisionExemptComponent>();

            oneup->addComponent<WaitUntilComponent>(
                [=](Entity* entity) {
                   return position->getTop() >
                          entity->getComponent<PositionComponent>()->getBottom();
                },
                [&](Entity* entity) {
                   entity->addComponent<GravityComponent>();
                   entity->getComponent<MovingComponent>()->velocity.x = COLLECTIBLE_SPEED;
                   entity->remove<CollisionExemptComponent>();
                   entity->remove<WaitUntilComponent>();
                });
         };
      } break;
      case MysteryBoxType::VINES: {
         Vector2f blockPosition =
             entity->getComponent<PositionComponent>()->position / SCALED_CUBE_SIZE;

         auto vineData =
             getVineCoordinate(scene->getLevelData().vineLocations, blockPosition.convertTo<int>());

         if (std::get<0>(vineData) == Vector2i(0, 0)) {
            break;
         }

         std::vector<Entity*> vineParts;

         int vineLength = 0;

         int vineTopID = getReferenceBlockIDAsEntity(entityID, 100);
         int vineBodyID = getReferenceBlockIDAsEntity(entityID, 148);

         mysteryBox->whenDispensed = [=](Entity* originalBlock) mutable {
            Entity* dispenseSound(world->create());
            dispenseSound->addComponent<SoundComponent>(SoundID::POWER_UP_APPEAR);

            originalBlock->addComponent<AboveForegroundComponent>();

            Entity* vineTop(world->create());

            auto* position = originalBlock->getComponent<PositionComponent>();

            vineTop->addComponent<PositionComponent>(position->position,
                                                     Vector2i(SCALED_CUBE_SIZE));
            vineTop->addComponent<TextureComponent>(blockTexture);

            vineTop->addComponent<SpritesheetComponent>(
                ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE, 1, 1, 1, ORIGINAL_CUBE_SIZE,
                ORIGINAL_CUBE_SIZE, Map::BlockIDCoordinates.at(vineTopID));

            vineTop->addComponent<MovingComponent>(Vector2f(0, -1.0), Vector2f(0, 0));

            vineTop->addComponent<MoveOutsideCameraComponent>();

            vineTop->addComponent<FrictionExemptComponent>();

            vineTop->addComponent<CollisionExemptComponent>();

            vineTop->addComponent<VineComponent>(
                std::get<0>(vineData), std::get<1>(vineData), std::get<2>(vineData),
                std::get<3>(vineData), std::get<4>(vineData), std::get<5>(vineData),
                std::get<6>(vineData), std::get<7>(vineData), vineParts);

            vineTop->addComponent<ForegroundComponent>();

            vineParts.push_back(vineTop);

            vineLength++;

            Entity* vineGrowController(world->create());

            /* Periodically waits until the bottom of the vine has moved past the block, and then
             * adds another piece to the vine.
             *
             * This keeps happening until the vine has fully grown
             */
            vineGrowController->addComponent<WaitUntilComponent>(
                [=, &vineParts](Entity* entity) {
                   auto* position = originalBlock->getComponent<PositionComponent>();

                   return vineParts.back()->getComponent<PositionComponent>()->getBottom() <=
                          position->getTop();
                },
                [=, &blockTexture, &vineParts, &vineLength](Entity* entity) {
                   auto* position = originalBlock->getComponent<PositionComponent>();

                   // Adds another part to the vine
                   if (vineLength < 6) {
                      Entity* vinePiece(world->create());

                      vinePiece->addComponent<PositionComponent>(position->position,
                                                                 Vector2i(SCALED_CUBE_SIZE));

                      vinePiece->addComponent<TextureComponent>(blockTexture);

                      vinePiece->addComponent<SpritesheetComponent>(
                          ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE, 1, 1, 1, ORIGINAL_CUBE_SIZE,
                          ORIGINAL_CUBE_SIZE, Map::BlockIDCoordinates.at(vineBodyID));

                      vinePiece->addComponent<MovingComponent>(Vector2f(0, -1.0), Vector2f(0, 0));

                      vinePiece->addComponent<MoveOutsideCameraComponent>();

                      vinePiece->addComponent<FrictionExemptComponent>();

                      vinePiece->addComponent<CollisionExemptComponent>();

                      vinePiece->addComponent<VineComponent>(
                          std::get<0>(vineData), std::get<1>(vineData), std::get<2>(vineData),
                          std::get<3>(vineData), std::get<4>(vineData), std::get<5>(vineData),
                          std::get<6>(vineData), std::get<7>(vineData), vineParts);

                      vinePiece->addComponent<ForegroundComponent>();

                      vineParts.push_back(vinePiece);
                      vineLength++;
                      // If the vine is fully grown and the last vine is no longer in the block
                   } else if (vineParts.back()->getComponent<PositionComponent>()->getBottom() <=
                              position->getTop()) {
                      for (Entity* e : vineParts) {
                         e->getComponent<MovingComponent>()->velocity.y = 0;
                      }
                      vineGrowController->remove<WaitUntilComponent>();

                      world->destroy(vineGrowController);
                   }
                });
         };
      } break;
      default:
         break;
   }
}

void MapSystem::createForegroundEntities(World* world, int coordinateX, int coordinateY,
                                         int entityID, int referenceID,
                                         bool createInvisibleBlocks) {
   switch (referenceID) {
      case -1:
         if (createInvisibleBlocks &&
             scene->collectiblesMap.getLevelData()[coordinateY][coordinateX] != -1) {
            Entity* entity(createBlockEntity(world, coordinateX, coordinateY, 53));

            entity->addComponent<InvisibleBlockComponent>();
            entity->addComponent<BumpableComponent>();

            MysteryBoxType collectibleType = MysteryBoxType::NONE;

            int collectibleID = scene->collectiblesMap.getLevelData()[coordinateY][coordinateX];
            int referenceCollectibleID = getReferenceBlockID(collectibleID);

            int blankBlockID;

            if (collectibleID != 608) {
               blankBlockID = getReferenceBlockIDAsEntity(collectibleID, 53);
            } else {
               switch (scene->getLevelData().levelType) {
                  case LevelType::UNDERGROUND:
                  case LevelType::START_UNDERGROUND:
                     blankBlockID = 69;  // nice
                     break;
                  case LevelType::CASTLE:
                     blankBlockID = 581;
                     break;
                  default:
                     blankBlockID = 53;
                     break;
               }
            }

            switch (referenceCollectibleID) {
               case 52:  // One-up
                  collectibleType = MysteryBoxType::ONE_UP;
                  break;
               case 96:  // Super Star
                  collectibleType = MysteryBoxType::SUPER_STAR;
                  break;
               case 144:  // Coin
                  collectibleType = MysteryBoxType::COINS;
                  break;
               case 608:  // Mushroom
                  collectibleType = MysteryBoxType::MUSHROOM;
                  break;
               default:
                  break;
            }

            if (collectibleType != MysteryBoxType::NONE) {
               entity->addComponent<MysteryBoxComponent>(collectibleType);

               addItemDispenser(world, entity, blankBlockID, 53);
            }
         }
         break;
      case 394:
      case 762:
      case 810:
      case 811:
         break;
      case 144: {  // COIN
         Entity* entity(world->create());

         entity->addComponent<PositionComponent>(
             Vector2f(coordinateX, coordinateY) * SCALED_CUBE_SIZE, Vector2i(SCALED_CUBE_SIZE));

         entity->addComponent<TextureComponent>(scene->blockTexture, false, false);

         entity->addComponent<SpritesheetComponent>(ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE, 1, 1, 1,
                                                    ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE,
                                                    Map::BlockIDCoordinates.at(entityID));

         entity->addComponent<AnimationComponent>(
             std::vector<int>{entityID, entityID + 1, entityID + 2, entityID + 3}, 8,
             Map::BlockIDCoordinates);

         entity->addComponent<PausedAnimationComponent>(0, 25);

         entity->addComponent<CollectibleComponent>(CollectibleType::COIN);

         break;
      }
      /* ****************************************************************** */
      case 101:
      case 149: {  // FLAG POLE
         Entity* entity(world->create());

         entity->addComponent<PositionComponent>(
             Vector2f(coordinateX, coordinateY) * SCALED_CUBE_SIZE, Vector2i(SCALED_CUBE_SIZE));

         entity->addComponent<TextureComponent>(scene->blockTexture, false, false);

         entity->addComponent<SpritesheetComponent>(ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE, 1, 1, 1,
                                                    ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE,
                                                    Map::BlockIDCoordinates.at(entityID));

         entity->addComponent<ForegroundComponent>();

         entity->addComponent<FlagPoleComponent>();
         break;
      }
      /* ****************************************************************** */
      case 152: {  // FLAG
         Entity* entity(world->create());

         entity->addComponent<PositionComponent>(
             Vector2f(coordinateX * SCALED_CUBE_SIZE + SCALED_CUBE_SIZE / 2,
                      coordinateY * SCALED_CUBE_SIZE),
             Vector2i(SCALED_CUBE_SIZE));

         entity->addComponent<TextureComponent>(scene->blockTexture, false, false);

         entity->addComponent<SpritesheetComponent>(ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE, 1, 1, 1,
                                                    ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE,
                                                    Map::BlockIDCoordinates.at(entityID));

         entity->addComponent<MovingComponent>(Vector2f(0, 0), Vector2f(0, 0));

         entity->addComponent<ForegroundComponent>();

         entity->addComponent<FlagComponent>();
         break;
      }
      /* ****************************************************************** */
      case 192: {  // QUESTION BLOCK
         Entity* entity = createBlockEntity(world, coordinateX, coordinateY, entityID);

         entity->addComponent<AnimationComponent>(
             std::vector<int>{entityID, entityID + 1, entityID + 2, entityID + 3}, 8,
             Map::BlockIDCoordinates);

         entity->addComponent<PausedAnimationComponent>(0, 25);

         entity->addComponent<BumpableComponent>();

         MysteryBoxType collectibleType = MysteryBoxType::COINS;

         int collectibleID = scene->collectiblesMap.getLevelData()[coordinateY][coordinateX];
         if (collectibleID != -1) {
            int referenceCollectibleID = getReferenceBlockID(collectibleID);
            switch (referenceCollectibleID) {
               case 52:  // One-up
                  collectibleType = MysteryBoxType::ONE_UP;
                  break;
               case 96:  // Super Star
                  collectibleType = MysteryBoxType::SUPER_STAR;
                  break;
               case 144:  // Coin
                  collectibleType = MysteryBoxType::COINS;
                  break;
               case 608:  // Mushroom
                  collectibleType = MysteryBoxType::MUSHROOM;
                  break;
               default:
                  break;
            }
         }

         if (collectibleType != MysteryBoxType::NONE) {
            entity->addComponent<MysteryBoxComponent>(collectibleType);

            addItemDispenser(world, entity, entityID, referenceID);
         }
      } break;
      /* ****************************************************************** */
      case 240: {  // AXE
         Entity* entity(world->create());

         entity->addComponent<PositionComponent>(
             Vector2f(coordinateX, coordinateY) * SCALED_CUBE_SIZE, Vector2i(SCALED_CUBE_SIZE));

         entity->addComponent<TextureComponent>(scene->blockTexture, false, false);

         entity->addComponent<SpritesheetComponent>(ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE, 1, 1, 1,
                                                    ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE,
                                                    Map::BlockIDCoordinates.at(entityID));

         entity->addComponent<AnimationComponent>(
             std::vector<int>{entityID, entityID + 1, entityID + 2, entityID + 3}, 8,
             Map::BlockIDCoordinates);

         entity->addComponent<PausedAnimationComponent>(0, 25);

         entity->addComponent<ForegroundComponent>();

         entity->addComponent<AxeComponent>();
      } break;
      /* ****************************************************************** */
      case 289:
      case 290: {  // BRICKS
         Entity* entity = createBlockEntity(world, coordinateX, coordinateY, entityID);

         int debrisID = getReferenceBlockIDAsEntity(entityID, 291);

         entity->addComponent<DestructibleComponent>(Map::BlockIDCoordinates.at(debrisID));

         entity->addComponent<BumpableComponent>();

         MysteryBoxType boxType = MysteryBoxType::NONE;

         int collectibleID = scene->collectiblesMap.getLevelData()[coordinateY][coordinateX];
         if (collectibleID != -1) {
            int referenceCollectibleID = getReferenceBlockID(collectibleID);
            switch (referenceCollectibleID) {
               case 52:  // One-up
                  boxType = MysteryBoxType::ONE_UP;
                  break;
               case 96:  // Super Star
                  boxType = MysteryBoxType::SUPER_STAR;
                  break;
               case 144:  // Coin
                  boxType = MysteryBoxType::COINS;
                  break;
               case 148:  // VINES
                  boxType = MysteryBoxType::VINES;
                  break;
               case 608:  // Mushroom
                  boxType = MysteryBoxType::MUSHROOM;
                  break;
            }
         }

         if (boxType != MysteryBoxType::NONE) {
            entity->addComponent<MysteryBoxComponent>(boxType);

            addItemDispenser(world, entity, entityID, referenceID);
         }
      } break;
      /* ****************************************************************** */
      case 339: {  // BRIDGE CHAIN (this is here so it gets destroyed with the bridge
         if (scene->getLevelData().levelType == LevelType::CASTLE) {
            Entity* bridgeChain(createBlockEntity(world, coordinateX, coordinateY, entityID));

            bridgeChain->addComponent<BridgeChainComponent>();
         } else {
            createBlockEntity(world, coordinateX, coordinateY, entityID);
         }
      } break;
      /* ****************************************************************** */
      case 346: {  // TRAMPOLINE
         Entity* trampolineTop(createBlockEntity(world, coordinateX, coordinateY, entityID));
         Entity* trampolineBottom(
             createBlockEntity(world, coordinateX, coordinateY + 1, entityID + 48));

         trampolineBottom->remove<TileComponent>();

         trampolineTop->addComponent<TrampolineComponent>(
             trampolineBottom, (int[3]){entityID, entityID + 1, entityID + 2},
             (int[3]){entityID + 48, entityID + 1 + 48, entityID + 2 + 48});
      } break;
      /* ****************************************************************** */
      case 392: {  // BRIDGE
         if (scene->getLevelData().levelType == LevelType::CASTLE) {
            if (getReferenceBlockID(
                    scene->foregroundMap.getLevelData()[coordinateY][coordinateX - 1]) != 392) {
               Entity* bridge(createBlockEntity(world, coordinateX, coordinateY, entityID));

               auto* bridgeComponent = bridge->addComponent<BridgeComponent>();

               bridgeComponent->connectedBridgeParts.push_back(bridge);

               int futureCoordinateCheck = coordinateX;

               while (
                   getReferenceBlockID(
                       scene->foregroundMap.getLevelData()[coordinateY][++futureCoordinateCheck]) ==
                   392) {
                  Entity* connectedBridge(
                      createBlockEntity(world, futureCoordinateCheck, coordinateY, entityID));

                  bridgeComponent->connectedBridgeParts.push_back(connectedBridge);
               }
            }
         } else {
            createBlockEntity(world, coordinateX, coordinateY, entityID);
         }
      } break;
      /* ****************************************************************** */
      case 609: {  // MOVING PLATFORM
         createBlockEntity(world, coordinateX, coordinateY, entityID);
      } break;
      /* ****************************************************************** */
      case 761: {  // MOVING PLATFORM (2 wide)
         auto platformData = getPlatformCoordinate(scene->getLevelData().movingPlatformDirections,
                                                   Vector2i(coordinateX, coordinateY));

         createPlatformEntity(world, coordinateX, coordinateY, entityID, 2, platformData);
      } break;
      /* ****************************************************************** */
      case 809: {  // MOVING PLATFORM (3 wide)
         auto platformData = getPlatformCoordinate(scene->getLevelData().movingPlatformDirections,
                                                   Vector2i(coordinateX, coordinateY));
         if (std::get<0>(platformData) != PlatformMotionType::NONE) {
            createPlatformEntity(world, coordinateX, coordinateY, entityID, 3, platformData);
            return;
         }

         auto platformLevelData = getPlatformLevelCoordinate(
             scene->getLevelData().platformLevelLocations, Vector2i(coordinateX, coordinateY));

         if (std::get<0>(platformLevelData) != Vector2i(0, 0)) {
            createPlatformLevelEntity(world, platformLevelData);
            return;
         }

         //         createPlatformEntity(world, coordinateX, coordinateY, entityID, 3,
         //         platformData);
      } break;
      /* ****************************************************************** */
      default: {
         createBlockEntity(world, coordinateX, coordinateY, entityID);
      } break;
   }
}

void MapSystem::createEnemyEntities(World* world, int coordinateX, int coordinateY, int entityID,
                                    int referenceID) {
   switch (referenceID) {
      case -1:
         break;
         /* ****************************************************************** */
      case 38: {  // KOOPA
         Entity* entity(world->create());

         entity->addComponent<PositionComponent>(
             Vector2f(coordinateX, coordinateY) * SCALED_CUBE_SIZE,
             Vector2i(SCALED_CUBE_SIZE, SCALED_CUBE_SIZE * 2),
             SDL_Rect{0, SCALED_CUBE_SIZE, SCALED_CUBE_SIZE, SCALED_CUBE_SIZE});

         entity->addComponent<TextureComponent>(scene->enemyTexture);

         entity->addComponent<SpritesheetComponent>(ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE * 2, 1,
                                                    1, 0, ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE,
                                                    Map::EnemyIDCoordinates.at(entityID));

         int firstAnimationID = entityID;

         entity->addComponent<AnimationComponent>(
             std::vector<int>{firstAnimationID, firstAnimationID + 1}, 6, Map::EnemyIDCoordinates);

         entity->addComponent<MovingComponent>(Vector2f(-ENEMY_SPEED, 0), Vector2f(0, 0));

         entity->addComponent<GravityComponent>();

         entity->addComponent<CrushableComponent>([=](Entity* entity) {
            entity->getComponent<EnemyComponent>()->enemyType = EnemyType::KOOPA_SHELL;
            entity->getComponent<MovingComponent>()->velocity.x = 0.0;
            entity->getComponent<SpritesheetComponent>()->setEntityHeight(ORIGINAL_CUBE_SIZE);

            entity->addComponent<DestroyOutsideCameraComponent>();

            auto* position = entity->getComponent<PositionComponent>();
            position->scale.y = SCALED_CUBE_SIZE;
            position->hitbox = SDL_Rect{0, 0, SCALED_CUBE_SIZE, SCALED_CUBE_SIZE};
            position->position.y += SCALED_CUBE_SIZE;

            int shellCoordinate = getReferenceEnemyIDAsEntity(entityID, 77);
            entity->getComponent<SpritesheetComponent>()->setSpritesheetCoordinates(
                Map::EnemyIDCoordinates.at(shellCoordinate));

            entity->remove<AnimationComponent>();
         });

         entity->addComponent<EnemyComponent>(EnemyType::KOOPA);
      } break;
      /* ****************************************************************** */
      case 39: {  // KOOPA (shifted to the right)
         Entity* entity(world->create());

         entity->addComponent<PositionComponent>(
             Vector2f(coordinateX * SCALED_CUBE_SIZE + SCALED_CUBE_SIZE / 2,
                      coordinateY * SCALED_CUBE_SIZE),
             Vector2i(SCALED_CUBE_SIZE, SCALED_CUBE_SIZE * 2),
             SDL_Rect{0, SCALED_CUBE_SIZE, SCALED_CUBE_SIZE, SCALED_CUBE_SIZE});

         entity->addComponent<TextureComponent>(scene->enemyTexture);

         entity->addComponent<SpritesheetComponent>(ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE * 2, 1,
                                                    1, 0, ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE,
                                                    Map::EnemyIDCoordinates.at(entityID));

         int firstAnimationID = entityID - 1;

         entity->addComponent<AnimationComponent>(
             std::vector<int>{firstAnimationID, firstAnimationID + 1}, 6, Map::EnemyIDCoordinates);

         entity->addComponent<MovingComponent>(Vector2f(-ENEMY_SPEED, 0), Vector2f(0, 0));

         entity->addComponent<GravityComponent>();

         entity->addComponent<CrushableComponent>([=](Entity* entity) {
            entity->getComponent<EnemyComponent>()->enemyType = EnemyType::KOOPA_SHELL;
            entity->getComponent<MovingComponent>()->velocity.x = 0.0;
            entity->getComponent<SpritesheetComponent>()->setEntityHeight(ORIGINAL_CUBE_SIZE);

            entity->addComponent<DestroyOutsideCameraComponent>();

            auto* position = entity->getComponent<PositionComponent>();
            position->scale.y = SCALED_CUBE_SIZE;
            position->hitbox = SDL_Rect{0, 0, SCALED_CUBE_SIZE, SCALED_CUBE_SIZE};
            position->position.y += SCALED_CUBE_SIZE;

            int shellCoordinate = getReferenceEnemyIDAsEntity(entityID, 77);
            entity->getComponent<SpritesheetComponent>()->setSpritesheetCoordinates(
                Map::EnemyIDCoordinates.at(shellCoordinate));

            entity->remove<AnimationComponent>();
         });

         entity->addComponent<EnemyComponent>(EnemyType::KOOPA);

      } break;
      /* ****************************************************************** */
      case 44: {  // PIRHANNA PLANT
         Entity* pirhanna(world->create());

         auto* position = pirhanna->addComponent<PositionComponent>(
             Vector2f(coordinateX * SCALED_CUBE_SIZE + SCALED_CUBE_SIZE / 2,
                      coordinateY * SCALED_CUBE_SIZE),
             Vector2i(SCALED_CUBE_SIZE, SCALED_CUBE_SIZE * 2), SDL_Rect{24, 48, 16, 16});

         pirhanna->addComponent<TextureComponent>(scene->enemyTexture);

         pirhanna->addComponent<SpritesheetComponent>(ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE * 2, 1,
                                                      1, 0, ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE,
                                                      Map::EnemyIDCoordinates.at(entityID));

         pirhanna->addComponent<AnimationComponent>(std::vector<int>{entityID, entityID + 1}, 4,
                                                    Map::EnemyIDCoordinates);

         pirhanna->addComponent<MovingComponent>(Vector2f(0, 0), Vector2f(0, 0));

         pirhanna->addComponent<MoveOutsideCameraComponent>();

         pirhanna->addComponent<CollisionExemptComponent>();

         pirhanna->addComponent<FrictionExemptComponent>();

         pirhanna->addComponent<EnemyComponent>(EnemyType::PIRANHA_PLANT);

         auto* piranhaComponent = pirhanna->addComponent<PiranhaPlantComponent>();

         piranhaComponent->pipeCoordinates =
             Vector2f(position->position.x, position->position.y + SCALED_CUBE_SIZE * 2);

         pirhanna->addComponent<TimerComponent>(
             [](Entity* entity) {
                auto* piranha = entity->getComponent<PiranhaPlantComponent>();

                if (piranha->inPipe) {
                   entity->getComponent<MovingComponent>()->velocity.y = -1;

                   entity->addComponent<WaitUntilComponent>(
                       [=](Entity* entity) {
                          return entity->getComponent<PositionComponent>()->position.y <=
                                 piranha->pipeCoordinates.y - SCALED_CUBE_SIZE * 2;
                       },
                       [=](Entity* entity) {
                          entity->getComponent<PositionComponent>()->setBottom(
                              piranha->pipeCoordinates.y);

                          entity->getComponent<MovingComponent>()->velocity.y = 0;

                          entity->remove<WaitUntilComponent>();
                       });

                   piranha->inPipe = false;
                } else {
                   entity->getComponent<MovingComponent>()->velocity.y = 1;

                   entity->addComponent<WaitUntilComponent>(
                       [=](Entity* entity) {
                          return entity->getComponent<PositionComponent>()->position.y >=
                                 piranha->pipeCoordinates.y;
                       },
                       [=](Entity* entity) {
                          entity->getComponent<PositionComponent>()->setTop(
                              piranha->pipeCoordinates.y);

                          entity->getComponent<MovingComponent>()->velocity.y = 0;

                          entity->remove<WaitUntilComponent>();
                       });

                   piranha->inPipe = true;
                }
             },
             3 * MAX_FPS);
      } break;
      /* ****************************************************************** */
      case 48: {  // BLOOPER
         Entity* blooper(world->create());

         auto* position = blooper->addComponent<PositionComponent>(
             Vector2f(coordinateX, coordinateY) * SCALED_CUBE_SIZE,
             Vector2i(SCALED_CUBE_SIZE, SCALED_CUBE_SIZE * 2),
             SDL_Rect{0, SCALED_CUBE_SIZE / 2, SCALED_CUBE_SIZE, SCALED_CUBE_SIZE});

         blooper->addComponent<TextureComponent>(scene->enemyTexture);

         blooper->addComponent<SpritesheetComponent>(ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE * 2, 1,
                                                     1, 0, ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE,
                                                     Map::EnemyIDCoordinates.at(entityID));

         blooper->addComponent<AnimationComponent>(std::vector<int>{entityID, entityID + 1}, 2,
                                                   Map::EnemyIDCoordinates);

         auto* move =
             blooper->addComponent<MovingComponent>(Vector2f(0, 0), Vector2f(0, -0.47480f));

         blooper->addComponent<GravityComponent>();

         blooper->addComponent<FrictionExemptComponent>();

         blooper->addComponent<CollisionExemptComponent>();

         blooper->addComponent<TimerComponent>(
             [=](Entity* entity) {
                if (!Camera::Get().inCameraRange(position)) {
                   return;
                }

                entity->remove<GravityComponent>();

                move->acceleration.y = 0;

                auto* playerPosition =
                    world->findFirst<PlayerComponent>()->getComponent<PositionComponent>();

                move->velocity.x = (playerPosition->position.x > position->position.x) ? 3.0 : -3.0;

                move->velocity.y =
                    (position->position.y < Camera::Get().getCameraCenterY()) ? 3.0 : -3.0;

                entity->addComponent<CallbackComponent>(
                    [=](Entity* entity) {
                       entity->addComponent<GravityComponent>();

                       move->velocity.x = move->velocity.y = 0;
                       move->acceleration.y = -0.47480f;
                    },
                    MAX_FPS / 2);
             },
             MAX_FPS);

         blooper->addComponent<EnemyComponent>(EnemyType::BLOOPER);
      } break;
      /* ****************************************************************** */
      case 50: {  // LAKITU
         Entity* lakitu(world->create());

         lakitu->addComponent<PositionComponent>(
             Vector2f(coordinateX, coordinateY) * SCALED_CUBE_SIZE,
             Vector2i(SCALED_CUBE_SIZE, SCALED_CUBE_SIZE * 2));

         lakitu->addComponent<TextureComponent>(scene->enemyTexture);

         lakitu->addComponent<SpritesheetComponent>(ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE * 2, 1,
                                                    1, 0, ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE,
                                                    Map::EnemyIDCoordinates.at(entityID));

         lakitu->addComponent<MovingComponent>(Vector2f(0, 0), Vector2f(0, 0));

         lakitu->addComponent<FrictionExemptComponent>();

         lakitu->addComponent<CrushableComponent>([](Entity* entity) {
            entity->getComponent<TextureComponent>()->setVerticalFlipped(true);
            entity->addComponent<DeadComponent>();
            entity->addComponent<GravityComponent>();
            entity->addComponent<ParticleComponent>();
         });

         lakitu->addComponent<LakituComponent>();

         lakitu->addComponent<EnemyComponent>(EnemyType::LAKITU);

         int inCloudID = getReferenceEnemyIDAsEntity(entityID, 86);

         std::function<Entity*(Entity*)> createSpine = [=](Entity* entity) {
            auto* position = entity->getComponent<PositionComponent>();
            auto* texture = entity->getComponent<TextureComponent>();

            Entity* spine(world->create());

            spine->addComponent<PositionComponent>(position->position,
                                                   Vector2i(SCALED_CUBE_SIZE, SCALED_CUBE_SIZE));

            spine->addComponent<TextureComponent>(scene->enemyTexture);

            spine->addComponent<SpritesheetComponent>(ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE, 1, 1,
                                                      0, ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE,
                                                      Map::EnemyIDCoordinates.at(500));

            spine->addComponent<AnimationComponent>(std::vector<int>{500, 501}, 4,
                                                    Map::EnemyIDCoordinates);

            spine->addComponent<MovingComponent>(
                Vector2f((texture->isHorizontalFlipped()) ? 2.5 : -2.5, 0), Vector2f(0, 0));

            spine->addComponent<GravityComponent>();

            spine->addComponent<DestroyOutsideCameraComponent>();

            spine->addComponent<EnemyComponent>(EnemyType::SPINE);

            return spine;
         };

         std::function<void(Entity*)> throwSpine = [=](Entity* entity) {
            auto* position = entity->getComponent<PositionComponent>();
            auto* spritesheet = entity->getComponent<SpritesheetComponent>();

            CommandScheduler::getInstance().addCommand(new SequenceCommand(std::vector<Command*>{
                /* Set Lakitu to be in the cloud */
                new RunCommand([=]() {
                   position->scale.y = SCALED_CUBE_SIZE;
                   position->position.y += SCALED_CUBE_SIZE;

                   spritesheet->setEntityHeight(ORIGINAL_CUBE_SIZE);
                   spritesheet->setSpritesheetCoordinates(Map::EnemyIDCoordinates.at(inCloudID));
                }),
                new WaitCommand(0.75),
                /* Move out of the cloud and launch a spine */
                new RunCommand([=]() {
                   position->scale.y = SCALED_CUBE_SIZE * 2;
                   position->position.y -= SCALED_CUBE_SIZE;

                   spritesheet->setEntityHeight(ORIGINAL_CUBE_SIZE * 2);
                   spritesheet->setSpritesheetCoordinates(Map::EnemyIDCoordinates.at(entityID));

                   createSpine(entity);
                }),
            }));
         };

         lakitu->addComponent<TimerComponent>(throwSpine, 3 * MAX_FPS);
      } break;
      /* ****************************************************************** */
      case 61: {  // BOWSER
         Entity* bowser(world->create());

         auto* position = bowser->addComponent<PositionComponent>(
             Vector2f(coordinateX, coordinateY) * SCALED_CUBE_SIZE, Vector2i(SCALED_CUBE_SIZE * 2));

         auto* texture = bowser->addComponent<TextureComponent>(scene->enemyTexture);

         bowser->addComponent<SpritesheetComponent>(ORIGINAL_CUBE_SIZE * 2, ORIGINAL_CUBE_SIZE * 2,
                                                    1, 1, 0, ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE,
                                                    Map::EnemyIDCoordinates.at(entityID));

         auto* move = bowser->addComponent<MovingComponent>(Vector2f(0, 0), Vector2f(0, -0.3));

         bowser->addComponent<GravityComponent>();
         bowser->addComponent<FrictionExemptComponent>();

         int mouthOpenID = getReferenceEnemyIDAsEntity(entityID, 61);
         int mouthClosedID = getReferenceEnemyIDAsEntity(entityID, 65);

         int hammerID = getReferenceEnemyIDAsEntity(entityID, 60);

         std::vector<int> mouthOpenAnimation = {mouthOpenID, mouthOpenID + 2};
         std::vector<int> mouthClosedAnimation = {mouthClosedID, mouthClosedID + 2};

         auto* animation = bowser->addComponent<AnimationComponent>(mouthOpenAnimation, 2,
                                                                    Map::EnemyIDCoordinates);

         std::vector<std::function<void(Entity*)>> bowserMovements = {
             [=](Entity* entity) {  // MOVE
                auto* bowserComponent = entity->getComponent<BowserComponent>();

                if (bowserComponent->lastMoveDirection == Direction::LEFT) {
                   move->velocity.x = 1.0;
                   bowserComponent->lastMoveDirection = Direction::RIGHT;
                } else {
                   move->velocity.x = 1.0;
                   bowserComponent->lastMoveDirection = Direction::LEFT;
                }
                bowserComponent->lastMoveTime = 0;
             },
             [=](Entity* entity) {  // STOP
                move->velocity.x = 0;
                entity->getComponent<BowserComponent>()->lastStopTime = 0;
             },
             [=](Entity* entity) {  // JUMP
                auto* bowserComponent = entity->getComponent<BowserComponent>();

                move->velocity.y = -5.0;
                move->acceleration.y = -0.35;
                bowserComponent->lastJumpTime = 0;
             },
         };

         std::vector<std::function<void(Entity*, int number)>> bowserAttacks = {
             [=](Entity* entity, int number) {  // LAUNCH FIRE
                auto* bowserComponent = entity->getComponent<BowserComponent>();

                animation->frameIDS = mouthClosedAnimation;
                entity->addComponent<CallbackComponent>(
                    [=](Entity* entity, int number = 0) {
                       animation->frameIDS = mouthOpenAnimation;

                       Entity* blastSound(world->create());
                       blastSound->addComponent<SoundComponent>(SoundID::BOWSER_FIRE);

                       Entity* fireBlast(world->create());

                       auto* blastPosition = fireBlast->addComponent<PositionComponent>(
                           Vector2f(0, position->getTop() + 4),
                           Vector2i(SCALED_CUBE_SIZE + SCALED_CUBE_SIZE / 2, SCALED_CUBE_SIZE / 2));

                       auto* blastTexture =
                           fireBlast->addComponent<TextureComponent>(scene->enemyTexture);

                       fireBlast->addComponent<SpritesheetComponent>(
                           ORIGINAL_CUBE_SIZE + ORIGINAL_CUBE_SIZE / 2, ORIGINAL_CUBE_SIZE / 2, 1,
                           1, 0, ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE,
                           Map::EnemyIDCoordinates.at(470));

                       fireBlast->addComponent<AnimationComponent>(std::vector<int>{470, 505}, 16,
                                                                   Map::EnemyIDCoordinates);

                       auto* blastMove =
                           fireBlast->addComponent<MovingComponent>(Vector2f(0, 0), Vector2f(0, 0));

                       fireBlast->addComponent<FrictionExemptComponent>();

                       fireBlast->addComponent<DestroyOutsideCameraComponent>();

                       if (texture->isHorizontalFlipped()) {
                          blastMove->velocity.x = 3.0;
                          blastPosition->setLeft(position->getRight());
                          blastTexture->setHorizontalFlipped(true);
                       } else {
                          blastMove->velocity.x = -3.0;
                          blastPosition->setRight(position->getLeft());
                          blastTexture->setHorizontalFlipped(false);
                       }

                       fireBlast->addComponent<ProjectileComponent>(ProjectileType::OTHER);
                    },
                    MAX_FPS * 2);
                bowserComponent->lastAttackTime = 0;
             },
             [=](Entity* entity, int number = 0) {  // THROW HAMMERS
                auto* bowserComponent = entity->getComponent<BowserComponent>();

                for (int i = 0; i < number; i++) {
                   Entity* hammer(world->create());
                   hammer->addComponent<CallbackComponent>(
                       [=](Entity* hammer) {
                          auto* hammerPosition = hammer->addComponent<PositionComponent>(
                              Vector2f(position->getLeft(), position->getTop()),
                              Vector2i(SCALED_CUBE_SIZE));

                          if (texture->isHorizontalFlipped()) {
                             hammerPosition->setLeft(position->getRight());
                          } else {
                             hammerPosition->setRight(position->getLeft());
                          }

                          hammer->addComponent<TextureComponent>(scene->enemyTexture);

                          hammer->addComponent<SpritesheetComponent>(
                              ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE, 1, 1, 0, ORIGINAL_CUBE_SIZE,
                              ORIGINAL_CUBE_SIZE, Map::EnemyIDCoordinates.at(hammerID));

                          float randomXVelocity =
                              -((float)((float)rand() / (float)RAND_MAX) + 2.25);
                          float randomYVelocity =
                              -((float)((float)rand() / (float)RAND_MAX) * 0.5 + 6);

                          if (texture->isHorizontalFlipped()) {
                             randomXVelocity *= -1;
                          }

                          hammer->addComponent<MovingComponent>(
                              Vector2f(randomXVelocity, randomYVelocity), Vector2f(0, -0.35));

                          hammer->addComponent<FrictionExemptComponent>();

                          hammer->addComponent<GravityComponent>();

                          hammer->addComponent<DestroyOutsideCameraComponent>();

                          hammer->addComponent<ProjectileComponent>(ProjectileType::OTHER);

                          hammer->addComponent<ParticleComponent>();
                       },
                       i * 4);
                }
                bowserComponent->lastAttackTime = 0;
             },
         };

         bowser->addComponent<BowserComponent>(bowserAttacks, bowserMovements);

         bowser->addComponent<EnemyComponent>(EnemyType::BOWSER);
      } break;
      /* ****************************************************************** */
      case 70: {  // GOOMBA
         Entity* entity(world->create());

         entity->addComponent<PositionComponent>(
             Vector2f(coordinateX, coordinateY) * SCALED_CUBE_SIZE, Vector2i(SCALED_CUBE_SIZE));

         entity->addComponent<TextureComponent>(scene->enemyTexture);

         entity->addComponent<SpritesheetComponent>(ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE, 1, 1, 0,
                                                    ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE,
                                                    Map::EnemyIDCoordinates.at(entityID));

         int firstAnimationID = entityID;

         entity->addComponent<AnimationComponent>(
             std::vector<int>{firstAnimationID, firstAnimationID + 1}, 6, Map::EnemyIDCoordinates);

         entity->addComponent<MovingComponent>(Vector2f(-ENEMY_SPEED, 0), Vector2f(0, 0));

         entity->addComponent<CrushableComponent>([=](Entity* entity) {
            int deadEnemyID = getReferenceEnemyIDAsEntity(entityID, 72);

            entity->getComponent<SpritesheetComponent>()->setSpritesheetCoordinates(
                Map::EnemyIDCoordinates.at(deadEnemyID));

            entity->addComponent<DeadComponent>();

            entity->remove<AnimationComponent>();

            entity->remove<MovingComponent>();

            entity->addComponent<DestroyDelayedComponent>(20);
         });

         entity->addComponent<GravityComponent>();

         entity->addComponent<EnemyComponent>(EnemyType::GOOMBA);
      } break;
      /* ****************************************************************** */
      case 71: {  // GOOMBA (shifted to the right)
         Entity* entity(world->create());

         entity->addComponent<PositionComponent>(
             Vector2f((coordinateX * SCALED_CUBE_SIZE) + ORIGINAL_CUBE_SIZE,
                      coordinateY * SCALED_CUBE_SIZE),
             Vector2i(SCALED_CUBE_SIZE));

         entity->addComponent<TextureComponent>(scene->enemyTexture);

         entity->addComponent<SpritesheetComponent>(ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE, 1, 1, 0,
                                                    ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE,
                                                    Map::EnemyIDCoordinates.at(entityID - 1));

         int firstAnimationID = entityID - 1;

         entity->addComponent<AnimationComponent>(
             std::vector<int>{firstAnimationID, firstAnimationID + 1}, 6, Map::EnemyIDCoordinates);

         entity->addComponent<MovingComponent>(Vector2f(-ENEMY_SPEED, 0), Vector2f(0, 0));

         entity->addComponent<CrushableComponent>([=](Entity* entity) {
            int deadEnemyID = getReferenceEnemyIDAsEntity(entityID, 72);

            entity->getComponent<SpritesheetComponent>()->setSpritesheetCoordinates(
                Map::EnemyIDCoordinates.at(deadEnemyID));

            entity->addComponent<DeadComponent>();

            entity->remove<AnimationComponent>();

            entity->remove<MovingComponent>();

            entity->addComponent<DestroyDelayedComponent>(20);
         });

         entity->addComponent<GravityComponent>();

         entity->addComponent<EnemyComponent>(EnemyType::GOOMBA);
      } break;
      /* ****************************************************************** */
      case 81: {  // CHEEP CHEEP
         Entity* entity(world->create());

         entity->addComponent<PositionComponent>(
             Vector2f(coordinateX, coordinateY) * SCALED_CUBE_SIZE, Vector2i(SCALED_CUBE_SIZE));

         entity->addComponent<TextureComponent>(scene->enemyTexture);

         entity->addComponent<SpritesheetComponent>(ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE, 1, 1, 0,
                                                    ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE,
                                                    Map::EnemyIDCoordinates.at(entityID));

         entity->addComponent<AnimationComponent>(std::vector<int>{entityID, entityID + 1}, 6,
                                                  Map::EnemyIDCoordinates);

         entity->addComponent<MovingComponent>(Vector2f(-ENEMY_SPEED, 0), Vector2f(0, 0));

         entity->addComponent<CollisionExemptComponent>();

         entity->addComponent<FrictionExemptComponent>();

         entity->addComponent<EnemyComponent>(EnemyType::CHEEP_CHEEP);
      } break;
      /* ****************************************************************** */
      case 455: {  // NORMAL KOOPA (RED)
         Entity* entity(world->create());

         entity->addComponent<PositionComponent>(
             Vector2f(coordinateX, coordinateY) * SCALED_CUBE_SIZE,
             Vector2i(SCALED_CUBE_SIZE, SCALED_CUBE_SIZE * 2),
             SDL_Rect{0, SCALED_CUBE_SIZE, SCALED_CUBE_SIZE, SCALED_CUBE_SIZE});

         entity->addComponent<TextureComponent>(scene->enemyTexture);

         entity->addComponent<SpritesheetComponent>(ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE * 2, 1,
                                                    1, 0, ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE,
                                                    Map::EnemyIDCoordinates.at(entityID));

         int firstAnimationID = entityID;

         entity->addComponent<AnimationComponent>(
             std::vector<int>{firstAnimationID, firstAnimationID + 1}, 6, Map::EnemyIDCoordinates);

         entity->addComponent<MovingComponent>(Vector2f(-ENEMY_SPEED, 0), Vector2f(0, 0));

         entity->addComponent<GravityComponent>();

         entity->addComponent<CrushableComponent>([=](Entity* entity) {
            entity->getComponent<EnemyComponent>()->enemyType = EnemyType::KOOPA_SHELL;
            entity->getComponent<MovingComponent>()->velocity.x = 0.0;
            entity->getComponent<SpritesheetComponent>()->setEntityHeight(ORIGINAL_CUBE_SIZE);

            entity->addComponent<DestroyOutsideCameraComponent>();

            auto* position = entity->getComponent<PositionComponent>();
            position->scale.y = SCALED_CUBE_SIZE;
            position->hitbox = SDL_Rect{0, 0, SCALED_CUBE_SIZE, SCALED_CUBE_SIZE};
            position->position.y += SCALED_CUBE_SIZE;

            int shellCoordinate = 494;
            entity->getComponent<SpritesheetComponent>()->setSpritesheetCoordinates(
                Map::EnemyIDCoordinates.at(shellCoordinate));

            entity->remove<AnimationComponent>();
         });

         entity->addComponent<EnemyComponent>(EnemyType::KOOPA);
      } break;
      /* ****************************************************************** */
      case 498: {  // CHEEP CHEEP (RED)
         if (scene->backgroundMap.getLevelData()[coordinateY][coordinateX] ==
             186) {  // If underwater
            Entity* entity(world->create());

            entity->addComponent<PositionComponent>(
                Vector2f(coordinateX, coordinateY) * SCALED_CUBE_SIZE, Vector2i(SCALED_CUBE_SIZE));

            entity->addComponent<TextureComponent>(scene->enemyTexture);

            entity->addComponent<SpritesheetComponent>(ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE, 1, 1,
                                                       0, ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE,
                                                       Map::EnemyIDCoordinates.at(entityID));

            entity->addComponent<AnimationComponent>(std::vector<int>{entityID, entityID + 1}, 6,
                                                     Map::EnemyIDCoordinates);

            entity->addComponent<MovingComponent>(Vector2f(-ENEMY_SPEED, 0), Vector2f(0, 0));

            entity->addComponent<CollisionExemptComponent>();

            entity->addComponent<FrictionExemptComponent>();

            entity->addComponent<EnemyComponent>(EnemyType::CHEEP_CHEEP);
         } else {  // If in those flying patches of them idk what they're called
            Entity* entity(world->create());

            auto* position = entity->addComponent<PositionComponent>(
                Vector2f(coordinateX, coordinateY) * SCALED_CUBE_SIZE, Vector2i(SCALED_CUBE_SIZE));

            entity->addComponent<TextureComponent>(scene->enemyTexture, true);

            entity->addComponent<SpritesheetComponent>(ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE, 1, 1,
                                                       0, ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE,
                                                       Map::EnemyIDCoordinates.at(entityID));

            entity->addComponent<AnimationComponent>(std::vector<int>{entityID, entityID + 1}, 6,
                                                     Map::EnemyIDCoordinates);

            auto* move = entity->addComponent<MovingComponent>(Vector2f(0, 0), Vector2f(0, 0));

            entity->addComponent<MoveOutsideCameraComponent>();

            entity->addComponent<GravityComponent>();

            entity->addComponent<CollisionExemptComponent>();

            entity->addComponent<FrictionExemptComponent>();

            entity->addComponent<TimerComponent>(
                [=](Entity* entity) {
                   entity->addComponent<CallbackComponent>(
                       [&](Entity* entity) {
                          if (Camera::Get().inCameraYRange(position)) {
                             return;
                          }
                          position->position =
                              Vector2f(coordinateX, coordinateY) * SCALED_CUBE_SIZE;

                          if (!Camera::Get().inCameraXRange(position)) {
                             return;
                          }

                          move->velocity.x = generateRandomNumber(2.0f, 4.0f);

                          move->velocity.y = -10.0;

                          move->acceleration.y = -0.4542;
                       },
                       MAX_FPS * generateRandomNumber(0.5, 1.5));
                },
                MAX_FPS * 2.5);

            entity->addComponent<EnemyComponent>(EnemyType::CHEEP_CHEEP);
         }
      } break;
      /* ****************************************************************** */
      case 504: {  // LAVA BUBBLE
         Entity* entity(world->create());

         entity->addComponent<PositionComponent>(
             Vector2f(coordinateX, coordinateY + 1) * SCALED_CUBE_SIZE, Vector2i(SCALED_CUBE_SIZE));

         int resetYLevel = (coordinateY + 1) * SCALED_CUBE_SIZE;

         entity->addComponent<TextureComponent>(scene->enemyTexture);

         entity->addComponent<SpritesheetComponent>(ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE, 1, 1, 0,
                                                    ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE,
                                                    Map::EnemyIDCoordinates.at(entityID));

         entity->addComponent<MovingComponent>(Vector2f(0, 0), Vector2f(0, 0));

         entity->addComponent<GravityComponent>();

         entity->addComponent<TimerComponent>(
             [resetYLevel](Entity* entity) {
                entity->getComponent<PositionComponent>()->position.y = resetYLevel;
                entity->getComponent<MovingComponent>()->velocity.y = -10.0;
                entity->getComponent<MovingComponent>()->acceleration.y = -0.40;
             },
             MAX_FPS * 6);

         entity->addComponent<EnemyComponent>(EnemyType::LAVA_BUBBLE);
      } break;
      /* ****************************************************************** */
      default: {
         //         Entity* entity(world->create());
         //
         //         entity->addComponent<PositionComponent>(
         //             Vector2f(coordinateX * SCALED_CUBE_SIZE, coordinateY * SCALED_CUBE_SIZE),
         //             Vector2i(SCALED_CUBE_SIZE, SCALED_CUBE_SIZE));
         //
         //         entity->addComponent<TextureComponent>(scene->enemyTexture, ORIGINAL_CUBE_SIZE,
         //                                                ORIGINAL_CUBE_SIZE, 1, 1, 0,
         //                                                ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE,
         //                                                Map::EnemyIDCoordinates.at(entityID),
         //                                                false, false);
         //
         //         entity->addComponent<MovingComponent>(-ENEMY_SPEED, 0, 0, 0);
         //
         //         entity->addComponent<GravityComponent>();
      } break;
   }
}

void MapSystem::createFireBarEntities(World* world) {
   for (unsigned int i = 0; i < scene->foregroundMap.getLevelData().size(); i++) {
      for (unsigned int j = 0; j < scene->foregroundMap.getLevelData()[0].size(); j++) {
         auto fireBarCoordinate =
             getFireBarCoordinate(scene->getLevelData().fireBarLocations, Vector2i(j, i));

         if (std::get<0>(fireBarCoordinate) != Vector2i(0, 0)) {
            float startAngle = (float)std::get<1>(fireBarCoordinate);
            RotationDirection rotationDirection = std::get<2>(fireBarCoordinate);
            int barLength = std::get<3>(fireBarCoordinate);

            for (int bar = 0; bar < barLength; bar++) {
               Entity* barElement(world->create());

               barElement->addComponent<PositionComponent>(
                   Vector2f(j, i) * SCALED_CUBE_SIZE, Vector2i(SCALED_CUBE_SIZE),
                   SDL_Rect{0, 0, SCALED_CUBE_SIZE / 4, SCALED_CUBE_SIZE / 4});

               barElement->addComponent<TextureComponent>(scene->blockTexture, false, false);

               barElement->addComponent<SpritesheetComponent>(
                   ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE, 1, 1, 1, ORIGINAL_CUBE_SIZE,
                   ORIGINAL_CUBE_SIZE, Map::BlockIDCoordinates.at(611));

               barElement->addComponent<AnimationComponent>(std::vector<int>{611, 612, 613, 614},
                                                            12, Map::BlockIDCoordinates);

               barElement->addComponent<FireBarComponent>(Vector2f(j, i) * SCALED_CUBE_SIZE,
                                                          bar * ORIGINAL_CUBE_SIZE, startAngle,
                                                          rotationDirection);

               barElement->addComponent<TimerComponent>(
                   [&](Entity* entity) {
                      auto* barComponent = entity->getComponent<FireBarComponent>();

                      switch (barComponent->direction) {
                         case RotationDirection::CLOCKWISE:
                            barComponent->barAngle -= 10;
                            break;
                         case RotationDirection::COUNTER_CLOCKWISE:
                            barComponent->barAngle += 10;
                            break;
                         default:
                            break;
                      }
                   },
                   10);

               if (bar != barLength - 1) {
                  barElement->addComponent<EnemyComponent>(EnemyType::FIRE_BAR);
               }

               barElement->addComponent<ForegroundComponent>();
            }
         }
      }
   }
}

void MapSystem::loadEntities(World* world) {
   auto blockTexture = scene->blockTexture;
   auto enemyTexture = scene->enemyTexture;

   for (unsigned i = 0; i < scene->backgroundMap.getLevelData().size(); i++) {
      for (unsigned j = 0; j < scene->backgroundMap.getLevelData()[0].size(); j++) {
         int entityID = scene->backgroundMap.getLevelData()[i][j];
         switch (entityID) {
            case -1:
               break;
            case 391:
            case 393:
               break;
            default: {
               Entity* entity(world->create());

               entity->addComponent<PositionComponent>(Vector2f(j, i) * SCALED_CUBE_SIZE,
                                                       Vector2i(SCALED_CUBE_SIZE));

               entity->addComponent<TextureComponent>(blockTexture, false, false);

               entity->addComponent<SpritesheetComponent>(
                   ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE, 1, 1, 1, ORIGINAL_CUBE_SIZE,
                   ORIGINAL_CUBE_SIZE, Map::BlockIDCoordinates.at(entityID));

               entity->addComponent<BackgroundComponent>();
            } break;
         }
      }
   }
   for (unsigned i = 0; i < scene->undergroundMap.getLevelData().size(); i++) {
      for (unsigned j = 0; j < scene->undergroundMap.getLevelData()[0].size(); j++) {
         int entityID = scene->undergroundMap.getLevelData()[i][j];
         int referenceID = getReferenceBlockID(entityID);

         createForegroundEntities(world, j, i, entityID, referenceID);
      }
   }
   for (unsigned i = 0; i < scene->foregroundMap.getLevelData().size(); i++) {
      for (unsigned j = 0; j < scene->foregroundMap.getLevelData()[0].size(); j++) {
         int entityID = scene->foregroundMap.getLevelData()[i][j];
         int referenceID = getReferenceBlockID(entityID);

         createForegroundEntities(world, j, i, entityID, referenceID, true);
      }
   }

   createFireBarEntities(world);

   for (unsigned i = 0; i < scene->enemiesMap.getLevelData().size(); i++) {
      for (unsigned j = 0; j < scene->enemiesMap.getLevelData()[0].size(); j++) {
         int entityID = scene->enemiesMap.getLevelData()[i][j];
         int referenceID = getReferenceEnemyID(entityID);
         switch (entityID) {
            case -1:
               break;
            case 73:
            case 79:
            case 83:
            case 85:
            case 91:
            case 490:
            case 492:
            case 496:
               break;
            default:
               createEnemyEntities(world, j, i, entityID, referenceID);
               break;
         }
      }
   }

   for (unsigned int i = 0; i < scene->aboveForegroundMap.getLevelData().size(); i++) {
      for (unsigned int j = 0; j < scene->aboveForegroundMap.getLevelData()[0].size(); j++) {
         int entityID = scene->aboveForegroundMap.getLevelData()[i][j];
         int referenceID = getReferenceBlockID(entityID);
         switch (referenceID) {
            case -1:
               break;
            case 150:
            case 292: {  // WARP PIPE
               Entity* entity(world->create());

               auto* position = entity->addComponent<PositionComponent>(
                   Vector2f(j, i) * SCALED_CUBE_SIZE, Vector2i(SCALED_CUBE_SIZE));

               entity->addComponent<TextureComponent>(blockTexture, false, false);

               entity->addComponent<SpritesheetComponent>(
                   ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE, 1, 1, 1, ORIGINAL_CUBE_SIZE,
                   ORIGINAL_CUBE_SIZE, Map::BlockIDCoordinates.at(entityID));

               auto pipe =
                   getPipeCoordinate(scene->getLevelData().warpPipeLocations, Vector2i(j, i));

               if (std::get<2>(pipe) != Direction::NONE) {
                  Vector2i playerCoordinates = std::get<0>(pipe);
                  Vector2i cameraCoordinates = std::get<1>(pipe);
                  Direction inDirection = std::get<2>(pipe);
                  Direction outDirection = std::get<3>(pipe);
                  bool cameraFreeze = std::get<4>(pipe);
                  BackgroundColor color = std::get<5>(pipe);
                  LevelType levelType = std::get<6>(pipe);
                  Vector2i newLevel = std::get<7>(pipe);

                  entity->addComponent<WarpPipeComponent>(playerCoordinates, cameraCoordinates,
                                                          inDirection, outDirection, cameraFreeze,
                                                          color, levelType, newLevel);

                  switch (inDirection) {
                     case Direction::UP:
                        position->hitbox.x = 32;
                        position->hitbox.w = 0;
                        break;
                     case Direction::DOWN:
                        position->hitbox.x = 32;
                        position->hitbox.w = 0;
                        break;
                     case Direction::LEFT:
                        position->hitbox.y = 32;
                        position->hitbox.h = 0;
                        break;
                     case Direction::RIGHT:
                        position->hitbox.y = 32;
                        position->hitbox.h = 0;
                        break;
                     default:
                        break;
                  }
               }

               entity->addComponent<AboveForegroundComponent>();
            } break;
            default: {
               Entity* entity(world->create());

               entity->addComponent<PositionComponent>(Vector2f(j, i) * SCALED_CUBE_SIZE,
                                                       Vector2i(SCALED_CUBE_SIZE));

               entity->addComponent<TextureComponent>(blockTexture, false, false);

               entity->addComponent<SpritesheetComponent>(
                   ORIGINAL_CUBE_SIZE, ORIGINAL_CUBE_SIZE, 1, 1, 1, ORIGINAL_CUBE_SIZE,
                   ORIGINAL_CUBE_SIZE, Map::BlockIDCoordinates.at(entityID));

               entity->addComponent<AboveForegroundComponent>();
            } break;
         }
      }
   }

   // Floating text
   for (auto floatingText : scene->getLevelData().floatingTextLocations) {
      Entity* text(world->create());

      text->addComponent<PositionComponent>(
          std::get<0>(floatingText).convertTo<float>() * SCALED_CUBE_SIZE, Vector2i());

      text->addComponent<TextComponent>(std::get<1>(floatingText), 16, true);

      text->addComponent<FloatingTextComponent>();
   }

   // Set the camera max (i don't know where to put this)
   Camera::Get().setCameraMaxX(scene->getLevelData().cameraMax * SCALED_CUBE_SIZE);
}

void MapSystem::loadEntities() {
   loadEntities(scene->getWorld());
}

void MapSystem::hideGameEntities(World* world) {
   world->find<TextureComponent>([](Entity* entity) {
      if (entity->hasComponent<IconComponent>()) {
         return;
      }
      entity->getComponent<TextureComponent>()->setVisible(false);
   });
}

void MapSystem::showGameEntities(World* world) {
   world->find<TextureComponent>([](Entity* entity) {
      if (entity->hasComponent<IconComponent>()) {
         return;
      }
      entity->getComponent<TextureComponent>()->setVisible(true);
   });
}
