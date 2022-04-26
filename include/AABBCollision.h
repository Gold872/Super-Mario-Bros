#pragma once

#include "ECS/Components.h"
#include "ECS/ECS.h"

// Direction checkCollisionY(Entity* solid, PositionComponent* position, MovingComponent* move);
// Direction checkCollisionX(Entity* solid, PositionComponent* position, MovingComponent* move);

bool AABBCollision(PositionComponent* a, PositionComponent* b);
bool AABBCollision(float x, float y, float w, float h, PositionComponent* b);
bool AABBCollision(float x1, float y1, float w1, float h1, float x2, float y2, float w2, float h2);

bool AABBTotalCollision(PositionComponent* a, PositionComponent* b);
bool AABBTotalCollision(float x, float y, float w, float h, PositionComponent* b);
bool AABBTotalCollision(float x1, float y1, float w1, float h1, float x2, float y2, float w2,
                        float h2);
