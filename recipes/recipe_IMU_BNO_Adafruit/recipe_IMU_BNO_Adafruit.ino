/* NOTES

This is for using the BNO08X with SF Artemis boards.

--------------------------------------------------------------------------------------------
SPI issue

There is a small issue with the SF Artemis and the Adafruit BNO08x library out-of-the-box.
- Adafruit_BNO08x has dependency on both Adafruit_BusIO and Adafruit_Sensor
- Adafruit_BusIO misses some macros about SPI mapping on SF Artemis
- this means that Adafruit_BusIO cannot work at the moment with SF Artemis SPI, which is not a big deal
- but this results in compilation issues out-of-the-box

The solution is to modify the Adafruit_BusIO library with some dummy SPI code. This way, cannot work
with SPI sensor (not a big deal), but compiles and works fine with I2C sensors.

The dummy code fix was initially described at: https://forum.sparkfun.com/viewtopic.php?t=53898

Replace the Adafruit_BusIO/Adafruit_SPIDevice.cpp code for Adafruit_SPIDevice with:

 Adafruit_SPIDevice::Adafruit_SPIDevice(int8_t cspin, int8_t sckpin,
                                       int8_t misopin, int8_t mosipin,
                                       uint32_t freq, BitOrder dataOrder,
                                       uint8_t dataMode) {
  _cs = cspin;
  _sck = sckpin;
  _miso = misopin;
  _mosi = mosipin;

#ifdef BUSIO_USE_FAST_PINIO
  // csPort = (BusIO_PortReg *)portOutputRegister(digitalPinToPort(cspin));
  // csPinMask = digitalPinToBitMask(cspin);
  // if (mosipin != -1) {
    // mosiPort = (BusIO_PortReg *)portOutputRegister(digitalPinToPort(mosipin));
    // mosiPinMask = digitalPinToBitMask(mosipin);
  // }
  // if (misopin != -1) {
    // misoPort = (BusIO_PortReg *)portInputRegister(digitalPinToPort(misopin));
    // misoPinMask = digitalPinToBitMask(misopin);
  // }
  // clkPort = (BusIO_PortReg *)portOutputRegister(digitalPinToPort(sckpin));
  // clkPinMask = digitalPinToBitMask(sckpin);
#endif

  _freq = freq;
  _dataOrder = dataOrder;
  _dataMode = dataMode;
  _begun = false;
  _spiSetting = new SPISettings(freq, dataOrder, dataMode);
  _spi = NULL;
}

--------------------------------------------------------------------------------------------
Notes about using different suppliers of BNO08x boards

I got my first BNO08x from Sparkfun. They use the default I2C address: 0x4B . This seems to
be a different default to Adafruit. So to get the Adafruit code to work, need to override the
default argument for the sensor start-up:

bno08x.begin_I2C(0x4B, &Wire, 0)

*/

#include "Arduino.h"

#include <Adafruit_BNO08x.h>

Adafruit_BNO08x bno08x{};
sh2_SensorValue_t sensorValue;

void setReports(void) {
  Serial.println("Setting desired reports");
  if (!bno08x.enableReport(SH2_ACCELEROMETER)) {
    Serial.println("Could not enable accelerometer");
  }
  if (!bno08x.enableReport(SH2_GYROSCOPE_CALIBRATED)) {
    Serial.println("Could not enable gyroscope");
  }
  if (!bno08x.enableReport(SH2_MAGNETIC_FIELD_CALIBRATED)) {
    Serial.println("Could not enable magnetic field calibrated");
  }
  if (!bno08x.enableReport(SH2_LINEAR_ACCELERATION)) {
    Serial.println("Could not enable linear acceleration");
  }
  if (!bno08x.enableReport(SH2_ROTATION_VECTOR)) {
    Serial.println("Could not enable rotation vector");
  }
  if (!bno08x.enableReport(SH2_GEOMAGNETIC_ROTATION_VECTOR)) {
    Serial.println("Could not enable geomagnetic rotation vector");
  }
  if (!bno08x.enableReport(SH2_GAME_ROTATION_VECTOR)) {
    Serial.println("Could not enable game rotation vector");
  }
  if (!bno08x.enableReport(SH2_STEP_COUNTER)) {
    Serial.println("Could not enable step counter");
  }
  if (!bno08x.enableReport(SH2_STABILITY_CLASSIFIER)) {
    Serial.println("Could not enable stability classifier");
  }
  if (!bno08x.enableReport(SH2_RAW_ACCELEROMETER)) {
    Serial.println("Could not enable raw accelerometer");
  }
  if (!bno08x.enableReport(SH2_RAW_GYROSCOPE)) {
    Serial.println("Could not enable raw gyroscope");
  }
  if (!bno08x.enableReport(SH2_RAW_MAGNETOMETER)) {
    Serial.println("Could not enable raw magnetometer");
  }
  if (!bno08x.enableReport(SH2_SHAKE_DETECTOR)) {
    Serial.println("Could not enable shake detector");
  }
  if (!bno08x.enableReport(SH2_PERSONAL_ACTIVITY_CLASSIFIER)) {
    Serial.println("Could not enable personal activity classifier");
  }
}

