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
   void setCameraMin(float x);
   void setCameraMax(float x);
   void setCameraFrozen(bool val);

   float getCameraX();
   float getCameraY();
   float getCameraCenterX();
   float getCameraCenterY();
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
   float m_cameraMinX = 0.0, m_cameraMaxX = 1.0;
   bool m_frozen = false;
};
