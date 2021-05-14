// Basic demo for accelerometer/gyro readings from Adafruit ISM330DHCX
// on the Artemis module

#include "Wire.h"

#include <SparkFun_Qwiic_Power_Switch_Arduino_Library.h>

#include <Adafruit_ISM330DHCX.h>
#include <Adafruit_LIS3MDL.h>
#include <Adafruit_Sensor.h>

// TODO
// need to calibrate; do this either by hand, or using dedicated library; see for example:
// #include <Adafruit_Sensor_Calibration.h>

#include <Adafruit_AHRS.h>
#include "kiss_clang_3d.h"

// this is the broken out Qwiic
TwoWire ArtemisWire(4);

QWIIC_POWER mySwitch;

Adafruit_ISM330DHCX ism330dhcx;
Adafruit_LIS3MDL lis3mdl;

// a true Kalman filter; slow but good
Adafruit_NXPSensorFusion filter;

// our frequency params
static constexpr unsigned long filter_update_rate_hz {100};
static constexpr unsigned long filter_update_interval_ms = 1000 / filter_update_rate_hz;
static constexpr unsigned long printing_interval_ms = 200;
unsigned long timestamp_sensors_filter;
unsigned long timestamp_printing;
unsigned long micros_timing;

// the values for the AHRS
float acc_x, acc_y, acc_z;
float gyr_x, gyr_y, gyr_z;
float mag_x, mag_y, mag_z;
float qr, qi, qj, qk;
float roll, pitch, heading;
int counter = 0;

sensors_event_t accel;
sensors_event_t gyro;
sensors_event_t temp;
sensors_event_t mag; 

