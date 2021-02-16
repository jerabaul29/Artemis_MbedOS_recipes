#include "Arduino.h"
#include <Wire.h>
#include "SparkFun_BNO080_Arduino_Library.h"
#include "vector_and_quaternion.h"

#include "etl.h"
#include "etl/circular_buffer.h"

#include "assert_tools.h"

BNO080 bno080_imu;

//------------------------------------------------------------------------
// helper functions

//Given a accuracy number, print what it means
void printAccuracyLevel(byte accuracyNumber){          // accuracy is:
  if (accuracyNumber == 0) Serial.print(F("U"));       // Unreliable
  else if (accuracyNumber == 1) Serial.print(F("L"));  // Low
  else if (accuracyNumber == 2) Serial.print(F("M"));  // Medium
  else if (accuracyNumber == 3) Serial.print(F("H"));  // High
}

//------------------------------------------------------------------------
// some parameters
// constexpr unsigned long reading_period_millis = 500;  // works very fine
constexpr unsigned long reading_period_millis = 5;  // high frequency, but should be fine on all outputs
constexpr unsigned long printout_frequency = 500;   // we do not want to print too often

//------------------------------------------------------------------------
// IMU controller

class IMU_Controller{
  public:
    void begin(uint8_t deviceAddress = BNO080_DEFAULT_ADDRESS, TwoWire &wirePort = Wire);
    bool data_available(void);
    void read_all_data(void);
    
};

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

  bno080_imu.begin();
  delay(50);
  bno080_imu.enableDebugging(Serial);
  delay(50);
  
  Wire.setClock(400000); //Increase I2C data rate to 400kHz
  delay(50);

  bno080_imu.enableAccelerometer(reading_period_millis);
  bno080_imu.enableMagnetometer(reading_period_millis);
  bno080_imu.enableGyro(reading_period_millis);
  bno080_imu.enableRotationVector(reading_period_millis);
  delay(5000);

  Serial.println(F("sensor set up, start measurin"));
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
  }

}
