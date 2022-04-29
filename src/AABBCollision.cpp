#include "AABBCollision.h"

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