void printActivity(uint8_t activity_id) {
  switch (activity_id) {
  case PAC_UNKNOWN:
    Serial.print("Unknown");
    break;
  case PAC_IN_VEHICLE:
    Serial.print("In Vehicle");
    break;
  case PAC_ON_BICYCLE:
    Serial.print("On Bicycle");
    break;
  case PAC_ON_FOOT:
    Serial.print("On Foot");
    break;
  case PAC_STILL:
    Serial.print("Still");
    break;
  case PAC_TILTING:
    Serial.print("Tilting");
    break;
  case PAC_WALKING:
    Serial.print("Walking");
    break;
  case PAC_RUNNING:
    Serial.print("Running");
    break;
  case PAC_ON_STAIRS:
    Serial.print("On Stairs");
    break;
  default:
    Serial.print("NOT LISTED");
  }
  Serial.print(" (");
  Serial.print(activity_id);
  Serial.print(")");
}

void setup(void) {
  Serial.begin(115200);
  while (!Serial){
    delay(10);
  }

  Serial.println("------ booted ------");

  // initialize
  while (!bno08x.begin_I2C(0x4B, &Wire, 0)) {
    Serial.println("Failed to find BNO08x chip");
    delay(500);
  }

  Wire.setClock(400000);
  delay(50);

  for (int n = 0; n < bno08x.prodIds.numEntries; n++) {
    Serial.print("Part ");
    Serial.print(bno08x.prodIds.entry[n].swPartNumber);
    Serial.print(": Version :");
    Serial.print(bno08x.prodIds.entry[n].swVersionMajor);
    Serial.print(".");
    Serial.print(bno08x.prodIds.entry[n].swVersionMinor);
    Serial.print(".");
    Serial.print(bno08x.prodIds.entry[n].swVersionPatch);
    Serial.print(" Build ");
    Serial.println(bno08x.prodIds.entry[n].swBuildNumber);
  }

  setReports();

  Serial.println("----- Reading events -----");
  delay(100);
}

