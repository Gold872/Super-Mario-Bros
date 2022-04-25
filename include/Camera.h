#pragma once

#include "ECS/Components.h"

class Camera {
  public:
   static void setCameraX(float x);
   static void setCameraY(float y);
   static void increaseCameraX(float value);
   static void updateCameraMin();
   static void setCameraMin(float x);
   static void setCameraMax(float x);
   static void setCameraFrozen(bool val);

   static float getCameraX();
   static float getCameraY();
   static float getCameraCenter();
   static float getCameraMinX();
   static float getCameraMaxX();

   static bool inCameraRange(PositionComponent* position);
   static bool inCameraXRange(PositionComponent* position);
   static bool inCameraYRange(PositionComponent* position);
   static bool isFrozen();

  private:
   static float cameraX, cameraY;
   static float cameraMinX, cameraMaxX;
   static bool frozen;
};