void setup(void) {

  Serial.begin(1000000);
  while (!Serial){}
  delay(10);

  ArtemisWire.begin();
  ArtemisWire.setClock(1000000);
  delay(500);

  Serial.println(F("Sparkfun power switch start!"));
  while (true){
    if (mySwitch.begin(ArtemisWire) == false){
        Serial.println(F("Qwiic Power Switch not detected at default I2C address, will try again..."));
        delay(100);
    }
    else{
      break;
    }
  }
  Serial.println(F("power switch found!"));

  // configure the power switch
  mySwitch.isolationOff();
  mySwitch.powerOn();
  // mySwitch.powerOff();
  mySwitch.pinMode(1, INPUT);
  mySwitch.pinMode(2, INPUT);
  delay(500);

  Serial.println("Adafruit ISM330DHCX start!");
  while (true){
    if (!ism330dhcx.begin_I2C(LSM6DS_I2CADDR_DEFAULT,
                     &ArtemisWire, 0)) {
      Serial.println("Failed to find ISM330DHCX chip, will try again...");
      delay(500);
    }
    else{
      break;
    }
  }
  Serial.println("ISM330DHCX Found!");

  Serial.println(F("Adafruit LIS3MDL start!"));
  while (true){
    if (! lis3mdl.begin_I2C(LIS3MDL_I2CADDR_DEFAULT,
                     &ArtemisWire)) {
      Serial.println("Failed to find LIS3MDL chip, will try again...");
      delay(500);
    }
    else{
      break;
    }
  }
  Serial.println("LIS3MDL Found!");

  delay(500);

  // set the accel gyro properties
  ism330dhcx.setAccelRange(LSM6DS_ACCEL_RANGE_2_G);
  ism330dhcx.setGyroRange(LSM6DS_GYRO_RANGE_125_DPS);
  ism330dhcx.setAccelDataRate(LSM6DS_RATE_833_HZ);
  ism330dhcx.setGyroDataRate(LSM6DS_RATE_833_HZ);
  ism330dhcx.configInt1(false, false, true); // accelerometer DRDY on INT1
  ism330dhcx.configInt2(false, true, false); // gyro DRDY on INT2
  delay(500);

  // set the magnetometer properties
  lis3mdl.setPerformanceMode(LIS3MDL_HIGHMODE);
  lis3mdl.setOperationMode(LIS3MDL_CONTINUOUSMODE);
  lis3mdl.setDataRate(LIS3MDL_DATARATE_560_HZ);
  lis3mdl.setRange(LIS3MDL_RANGE_4_GAUSS);
  lis3mdl.setIntThreshold(500);
  lis3mdl.configInterrupt(false, false, true, // enable z axis
                          true, // polarity
                          false, // don't latch
                          true); // enabled!
  delay(500);

  // print accel gyro info
  Serial.println(F("accel gyro info"));
  Serial.print("Accelerometer range set to: ");
  switch (ism330dhcx.getAccelRange()) {
    case LSM6DS_ACCEL_RANGE_2_G:
    Serial.println("+-2G");
    break;
  case LSM6DS_ACCEL_RANGE_4_G:
    Serial.println("+-4G");
    break;
  case LSM6DS_ACCEL_RANGE_8_G:
    Serial.println("+-8G");
    break;
  case LSM6DS_ACCEL_RANGE_16_G:
    Serial.println("+-16G");
    break;
  }

  Serial.print("Gyro range set to: ");
  switch (ism330dhcx.getGyroRange()) {
    case LSM6DS_GYRO_RANGE_125_DPS:
    Serial.println("125 degrees/s");
    break;
  case LSM6DS_GYRO_RANGE_250_DPS:
    Serial.println("250 degrees/s");
    break;
  case LSM6DS_GYRO_RANGE_500_DPS:
    Serial.println("500 degrees/s");
    break;
  case LSM6DS_GYRO_RANGE_1000_DPS:
    Serial.println("1000 degrees/s");
    break;
  case LSM6DS_GYRO_RANGE_2000_DPS:
    Serial.println("2000 degrees/s");
    break;
  case ISM330DHCX_GYRO_RANGE_4000_DPS:
    Serial.println("4000 degrees/s");
    break;
  }

  Serial.print("Accelerometer data rate set to: ");
  switch (ism330dhcx.getAccelDataRate()) {
    case LSM6DS_RATE_SHUTDOWN:
    Serial.println("0 Hz");
    break;
  case LSM6DS_RATE_12_5_HZ:
    Serial.println("12.5 Hz");
    break;
  case LSM6DS_RATE_26_HZ:
    Serial.println("26 Hz");
    break;
  case LSM6DS_RATE_52_HZ:
    Serial.println("52 Hz");
    break;
  case LSM6DS_RATE_104_HZ:
    Serial.println("104 Hz");
    break;
  case LSM6DS_RATE_208_HZ:
    Serial.println("208 Hz");
    break;
  case LSM6DS_RATE_416_HZ:
    Serial.println("416 Hz");
    break;
  case LSM6DS_RATE_833_HZ:
    Serial.println("833 Hz");
    break;
  case LSM6DS_RATE_1_66K_HZ:
    Serial.println("1.66 KHz");
    break;
  case LSM6DS_RATE_3_33K_HZ:
    Serial.println("3.33 KHz");
    break;
  case LSM6DS_RATE_6_66K_HZ:
    Serial.println("6.66 KHz");
    break;
  }

  Serial.print("Gyro data rate set to: ");
  switch (ism330dhcx.getGyroDataRate()) {
    case LSM6DS_RATE_SHUTDOWN:
    Serial.println("0 Hz");
    break;
  case LSM6DS_RATE_12_5_HZ:
    Serial.println("12.5 Hz");
    break;
  case LSM6DS_RATE_26_HZ:
    Serial.println("26 Hz");
    break;
  case LSM6DS_RATE_52_HZ:
    Serial.println("52 Hz");
    break;
  case LSM6DS_RATE_104_HZ:
    Serial.println("104 Hz");
    break;
  case LSM6DS_RATE_208_HZ:
    Serial.println("208 Hz");
    break;
  case LSM6DS_RATE_416_HZ:
    Serial.println("416 Hz");
    break;
  case LSM6DS_RATE_833_HZ:
    Serial.println("833 Hz");
    break;
  case LSM6DS_RATE_1_66K_HZ:
    Serial.println("1.66 KHz");
    break;
  case LSM6DS_RATE_3_33K_HZ:
    Serial.println("3.33 KHz");
    break;
  case LSM6DS_RATE_6_66K_HZ:
    Serial.println("6.66 KHz");
    break;
  }

  // print magneto info
  Serial.println(F("print gyro info"));

  Serial.print("Performance mode set to: ");
  switch (lis3mdl.getPerformanceMode()) {
      case LIS3MDL_LOWPOWERMODE: Serial.println("Low"); break;
    case LIS3MDL_MEDIUMMODE: Serial.println("Medium"); break;
    case LIS3MDL_HIGHMODE: Serial.println("High"); break;
    case LIS3MDL_ULTRAHIGHMODE: Serial.println("Ultra-High"); break;
  }

  Serial.print("Operation mode set to: ");
  // Single shot mode will complete conversion and go into power down
  switch (lis3mdl.getOperationMode()) {
    case LIS3MDL_CONTINUOUSMODE: Serial.println("Continuous"); break;
    case LIS3MDL_SINGLEMODE: Serial.println("Single mode"); break;
    case LIS3MDL_POWERDOWNMODE: Serial.println("Power-down"); break;
  }

  Serial.print("Data rate set to: ");
  switch (lis3mdl.getDataRate()) {
    case LIS3MDL_DATARATE_0_625_HZ: Serial.println("0.625 Hz"); break;
    case LIS3MDL_DATARATE_1_25_HZ: Serial.println("1.25 Hz"); break;
    case LIS3MDL_DATARATE_2_5_HZ: Serial.println("2.5 Hz"); break;
    case LIS3MDL_DATARATE_5_HZ: Serial.println("5 Hz"); break;
    case LIS3MDL_DATARATE_10_HZ: Serial.println("10 Hz"); break;
    case LIS3MDL_DATARATE_20_HZ: Serial.println("20 Hz"); break;
    case LIS3MDL_DATARATE_40_HZ: Serial.println("40 Hz"); break;
    case LIS3MDL_DATARATE_80_HZ: Serial.println("80 Hz"); break;
    case LIS3MDL_DATARATE_155_HZ: Serial.println("155 Hz"); break;
    case LIS3MDL_DATARATE_300_HZ: Serial.println("300 Hz"); break;
    case LIS3MDL_DATARATE_560_HZ: Serial.println("560 Hz"); break;
    case LIS3MDL_DATARATE_1000_HZ: Serial.println("1000 Hz"); break;
  }

  Serial.print("Range set to: ");
  switch (lis3mdl.getRange()) {
    case LIS3MDL_RANGE_4_GAUSS: Serial.println("+-4 gauss"); break;
    case LIS3MDL_RANGE_8_GAUSS: Serial.println("+-8 gauss"); break;
    case LIS3MDL_RANGE_12_GAUSS: Serial.println("+-12 gauss"); break;
    case LIS3MDL_RANGE_16_GAUSS: Serial.println("+-16 gauss"); break;
  }

  // initialize the filter
  filter.begin(filter_update_rate_hz);
  timestamp_sensors_filter = millis();
  timestamp_printing = millis();
}

