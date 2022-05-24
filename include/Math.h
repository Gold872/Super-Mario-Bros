#pragma once

#include <algorithm>

/*
 * The Vector2 stores an x and a y value. This is used for the position of all Entities
 * on the screen, and may also be used for image scaling.
 * */
template <typename T>
class Vector2 {
  public:
   Vector2() : x{0}, y{0} {};
   Vector2(T X, T Y) : x{X}, y{Y} {};
   Vector2(T BOTH) : x{BOTH}, y{BOTH} {};

   void setPosition(T X, T Y) {
      x = X;
      y = Y;
   }

   T x;
   T y;

   template <typename OTHER>
   Vector2<OTHER> convertTo() {
      return Vector2<OTHER>((OTHER)x, (OTHER)y);
   };
};

template <typename T>
bool operator==(const Vector2<T>& left, const Vector2<T>& right) {
   return left.x == right.x && left.y == right.y;
}

template <typename T>
bool operator!=(const Vector2<T>& left, const Vector2<T>& right) {
   return left.x != right.x || left.y != right.y;
}

template <typename T>
Vector2<T> operator+(const Vector2<T>& left, const Vector2<T>& right) {
   return Vector2<T>(left.x + right.x, left.y + right.y);
}

template <typename T>
Vector2<T> operator-(const Vector2<T>& left, const Vector2<T>& right) {
   return Vector2<T>(left.x - right.x, left.y - right.y);
}

template <typename T>
Vector2<T> operator*(const Vector2<T>& A, int B) {
   return Vector2<T>(A.x * B, A.y * B);
}

template <typename T>
Vector2<T> operator/(const Vector2<T>& A, int B) {
   return Vector2<T>(A.x / B, A.y / B);
}

template <typename T>
void operator+=(Vector2<T>& left, const Vector2<T>& right) {
   left.x += right.x;
   left.y += right.y;
}

template <typename T>
void operator-=(Vector2<T>& left, const Vector2<T>& right) {
   left.x -= right.x;
   left.y -= right.y;
}

template <typename T>
void operator*=(Vector2<T>& A, int B) {
   A.x *= B;
   A.y *= B;
}

typedef Vector2<int> Vector2i;
typedef Vector2<unsigned int> Vector2u;
typedef Vector2<float> Vector2f;

class PIDController {
  public:
   PIDController() = default;
   PIDController(double kP, double kI, double kD) : m_kP{kP}, m_kI{kI}, m_kD{kD} {}
   PIDController(double kP, double kI, double kD, double period)
       : m_kP{kP}, m_kI{kI}, m_kD{kD}, m_period{period} {}

   double calculate(double measurement) {
      m_positionError = m_setpoint - measurement;
      m_velocityError = (m_setpoint - measurement) / m_period;

      if (m_kI != 0) {
         m_totalError = clamp(m_totalError + m_velocityError * m_period, m_minIntegral / m_kI,
                              m_maxIntegral / m_kI);
      }

      return m_positionError * m_kP + m_totalError * m_kI + m_velocityError * m_kD;
   }

   double calculate(double measurement, double setpoint) {
      m_positionError = setpoint - measurement;
      m_velocityError = (setpoint - measurement) / m_period;

      if (m_kI != 0) {
         m_totalError = clamp(m_totalError + m_velocityError * m_period, m_minIntegral / m_kI,
                              m_maxIntegral / m_kI);
      }

      return (m_positionError * m_kP) + (m_totalError * m_kI) + (m_velocityError * m_kD);
   }

   void setSetpoint(double setpoint) {
      m_setpoint = setpoint;
   }

  private:
   double m_kP = 0;
   double m_kI = 0;
   double m_kD = 0;

   double m_period = 1.0;

   double m_minIntegral = -1.0;
   double m_maxIntegral = 1.0;

   double m_setpoint = 0;
   double m_totalError = 0;
   double m_velocityError = 0;
   double m_positionError = 0;

   double clamp(double value, double low, double high) {
      return std::max(low, std::min(value, high));
   }
};
