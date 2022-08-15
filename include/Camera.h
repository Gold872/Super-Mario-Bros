#pragma once

#include "ECS/Components.h"

class Camera {
  public:
   static Camera& Get() {
      return m_instance;
   }

   void setCameraX(float x);
   void setCameraY(float y);
   void increaseCameraX(float value);
   void updateCameraMin();
   void setCameraLeft(float x);
   void setCameraRight(float x);
   void setCameraFrozen(bool val);
   void setCameraMinX(float x);
   void setCameraMaxX(float x);

   float getCameraX();
   float getCameraY();
   float getCameraCenterX();
   float getCameraCenterY();
   float getCameraLeft();
   float getCameraRight();
   float getCameraMinX();
   float getCameraMaxX();

   bool inCameraRange(PositionComponent* position);
   bool inCameraXRange(PositionComponent* position);
   bool inCameraYRange(PositionComponent* position);
   bool isFrozen();

  private:
   Camera() {}

   Camera(const Camera&) = delete;

   static Camera m_instance;

   float m_cameraX = 0.0, m_cameraY = 0.0;
   float m_cameraMinX = 0.0, m_cameraMaxX = 0.0;
   bool m_frozen = false;
};
