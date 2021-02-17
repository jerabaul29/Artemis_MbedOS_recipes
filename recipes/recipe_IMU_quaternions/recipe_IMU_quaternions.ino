#include "Arduino.h"
#include <Wire.h>
#include "SparkFun_BNO080_Arduino_Library.h"
#include "vector_and_quaternion.h"

BNO080 bno080_imu;

//--------------------------------------------------------------------------------
// a few helper functions

void printAccuracyLevel(byte accuracyNumber){          // accuracy is:
  if (accuracyNumber == 0) Serial.print(F("U"));       // Unreliable
  else if (accuracyNumber == 1) Serial.print(F("L"));  // Low
  else if (accuracyNumber == 2) Serial.print(F("M"));  // Medium
  else if (accuracyNumber == 3) Serial.print(F("H"));  // High
}

//------------------------------------------------------------------------
// some parameters
constexpr unsigned long imu_output_period = 250;

//------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(10);
  Serial.println();
  Serial.println("---------- booted ----------");
  delay(10);

  vect_quat_library_self_diagnostic();

  Wire.begin();
  delay(50);

  Wire.setClock(400000);
  delay(50);

  bno080_imu.begin();
  delay(50);
  bno080_imu.enableDebugging(Serial);
  delay(50);
  
  bno080_imu.enableAccelerometer(imu_output_period);
  bno080_imu.enableMagnetometer(imu_output_period);
  bno080_imu.enableGyro(imu_output_period);
  bno080_imu.enableRotationVector(imu_output_period);
  delay(5000);

  Serial.println(F("sensor set up, start measuring"));
  Serial.println(F("the first measurements may be a bit off, the filter needs time to converge"));
}

//------------------------------------------------------------------------
void loop() {
  if (bno080_imu.dataAvailable() == true){
    float accel_x = bno080_imu.getAccelX();
    float accel_y = bno080_imu.getAccelY();
    float accel_z = bno080_imu.getAccelZ();
    byte accel_accuracy = bno080_imu.getLinAccelAccuracy();

    float gyro_x = bno080_imu.getGyroX();
    float gyro_y = bno080_imu.getGyroY();
    float gyro_z = bno080_imu.getGyroZ();
    byte gyro_accuracy = bno080_imu.getGyroAccuracy();

    float mag_x = bno080_imu.getMagX();
    float mag_y = bno080_imu.getMagY();
    float mag_z = bno080_imu.getMagZ();
    byte mag_accuracy = bno080_imu.getMagAccuracy();

    float quat_i = bno080_imu.getQuatI();
    float quat_j = bno080_imu.getQuatJ();
    float quat_k = bno080_imu.getQuatK();
    float quat_real = bno080_imu.getQuatReal();
    byte quat_accuracy = bno080_imu.getQuatAccuracy();
    float quat_radian_accuracry = bno080_imu.getQuatRadianAccuracy();

    Serial.print(F(" -accel: "));
    Serial.print(accel_x, 4);
    Serial.print(F(", "));
    Serial.print(accel_y, 4);
    Serial.print(F(", "));
    Serial.print(accel_z, 4);
    Serial.print(F(", pres: "));
    printAccuracyLevel(accel_accuracy);

    Serial.print(F(" -gyro: "));
    Serial.print(gyro_x, 4);
    Serial.print(F(", "));
    Serial.print(gyro_y, 4);
    Serial.print(F(", "));
    Serial.print(gyro_z, 4);
    Serial.print(F(", pres: "));
    printAccuracyLevel(gyro_accuracy);

    Serial.print(F(" -mag: "));
    Serial.print(mag_x, 4);
    Serial.print(F(", "));
    Serial.print(mag_y, 4);
    Serial.print(F(", "));
    Serial.print(mag_z, 4);
    Serial.print(F(", pres: "));
    printAccuracyLevel(mag_accuracy);
    
    Serial.print(F(" -quat: "));
    Serial.print(quat_i, 4);
    Serial.print(F(", "));
    Serial.print(quat_j, 4);
    Serial.print(F(", "));
    Serial.print(quat_k, 4);
    Serial.print(F(", "));
    Serial.print(quat_real, 4);
    Serial.print(F(", pres: "));
    printAccuracyLevel(quat_accuracy);
    Serial.print(F(", "));
    Serial.print(quat_radian_accuracry, 4);
    Serial.println();

    // look at quaternion data
    Quaternion quat_orientation {quat_real, quat_i, quat_j, quat_k};
    print(quat_orientation);
    Serial.print(F("quat norm: "));
    Serial.println(quat_orientation.norm());

    // acceleration in an IMU and NED referential
    Vector accel_imu_ref{accel_x, accel_y, accel_z};
    Serial.print(F("accel IMU frame of ref norm: "));
    Serial.println(accel_imu_ref.norm());

    Vector accel_NED_ref{};
    rotate_vect_by_quat_R(accel_imu_ref, quat_orientation, accel_NED_ref);
    Serial.print(F("accel NED frame of ref norm: "));
    Serial.println(accel_NED_ref.norm());
    Serial.print(F("accel NED: "));
    Serial.println(accel_NED_ref.v2);
  }
}

// TODO: quat norm is sometimes off!
