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
constexpr unsigned long imu_output_period_millis = 5UL;
unsigned long millis_last_analysis {0};
unsigned long millis_last_data_receiving {0};
constexpr bool verbose_loop {false};
constexpr bool verbose_timing {false};

//------------------------------------------------------------------------
void setup() {
  Serial.begin(230400);
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
  
  bno080_imu.enableAccelerometer(imu_output_period_millis);
  // bno080_imu.enableMagnetometer(imu_output_period_millis);
  // bno080_imu.enableGyro(imu_output_period_millis);
  bno080_imu.enableRotationVector(imu_output_period_millis);

  Serial.println(F("sensor set up, start measuring"));
  Serial.println(F("the first measurements may be a bit off, the filter needs time to converge"));

  delay(5000);
  
  millis_last_analysis = millis();
  millis_last_data_receiving = millis();

  Serial.println(F("wait for the first BNO output"));
  Serial.println();

  while (true){
    if (bno080_imu.getReadings() != 0){
      break;
    }
  }
}

float accel_x, accel_y, accel_z;
byte accel_accuracy;
float gyro_x, gyro_y, gyro_z;
byte gyro_accuracy;
float mag_x, mag_y, mag_z;
byte mag_accuracy;
float quat_i, quat_j, quat_k, quat_real, quat_radian_accuracy;
byte quat_accuracy;

