#include "Camera.h"

#include "Constants.h"
#include "ECS/Components.h"

Camera Camera::instance;

void Camera::setCameraX(float x) {
   cameraX = x;
}

void Camera::setCameraY(float y) {
   cameraY = y;
}

void Camera::increaseCameraX(float value) {
   cameraX += value;
}

void Camera::updateCameraMin() {
   cameraMinX = cameraX;
}

void Camera::setCameraMin(float x) {
   cameraMinX = x;
}

void Camera::setCameraMax(float x) {
   setCameraX(x - SCREEN_WIDTH);
}

void Camera::setCameraFrozen(bool val) {
   frozen = val;
}

float Camera::getCameraX() {
   return cameraX;
}

float Camera::getCameraY() {
   return cameraY;
}

float Camera::getCameraCenter() {
   return getCameraX() + (SCREEN_WIDTH / 2);
}

float Camera::getCameraMinX() {
   return cameraMinX;
}

float Camera::getCameraMaxX() {
   return getCameraX() + SCREEN_WIDTH;
}

bool Camera::isFrozen() {
   return frozen;
}

bool Camera::inCameraRange(PositionComponent* position) {
   return inCameraXRange(position) && inCameraYRange(position);
}

bool Camera::inCameraXRange(PositionComponent* position) {
   return position->position.x + position->scale.x >= getCameraX() &&
          position->position.x <= Camera::getCameraX() + SCREEN_WIDTH;
}

bool Camera::inCameraYRange(PositionComponent* position) {
   return position->position.y + position->scale.y >= getCameraY() &&
          position->position.y <= getCameraY() + SCREEN_HEIGHT;
}
