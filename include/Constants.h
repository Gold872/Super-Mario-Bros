#pragma once

constexpr int MAX_FPS = 60;

constexpr int ORIGINAL_CUBE_SIZE = 16;
constexpr int CUBE_SCALE_FACTOR = 2;
constexpr int SCALED_CUBE_SIZE = ORIGINAL_CUBE_SIZE * CUBE_SCALE_FACTOR;

constexpr int SCREEN_WIDTH = 25 * SCALED_CUBE_SIZE;
constexpr int SCREEN_HEIGHT = 15 * SCALED_CUBE_SIZE;

constexpr float GRAVITY = 0.5f;
constexpr float FRICTION = 0.94f;
constexpr float MAX_SPEED_X = 10.00f;
constexpr float MAX_SPEED_Y = 8.5f;

constexpr float MARIO_ACCELERATION_X = 0.24f;
constexpr float MARIO_JUMP_ACCELERATION = 1.10f;
constexpr float MARIO_BOUNCE = 3.5f;
constexpr float ENEMY_BOUNCE = 6.0f;  // when jumping on top of enemies
constexpr float ENEMY_SPEED = 1.0f;
constexpr float COLLECTIBLE_SPEED = 2.0f;

constexpr int TILE_ROUNDNESS = 4;
