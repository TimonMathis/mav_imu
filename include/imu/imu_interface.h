//
// Created by acey on 24.08.22.
//

#ifndef MAV_IMU_INCLUDE_IMU_INTERFACE_H_
#define MAV_IMU_INCLUDE_IMU_INTERFACE_H_

#include <sstream>
#include "spi_driver.h"

template<typename T>
struct vec3 {
  T x;
  T y;
  T z;

  std::string toString() {
    std::stringstream ss;
    ss << "X: " << x << " Y: " << y << " Z: " << z;
    return ss.str();
  }
};

template<typename T>
inline vec3<T> operator/(const vec3<T> t, T num) {
  return {t.x / num, t.y / num, t.z / num};
}

template<typename T>
inline vec3<T> operator/=(vec3<T>& t, T num) {
  return {t.x /= num, t.y /= num, t.z /= num};
}

template<typename T>
inline vec3<T> operator*(const vec3<T> t, T num) {
  return {t.x * num, t.y * num, t.z * num};
}

template<typename T>
inline vec3<T> operator*=(vec3<T>& t, T num) {
  return {t.x *= num, t.y *= num, t.z *= num};
}

namespace IImu {
inline static constexpr const int NaN = std::numeric_limits<int>::quiet_NaN();
}
using namespace IImu;

class ImuBurstResult {
 public:
  vec3<double> gyro = {NaN, NaN, NaN};
  vec3<double> acceleration = {NaN, NaN, NaN};
  vec3<double> magnetometer = {NaN, NaN, NaN};
  double baro{NaN};
  double temp{NaN};
};

class ImuInterface {
 public:

  virtual bool init() = 0;
  /**
   * Imu health check.
   * @return true if successful, otherwise false
   */
  virtual bool selftest() = 0;

  /**
   * Cleanup and close files used by IMU
   * @return
   */
  virtual bool close() = 0;

  /**
   * Gets angular velocity from gyro.
   * @return angular velocity in rad/s
   */
  virtual vec3<double> getGyro() = 0;

  /**
   * Gets acceleration data vector
   *
   * @return acceleration in m/s² as double
   */
  virtual vec3<double> getAcceleration() = 0;

  //! magnetometer measurement
  virtual vec3<double> getMagnetometer() {
    return {NaN, NaN, NaN};
  };

  /**
   * Gets barometric pressure
   * @return QFE pressure in hPa
   */
  virtual double getBarometer() {
    return NaN;
  };

  /**
   * Gets temperature
   * @return temperature in °C
   */
  virtual double getTemperature() {
    return NaN;
  };

  /**
   * Generic function to read spi register
   * @param cmd
   * @return
   */
  virtual int getRaw(std::vector<byte> cmd) = 0;

  /**
   * Reads all sensor data at once
   * @return struct with sensor data. Returns NaN if hardware does not support specific sensor.
   */
  virtual ImuBurstResult burst() {
    ImuBurstResult res{};
    res.gyro = getGyro();
    res.acceleration = getAcceleration();
    res.magnetometer = getMagnetometer();
    res.baro = getBarometer();
    res.temp = getTemperature();

    return res;
  }

  //virtual int getSerialnumber();
};

#endif //MAV_IMU_INCLUDE_IMU_INTERFACE_H_