void loop() {
  delay(10);

  if (bno08x.wasReset()) {
    Serial.print("sensor was reset ");
    setReports();
  }

  if (!bno08x.getSensorEvent(&sensorValue)) {
    return;
  }

  switch (sensorValue.sensorId) {

  case SH2_ACCELEROMETER:
    Serial.print("Accelerometer - x: ");
    Serial.print(sensorValue.un.accelerometer.x);
    Serial.print(" y: ");
    Serial.print(sensorValue.un.accelerometer.y);
    Serial.print(" z: ");
    Serial.println(sensorValue.un.accelerometer.z);
    break;
  case SH2_GYROSCOPE_CALIBRATED:
    Serial.print("Gyro - x: ");
    Serial.print(sensorValue.un.gyroscope.x);
    Serial.print(" y: ");
    Serial.print(sensorValue.un.gyroscope.y);
    Serial.print(" z: ");
    Serial.println(sensorValue.un.gyroscope.z);
    break;
  case SH2_MAGNETIC_FIELD_CALIBRATED:
    Serial.print("Magnetic Field - x: ");
    Serial.print(sensorValue.un.magneticField.x);
    Serial.print(" y: ");
    Serial.print(sensorValue.un.magneticField.y);
    Serial.print(" z: ");
    Serial.println(sensorValue.un.magneticField.z);
    break;
  case SH2_LINEAR_ACCELERATION:
    Serial.print("Linear Acceration - x: ");
    Serial.print(sensorValue.un.linearAcceleration.x);
    Serial.print(" y: ");
    Serial.print(sensorValue.un.linearAcceleration.y);
    Serial.print(" z: ");
    Serial.println(sensorValue.un.linearAcceleration.z);
    break;
  case SH2_ROTATION_VECTOR:
    Serial.print("Rotation Vector - r: ");
    Serial.print(sensorValue.un.rotationVector.real);
    Serial.print(" i: ");
    Serial.print(sensorValue.un.rotationVector.i);
    Serial.print(" j: ");
    Serial.print(sensorValue.un.rotationVector.j);
    Serial.print(" k: ");
    Serial.println(sensorValue.un.rotationVector.k);
    break;
  case SH2_GEOMAGNETIC_ROTATION_VECTOR:
    Serial.print("Geo-Magnetic Rotation Vector - r: ");
    Serial.print(sensorValue.un.geoMagRotationVector.real);
    Serial.print(" i: ");
    Serial.print(sensorValue.un.geoMagRotationVector.i);
    Serial.print(" j: ");
    Serial.print(sensorValue.un.geoMagRotationVector.j);
    Serial.print(" k: ");
    Serial.println(sensorValue.un.geoMagRotationVector.k);
    break;

  case SH2_GAME_ROTATION_VECTOR:
    Serial.print("Game Rotation Vector - r: ");
    Serial.print(sensorValue.un.gameRotationVector.real);
    Serial.print(" i: ");
    Serial.print(sensorValue.un.gameRotationVector.i);
    Serial.print(" j: ");
    Serial.print(sensorValue.un.gameRotationVector.j);
    Serial.print(" k: ");
    Serial.println(sensorValue.un.gameRotationVector.k);
    break;

  case SH2_STEP_COUNTER:
    Serial.print("Step Counter - steps: ");
    Serial.print(sensorValue.un.stepCounter.steps);
    Serial.print(" latency: ");
    Serial.println(sensorValue.un.stepCounter.latency);
    break;

  case SH2_STABILITY_CLASSIFIER: {
    Serial.print("Stability Classification: ");
    sh2_StabilityClassifier_t stability = sensorValue.un.stabilityClassifier;
    switch (stability.classification) {
    case STABILITY_CLASSIFIER_UNKNOWN:
      Serial.println("Unknown");
      break;
    case STABILITY_CLASSIFIER_ON_TABLE:
      Serial.println("On Table");
      break;
    case STABILITY_CLASSIFIER_STATIONARY:
      Serial.println("Stationary");
      break;
    case STABILITY_CLASSIFIER_STABLE:
      Serial.println("Stable");
      break;
    case STABILITY_CLASSIFIER_MOTION:
      Serial.println("In Motion");
      break;
    }
    break;
  }

  case SH2_RAW_ACCELEROMETER:
    Serial.print("Raw Accelerometer - x: ");
    Serial.print(sensorValue.un.rawAccelerometer.x);
    Serial.print(" y: ");
    Serial.print(sensorValue.un.rawAccelerometer.y);
    Serial.print(" z: ");
    Serial.println(sensorValue.un.rawAccelerometer.z);
    break;
  case SH2_RAW_GYROSCOPE:
    Serial.print("Raw Gyro - x: ");
    Serial.print(sensorValue.un.rawGyroscope.x);
    Serial.print(" y: ");
    Serial.print(sensorValue.un.rawGyroscope.y);
    Serial.print(" z: ");
    Serial.println(sensorValue.un.rawGyroscope.z);
    break;
  case SH2_RAW_MAGNETOMETER:
    Serial.print("Raw Magnetic Field - x: ");
    Serial.print(sensorValue.un.rawMagnetometer.x);
    Serial.print(" y: ");
    Serial.print(sensorValue.un.rawMagnetometer.y);
    Serial.print(" z: ");
    Serial.println(sensorValue.un.rawMagnetometer.z);
    break;

  case SH2_SHAKE_DETECTOR: {
    Serial.print("Shake Detector - shake detected on axis: ");
    sh2_ShakeDetector_t detection = sensorValue.un.shakeDetector;
    switch (detection.shake) {
    case SHAKE_X:
      Serial.println("X");
      break;
    case SHAKE_Y:
      Serial.println("Y");
      break;
    case SHAKE_Z:
      Serial.println("Z");
      break;
    default:
      Serial.println("None");
      break;
    }
  }

  case SH2_PERSONAL_ACTIVITY_CLASSIFIER: {

    sh2_PersonalActivityClassifier_t activity =
        sensorValue.un.personalActivityClassifier;
    uint8_t page_num = activity.page;
    Serial.print("Activity classification - Most likely: ");
    printActivity(activity.mostLikelyState);
    Serial.println("");

    Serial.println("Confidences:");
    // if PAC_OPTION_COUNT is ever > 10, we'll need to
    // care about page
    for (uint8_t i = 0; i < PAC_OPTION_COUNT; i++) {
      Serial.print("\t");
      printActivity(i);
      Serial.print(": ");
      Serial.println(activity.confidence[i]);
    }
  }
  }
}


