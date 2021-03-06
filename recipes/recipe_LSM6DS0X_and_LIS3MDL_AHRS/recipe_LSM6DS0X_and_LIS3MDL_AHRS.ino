
// Full orientation sensing using NXP/Madgwick/Mahony and a range of 9-DoF
// sensor sets.
// You *must* perform a magnetic calibration before this code will work.
//
// To view this data, use the Arduino Serial Monitor to watch the
// scrolling angles, or run the OrientationVisualiser example in Processing.
// Based on  https://github.com/PaulStoffregen/NXPMotionSense with adjustments
// to Adafruit Unified Sensor interface

#include "Arduino.h"
#include <Adafruit_Sensor_Calibration.h>
#include <Adafruit_AHRS.h>
#include "kiss_clang_3d.h"

Adafruit_Sensor *accelerometer, *gyroscope, *magnetometer;

// uncomment one combo 9-DoF!
#include "LSM6DS_LIS3MDL.h"  // can adjust to LSM6DS33, LSM6DS3U, LSM6DSOX...
//#include "LSM9DS.h"           // LSM9DS1 or LSM9DS0
//#include "NXP_FXOS_FXAS.h"  // NXP 9-DoF breakout

// pick your filter! slower == better quality output
Adafruit_NXPSensorFusion filter; // slowest
//Adafruit_Madgwick filter;  // faster than NXP
//Adafruit_Mahony filter;  // fastest/smalleset

#if defined(ADAFRUIT_SENSOR_CALIBRATION_USE_EEPROM)
  Adafruit_Sensor_Calibration_EEPROM cal;
#else
  Adafruit_Sensor_Calibration_SDFat cal;
#endif

#define FILTER_UPDATE_RATE_HZ 100
#define PRINT_EVERY_N_UPDATES 10

// if we should print all debug information
#define AHRS_DEBUG_OUTPUT

uint32_t timestamp;

void setup() {
  delay(500);
  
  Serial.begin(1000000);
  while (!Serial) yield();

  if (!cal.begin()) {
    Serial.println("Failed to initialize calibration helper");
  } else if (! cal.loadCalibration()) {
    Serial.println("No calibration loaded/found");
  }

  while (!init_sensors()) {  // it looks like the sensor takes a tiny bit of time to wake up and be available
    Serial.println("Failed to find sensors; are things correctly connected, are you using the right core?");
    delay(100);
  }
  
  accelerometer->printSensorDetails();
  gyroscope->printSensorDetails();
  magnetometer->printSensorDetails();

  setup_sensors();
  filter.begin(FILTER_UPDATE_RATE_HZ);
  timestamp = millis();

  // TODO: check what max I2C clock frequency can be
  Wire.setClock(400000); // 400KHz is definitely OK
  // Wire.setClock(600000); // probably not ok according to datasheet
}