void loop() {
  // should we update the measurement and filter?
  if (millis() - timestamp_sensors_filter >= filter_update_interval_ms){
    timestamp_sensors_filter += filter_update_interval_ms;

    enableBurstMode();
    micros_timing = micros();
    ism330dhcx.getEvent(&accel, &gyro, &temp);
    lis3mdl.getEvent(&mag);
    micros_timing = micros() - micros_timing;
    //Serial.print(F("getEvents took [us]: ")); Serial.println(micros_timing);

    // Update the SensorFusion filter
    micros_timing = micros();
    filter.update(gyro.gyro.x * SENSORS_RADS_TO_DPS, gyro.gyro.y * SENSORS_RADS_TO_DPS, gyro.gyro.z * SENSORS_RADS_TO_DPS, 
                  accel.acceleration.x, accel.acceleration.y, accel.acceleration.z, 
                  mag.magnetic.x, mag.magnetic.y, mag.magnetic.z);
    micros_timing = micros() - micros_timing;
    //Serial.print(F("Kalman filtering took [us]: ")); Serial.println(micros_timing);
    disableBurstMode();
  }

  if (millis() - timestamp_printing >= printing_interval_ms){
    timestamp_printing += printing_interval_ms;

    Serial.print("\t\tTemperature ");
    Serial.print(temp.temperature);
    Serial.println(" deg C");

    /* Display the results (acceleration is measured in m/s^2) */
    Serial.print("\t\tAccel X: ");
    Serial.print(accel.acceleration.x);
    Serial.print(" \tY: ");
    Serial.print(accel.acceleration.y);
    Serial.print(" \tZ: ");
    Serial.print(accel.acceleration.z);
    Serial.println(" m/s^2 ");

    /* Display the results (rotation is measured in rad/s) */
    Serial.print("\t\tGyro X: ");
    Serial.print(gyro.gyro.x);
    Serial.print(" \tY: ");
    Serial.print(gyro.gyro.y);
    Serial.print(" \tZ: ");
    Serial.print(gyro.gyro.z);
    Serial.println(" radians/s ");

    /* Display the results (magnetic field is measured in uTesla) */
    Serial.print("\t\tMagneto X: "); Serial.print(mag.magnetic.x);
    Serial.print(" \tY: "); Serial.print(mag.magnetic.y); 
    Serial.print(" \tZ: "); Serial.print(mag.magnetic.z); 
    Serial.println(" uTesla ");

    // display the output of the filter
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
    filter.getQuaternion(&qr, &qi, &qj, &qk);
    Serial.print("Quaternion: ");
    Serial.print(qr, 4);
    Serial.print(", ");
    Serial.print(qi, 4);
    Serial.print(", ");
    Serial.print(qj, 4);
    Serial.print(", ");
    Serial.println(qk, 4);  

    // perform the quaternion transform to go into a NED or similar referential
    vec3 accel_raw;
    vec3 accel_NED;
    quat quat_rotation;
    
    vec3_setter(&accel_raw, accel.acceleration.x, accel.acceleration.y, accel.acceleration.z);
    quat_setter(&quat_rotation, qr, qi, qj, qk);
    rotate_by_quat_R(&accel_raw, &quat_rotation, &accel_NED);
    
    Serial.print("rotated acc: ");
    Serial.print(accel_NED.i, 4); Serial.print(", ");
    Serial.print(accel_NED.j, 4); Serial.print(", ");
    Serial.print(accel_NED.k, 4); Serial.print(", ");
    Serial.println();

    Serial.println();
  }

}