//------------------------------------------------------------------------
void loop() {
  uint16_t reading_status = bno080_imu.getReadings();
  
  if (reading_status != 0){
    if (verbose_timing){
      Serial.print(F("received data at ms: ")); Serial.print(millis()); Serial.print(F(" | type: "));
    }
    millis_last_data_receiving = millis();

    if (reading_status == SENSOR_REPORTID_ACCELEROMETER){
      accel_x = bno080_imu.getAccelX();
      accel_y = bno080_imu.getAccelY();
      accel_z = bno080_imu.getAccelZ();
      accel_accuracy = bno080_imu.getLinAccelAccuracy();
      if (verbose_timing){
        Serial.print(F("accel"));
      }
    }

    if(reading_status == SENSOR_REPORTID_GYROSCOPE){
      gyro_x = bno080_imu.getGyroX();
      gyro_y = bno080_imu.getGyroY();
      gyro_z = bno080_imu.getGyroZ();
      gyro_accuracy = bno080_imu.getGyroAccuracy();
      if (verbose_timing){
        Serial.print(F("gyro"));
      }
    }

    if(reading_status == SENSOR_REPORTID_MAGNETIC_FIELD){
      mag_x = bno080_imu.getMagX();
      mag_y = bno080_imu.getMagY();
      mag_z = bno080_imu.getMagZ();
      mag_accuracy = bno080_imu.getMagAccuracy();
      if (verbose_timing){
        Serial.print(F("mag"));
      }
    }

    if(reading_status == SENSOR_REPORTID_ROTATION_VECTOR){
      quat_i = bno080_imu.getQuatI();
      quat_j = bno080_imu.getQuatJ();
      quat_k = bno080_imu.getQuatK();
      quat_real = bno080_imu.getQuatReal();
      quat_accuracy = bno080_imu.getQuatAccuracy();
      quat_radian_accuracy = bno080_imu.getQuatRadianAccuracy();
      if (verbose_timing){
        Serial.print(F("quat"));
      }
    }

    if (verbose_timing){
        Serial.print(F(" | receive and print took ")); Serial.print(millis() - millis_last_data_receiving); Serial.println(F(" ms"));
    }
  }

  if (millis() - millis_last_analysis > 2 * imu_output_period_millis){
    Serial.println(F("*************************************"));
    Serial.println(F("WARNING: QUAT ANALYSIS FALLING BEHIND"));
    Serial.print(F("ms: ")); Serial.println(millis());
    Serial.println(F("*************************************"));
    millis_last_analysis = millis();
  }

  if (millis() - millis_last_analysis >= imu_output_period_millis){
    millis_last_analysis += imu_output_period_millis;
    unsigned long millis_start = millis();

    if (verbose_loop){
      Serial.println();
      Serial.print(F("millis start analysis: ")); Serial.println(millis_start);
  
      Serial.print(F("accel: "));
      Serial.print(accel_x, 4);
      Serial.print(F(", "));
      Serial.print(accel_y, 4);
      Serial.print(F(", "));
      Serial.print(accel_z, 4);
      Serial.print(F(", pres: "));
      printAccuracyLevel(accel_accuracy);
  
      Serial.print(F(" | gyro: "));
      Serial.print(gyro_x, 4);
      Serial.print(F(", "));
      Serial.print(gyro_y, 4);
      Serial.print(F(", "));
      Serial.print(gyro_z, 4);
      Serial.print(F(", pres: "));
      printAccuracyLevel(gyro_accuracy);
  
      Serial.print(F(" | mag: "));
      Serial.print(mag_x, 4);
      Serial.print(F(", "));
      Serial.print(mag_y, 4);
      Serial.print(F(", "));
      Serial.print(mag_z, 4);
      Serial.print(F(", pres: "));
      printAccuracyLevel(mag_accuracy);
      
      Serial.print(F(" | quat: "));
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
      Serial.print(quat_radian_accuracy, 4);
      Serial.println();
    }

    // look at quaternion data
    Quaternion quat_orientation {quat_real, quat_i, quat_j, quat_k};
    
    if (verbose_loop){
      print(quat_orientation);
      Serial.print(F("quat norm: "));
      Serial.println(quat_orientation.norm());
    }

    if (abs(quat_orientation.norm() - 1.0f) > 1.0e-2){
      Serial.println(F("*************"));
      Serial.println(F("NON UNIT QUAT"));
      Serial.print(F("at ms: ")); Serial.print(millis()); Serial.print(F(", quat norm: ")); Serial.println(quat_orientation.norm());
      Serial.println(F("*************"));
    }

    // acceleration in an IMU and ENU referential (the datasheet says it outputs in East North Up rather than North East Down)
    Vector accel_imu_ref{accel_x, accel_y, accel_z};
    
    if (verbose_loop){
      Serial.print(F("accel norm IMU frame of ref: ")); Serial.print(accel_imu_ref.norm());
    }

    Vector accel_ENU_ref{};
    rotate_vect_by_quat_R(accel_imu_ref, quat_orientation, accel_ENU_ref);
    
    if (verbose_loop){
      Serial.print(F(" | ENU frame of ref: ")); Serial.println(accel_ENU_ref.norm());
      Serial.print(F("accel ENU: "));
      print(accel_ENU_ref);
    }

    // where is the X-direction of the IMU pointing?
    Vector imu_dir_x_ref_imu{1, 0, 0};
    Vector imu_dir_x_ref_enu{};
    rotate_vect_by_quat_R(imu_dir_x_ref_imu, quat_orientation, imu_dir_x_ref_enu);
    
    if (verbose_loop){
      Serial.print(F("IMU X-dir in ENU frame: "));
      print(imu_dir_x_ref_enu);
    }

    // quality checks
    if (abs(accel_ENU_ref.norm() - 9.81) > 10.0){
      Serial.println(F("************"));
      Serial.println(F("*HIGH ACCEL*"));
      Serial.print(F("at ms: ")); Serial.print(millis()); Serial.print(F(", accel vect: ")); print(accel_ENU_ref);
      Serial.println(F("************"));
    }

    unsigned long millis_end = millis();
    
    if (verbose_timing){
      Serial.print(F("millis end: ")); Serial.print(millis_end); Serial.print(F(" | analysis + print duration: ")); Serial.print(millis_end-millis_start); Serial.println(F("ms"));
      Serial.println();
    }
  }
}