void loop() {
  float roll, pitch, heading;
  float gx, gy, gz;
  static uint8_t counter = 0;

  if ((millis() - timestamp) < (1000 / FILTER_UPDATE_RATE_HZ)) {
    return;
  }

  timestamp = micros();

  // Read the motion sensors
  sensors_event_t accel, gyro, mag;
  accelerometer->getEvent(&accel);
  gyroscope->getEvent(&gyro);
  magnetometer->getEvent(&mag);

#if defined(AHRS_DEBUG_OUTPUT)
  Serial.print("I2C took "); Serial.print(micros()-timestamp); Serial.println(" us");
#endif
  timestamp = micros();

  //enableBurstMode();

  cal.calibrate(mag);
  cal.calibrate(accel);
  cal.calibrate(gyro);
  // Gyroscope needs to be converted from Rad/s to Degree/s
  // the rest are not unit-important
  gx = gyro.gyro.x * SENSORS_RADS_TO_DPS;
  gy = gyro.gyro.y * SENSORS_RADS_TO_DPS;
  gz = gyro.gyro.z * SENSORS_RADS_TO_DPS;

  // Update the SensorFusion filter
  filter.update(gx, gy, gz, 
                accel.acceleration.x, accel.acceleration.y, accel.acceleration.z, 
                mag.magnetic.x, mag.magnetic.y, mag.magnetic.z);

  //disableBurstMode();

#if defined(AHRS_DEBUG_OUTPUT)
  Serial.print("Update took "); Serial.print(micros()-timestamp); Serial.println(" us");
#endif
  timestamp = micros();

  // only print the calculated output once in a while
  if (counter++ <= PRINT_EVERY_N_UPDATES) {
    return;
  }
  // reset the counter
  counter = 0;

#if defined(AHRS_DEBUG_OUTPUT)
  Serial.print("Raw acc: ");
  Serial.print(accel.acceleration.x, 4); Serial.print(", ");
  Serial.print(accel.acceleration.y, 4); Serial.print(", ");
  Serial.print(accel.acceleration.z, 4); Serial.print(", ");
  Serial.print(F(" | gyr: "));
  Serial.print(gx, 4); Serial.print(", ");
  Serial.print(gy, 4); Serial.print(", ");
  Serial.print(gz, 4); Serial.print(", ");
  Serial.print(F(" | mag: "));
  Serial.print(mag.magnetic.x, 4); Serial.print(", ");
  Serial.print(mag.magnetic.y, 4); Serial.print(", ");
  Serial.print(mag.magnetic.z, 4); Serial.println("");
#endif

  // print the heading, pitch and roll
  roll = filter.getRoll();
  pitch = filter.getPitch();
  heading = filter.getYaw();
  Serial.print("Orientation: ");
  Serial.print(heading);
  Serial.print(", ");
  Serial.print(pitch);
  Serial.print(", ");
  Serial.println(roll);

  // print the quaternion
  float qw, qx, qy, qz;
  filter.getQuaternion(&qw, &qx, &qy, &qz);
  Serial.print("Quaternion: ");
  Serial.print(qw, 4);
  Serial.print(", ");
  Serial.print(qx, 4);
  Serial.print(", ");
  Serial.print(qy, 4);
  Serial.print(", ");
  Serial.println(qz, 4);  
  
#if defined(AHRS_DEBUG_OUTPUT)
  Serial.print("Printing took "); Serial.print(micros()-timestamp); Serial.println(" us");
#endif

  // perform the quaternion transform to go into a NED or similar referential
  vec3 accel_raw;
  vec3 accel_NED;
  quat quat_rotation;
  
  vec3_setter(&accel_raw, accel.acceleration.x, accel.acceleration.y, accel.acceleration.z);
  quat_setter(&quat_rotation, qw, qx, qy, qz);
  rotate_by_quat_R(&accel_raw, &quat_rotation, &accel_NED);
  
  Serial.print("rotated acc: ");
  Serial.print(accel_NED.i, 4); Serial.print(", ");
  Serial.print(accel_NED.j, 4); Serial.print(", ");
  Serial.print(accel_NED.k, 4); Serial.print(", ");
  Serial.println();
}

/*
testing on SFE Artemis Redboard with burst mode for the Kalman filtering:

I2C took 2915 us
Update took 676 us
Raw acc: -2.4736, -3.9248, 8.8959,  | gyr: 0.0000, -0.1487, -0.3062,  | mag: 23.4142, -7.2201, -25.2412
Orientation: 44.10, 14.35, -23.77
Quaternion: 0.8902, -0.2353, 0.0365, 0.3883
Printing took 4694 us

------------------

testing without burst mode:

I2C took 2917 us
Update took 1234 us
Raw acc: -2.3982, -3.7908, 8.9773,  | gyr: 0.2450, -0.2800, -0.3325,  | mag: 23.0780, -7.0155, -25.5481
Orientation: 43.27, 13.82, -22.90
Quaternion: 0.8956, -0.2266, 0.0369, 0.3809
Printing took 4729 us

------------------

some rough notes about the speed of I2C:

- if it takes 10 bytes to send one reading back and forth (asking, acking, checksumming, etc) [note: this may be very conservative]
- if we ask for 9 quantities
- then it is 90 bytes per "full reading"
- i.e. around 1000 bits per "full reading"

now if the I2C clock is 4.10^5, this means that I2C transmission will take 10^3 bits / 4.10^5 bits/s ie around 2.5ms, which is consistent with what is observed.
however, well possible that each quantity needs much less than 10 bytes. Them question of why transmissions are so slow.

*/
