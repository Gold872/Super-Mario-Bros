#include "Camera.h"

#include "Constants.h"
#include "ECS/Components.h"

Camera Camera::m_instance;

void Camera::setCameraX(float x) {
   m_cameraX = x;
}

void Camera::setCameraY(float y) {
   m_cameraY = y;
}

void Camera::increaseCameraX(float value) {
   m_cameraX += value;
}

void Camera::updateCameraMin() {
   m_cameraMinX = m_cameraX;
}

void Camera::setCameraMin(float x) {
   m_cameraMinX = x;
}

void Camera::setCameraMax(float x) {
   setCameraX(x - SCREEN_WIDTH);
}

void Camera::setCameraFrozen(bool val) {
   m_frozen = val;
}

float Camera::getCameraX() {
   return m_cameraX;
}

float Camera::getCameraY() {
   return m_cameraY;
}

float Camera::getCameraCenterX() {
   return getCameraX() + (SCREEN_WIDTH / 2);
}

float Camera::getCameraCenterY() {
   return getCameraY() + (SCREEN_HEIGHT / 2);
}

float Camera::getCameraMinX() {
   return m_cameraMinX;
}

float Camera::getCameraMaxX() {
   return getCameraX() + SCREEN_WIDTH;
}

bool Camera::isFrozen() {
   return m_frozen;
}

bool Camera::inCameraRange(PositionComponent* position) {
   return inCameraXRange(position) && inCameraYRange(position);
}

bool Camera::inCameraXRange(PositionComponent* position) {
   return position->position.x + position->scale.x >= getCameraX() &&
          position->position.x <= getCameraX() + SCREEN_WIDTH;
}

bool Camera::inCameraYRange(PositionComponent* position) {
   return position->position.y + position->scale.y >= getCameraY() &&
          position->position.y <= getCameraY() + SCREEN_HEIGHT;
}
