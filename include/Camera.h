#pragma once

#include "ECS/Components.h"

class Camera {
  public:
   static Camera& Get() {
      return instance;
   }

   void setCameraX(float x);
   void setCameraY(float y);
   void increaseCameraX(float value);
   void updateCameraMin();
   void setCameraMin(float x);
   void setCameraMax(float x);
   void setCameraFrozen(bool val);

   float getCameraX();
   float getCameraY();
   float getCameraCenter();
   float getCameraMinX();
   float getCameraMaxX();

   bool inCameraRange(PositionComponent* position);
   bool inCameraXRange(PositionComponent* position);
   bool inCameraYRange(PositionComponent* position);
   bool isFrozen();

  private:
   Camera() {}

   Camera(const Camera&) = delete;

   static Camera instance;

   float cameraX, cameraY;
   float cameraMinX, cameraMaxX;
   bool frozen;
};
