/* Test sketch for Adafruit BNO08x sensor in UART-RVC mode */

#include "Arduino.h"
#include "Adafruit_BNO08x_RVC.h"

// our communications
Adafruit_BNO08x_RVC rvc = Adafruit_BNO08x_RVC();
BNO08x_RVC_Data heading;

Uart serial_bno_vcr{1, 9, 8};

// a few helper functions
void apply_yaw(float &vx, float &vy, float &vz, float const &yaw_rad){
  float vvx = vx;
  float vvy = vy;
  float vvz = vz;

  float cy = cos(yaw_rad);
  float sy = sin(yaw_rad);

  vx =  cy * vvx - sy * vvy;
  vy =  sy * vvx + cy * vvy;
  vz = vvz;
}

void apply_pitch(float &vx, float &vy, float &vz, float const &pitch_rad){
  float vvx = vx;
  float vvy = vy;
  float vvz = vz;

  float cp = cos(pitch_rad);
  float sp = sin(pitch_rad);

  vx =  cp * vvx + sp * vvz;
  vy =  vvy;
  vz = -sp * vvx + cp * vvz;
}

void apply_roll(float & vx, float & vy, float & vz, float const &roll_rad){
  float vvx = vx;
  float vvy = vy;
  float vvz = vz;

  float cr = cos(roll_rad);
  float sr = sin(roll_rad);

  vx = vvx;
  vy = cr * vvy - sr * vvz;
  vz = sr * vvy + cr * vvz;
}

// our yaw, pitch, roll, accx, accy, accz variables
float yaw_rad;
float pitch_rad;
float roll_rad;
float accx;
float accy;
float accz;
/*
float r_accx;
float r_accy;
float r_accz;
*/

// a few constants
float twopi = 6.283185307179586;

// the sketch

void setup() {
  // Wait for serial monitor to open
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  Serial.println("Adafruit BNO08x IMU - UART-RVC mode");

  serial_bno_vcr.begin(115200); // This is the baud rate specified by the datasheet
  while (!serial_bno_vcr)
    delay(10);

  if (!rvc.begin(&serial_bno_vcr)) { // connect to the sensor over hardware serial
    Serial.println("Could not find BNO08x!");
    while (1)
      delay(10);
  }

  Serial.println("BNO08x found!");
}

void loop() {

  while (!rvc.read(&heading)) {
    ;
  }

  // in raw coords
  /*
  Serial.print(F("Y: "));
  Serial.print(heading.yaw);
  Serial.print(F(" P: "));
  Serial.print(heading.pitch);
  Serial.print(F(" R: "));
  Serial.print(heading.roll);
  Serial.print(F(" X: "));
  Serial.print(heading.x_accel);
  Serial.print(F(" Y: "));
  Serial.print(heading.y_accel);
  Serial.print(F(" Z: "));
  Serial.println(heading.z_accel);
  */

  
  // getting everyting in the right dimension
  // all of this is, "in which direction did I move"? and "in IMU frame of reference"
  yaw_rad = -heading.yaw * twopi / 360.0f;
  pitch_rad = heading.roll * twopi / 360.0f;
  roll_rad = heading.pitch * twopi / 360.0f;
  accx = heading.x_accel;
  accy = heading.y_accel;
  accz = heading.z_accel;
  

  // in "correct" coords
  /*
  Serial.print(F("Yaw: "));
  Serial.print(yaw_rad * 360.0f / twopi);
  Serial.print(F(" Pitch: "));
  Serial.print(pitch_rad * 360.0f / twopi);
  Serial.print(F(" Roll: "));
  Serial.print(roll_rad * 360.0f / twopi);
  Serial.print(F(" accx: "));
  Serial.print(accx);
  Serial.print(F(" accy: "));
  Serial.print(accy);
  Serial.print(F(" z: "));
  Serial.println(accz);
  */

  // applying the transforms
  // need the - to "go back"
  // question: in which order?
  
  
  apply_roll(accx, accy, accz, roll_rad);
  apply_pitch(accx, accy, accz, pitch_rad); 
  apply_yaw(accx, accy, accz, yaw_rad);

 /*
  // in the initial frame of reference
  Serial.print(F("Yaw: "));
  Serial.print(yaw_rad * 360.0f / twopi);
  Serial.print(F(" Pitch: "));
  Serial.print(pitch_rad * 360.0f / twopi);
  Serial.print(F(" Roll: "));
  Serial.print(roll_rad * 360.0f / twopi);
  Serial.print(F(" || acc x init: "));
  Serial.print(accx, 2);
  Serial.print(F(" | acc y init: "));
  Serial.print(accy, 2);
  */
  Serial.print(F(" | acc down: "));
  Serial.println(accz, 2);
  

}
