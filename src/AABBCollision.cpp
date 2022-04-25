#include "AABBCollision.h"

#include "Constants.h"

#include <cmath>

// Direction checkCollisionY(Entity* solid, PositionComponent* position, MovingComponent* move) {
//   auto solidPosition = solid->getComponent<PositionComponent>();
//   Direction direction = Direction::NONE;
//
//   if (move->velocityY >= 0.0f) {
//      // If falling
//      if (AABBTotalCollision(
//              position->position.x + (TILE_ROUNDNESS / 2), position->position.y + move->velocityY,
//              position->scale.x - TILE_ROUNDNESS, position->scale.y,
//              solidPosition->position.x + (TILE_ROUNDNESS / 2), solidPosition->position.y,
//              solidPosition->scale.x - TILE_ROUNDNESS, solidPosition->scale.y)) {
//         float topDistance =
//             std::abs(solidPosition->getTop() - (position->getBottom() + move->velocityY));
//         float bottomDistance =
//             std::abs((position->getTop() + move->velocityY) - solidPosition->getBottom());
//
//         if (topDistance < bottomDistance) {
//            position->setBottom(solidPosition->getTop());
//            solid->addComponent<TopCollisionComponent>();
//            direction = Direction::BOTTOM;
//         }
//      }
//   } else {
//      // Jumping
//      if (AABBTotalCollision(
//              position->position.x + TILE_ROUNDNESS, position->position.y + move->velocityY,
//              position->scale.x - (TILE_ROUNDNESS * 2), position->scale.y,
//              solidPosition->position.x + TILE_ROUNDNESS, solidPosition->position.y,
//              solidPosition->scale.x - (TILE_ROUNDNESS * 2), solidPosition->scale.y)) {
//         float topDistance =
//             std::abs(solidPosition->getTop() - (position->getBottom() + move->velocityY));
//         float bottomDistance =
//             std::abs((position->getTop() + move->velocityY) - solidPosition->getBottom());
//         if (topDistance > bottomDistance) {
//            position->setTop(solidPosition->getBottom());
//            solid->addComponent<BottomCollisionComponent>();
//            direction = Direction::TOP;
//         }
//      }
//   }
//   return direction;
//}
//
// Direction checkCollisionX(Entity* solid, PositionComponent* position, MovingComponent* move) {
//   auto solidPosition = solid->getComponent<PositionComponent>();
//   Direction direction = Direction::NONE;
//
//   if (AABBTotalCollision(position->position.x + move->velocityX, position->position.y,
//                          position->scale.x, position->scale.y - (TILE_ROUNDNESS * 2),
//                          solidPosition)) {
//      float leftDistance =
//          std::abs((position->getLeft() + move->velocityX) - solidPosition->getRight());
//      float rightDistance =
//          std::abs((position->getRight() + move->velocityX) - solidPosition->getLeft());
//      if (leftDistance < rightDistance) {
//         // Entity is inside block, push out
//         if (position->getLeft() < solidPosition->getRight()) {
//            position->position.x += std::min(0.5f, solidPosition->getRight() -
//            position->getLeft());
//         } else {
//            // The entity is about to get inside the block
//            position->setLeft(solidPosition->getRight());
//         }
//         solid->addComponent<RightCollisionComponent>();
//         direction = Direction::LEFT;
//      } else {
//         // Entity is inside block, push out
//         if (position->getRight() > solidPosition->getLeft()) {
//            position->position.x -= std::min(0.5f, position->getRight() -
//            solidPosition->getLeft());
//         } else {
//            // Entity is about to get inside the block
//            position->setRight(solidPosition->getLeft());
//         }
//         solid->addComponent<LeftCollisionComponent>();
//         direction = Direction::RIGHT;
//      }
//   }
//
//   return direction;
//}

bool AABBCollision(PositionComponent* a, PositionComponent* b) {
   return a->position.x + a->hitbox.x <= b->position.x + b->hitbox.x + b->hitbox.w &&
          a->position.x + a->hitbox.x + a->hitbox.w >= b->position.x + b->hitbox.x &&
          a->position.y + a->hitbox.y <= b->position.y + b->hitbox.y + b->hitbox.h &&
          a->position.y + a->hitbox.y + a->hitbox.h >= b->position.y + b->hitbox.y;
}

bool AABBCollision(float x, float y, float w, float h, PositionComponent* b) {
   return x <= b->position.x + b->scale.x && x + w >= b->position.x &&
          y <= b->position.y + b->scale.x && y + h >= b->position.y;
}

bool AABBCollision(float x1, float y1, float w1, float h1, float x2, float y2, float w2, float h2) {
   return x1 <= x2 + w2 && x1 + w1 >= x2 && y1 <= y2 + h2 && y1 + h1 >= y2;
}

bool AABBTotalCollision(PositionComponent* a, PositionComponent* b) {
   return a->position.x + a->hitbox.x < b->position.x + b->hitbox.x + b->hitbox.w &&
          a->position.x + a->hitbox.x + a->hitbox.w > b->position.x + b->hitbox.x &&
          a->position.y + a->hitbox.y < b->position.y + b->hitbox.y + b->hitbox.h &&
          a->position.y + a->hitbox.y + a->hitbox.h > b->position.y + b->hitbox.y;
}
bool AABBTotalCollision(float x, float y, float w, float h, PositionComponent* b) {
   return x < b->position.x + b->scale.x && x + w > b->position.x &&
          y < b->position.y + b->scale.x && y + h > b->position.y;
}

bool AABBTotalCollision(float x1, float y1, float w1, float h1, float x2, float y2, float w2,
                        float h2) {
   return x1 < x2 + w2 && x1 + w1 > x2 && y1 < y2 + h2 && y1 + h1 > y2;
}
