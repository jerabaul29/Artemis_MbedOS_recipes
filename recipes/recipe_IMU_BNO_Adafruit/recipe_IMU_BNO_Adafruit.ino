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
#include "WDT.h"

#include <Adafruit_BNO08x.h>

Adafruit_BNO08x bno08x{};
sh2_SensorValue_t sensorValue;

APM3_WDT wdt;

constexpr unsigned long sample_period_microseconds {5000UL};
bool constexpr verbose_report {false};

constexpr unsigned long baudrate {115200};

volatile bool dummy_var_for_wait {false};
volatile int remaining_watchdog_tries {10};

// Interrupt handler for the watchdog
extern "C" void am_watchdog_isr(void)
{
  // Clear the watchdog interrupt
  wdt.clear();
  
  if (remaining_watchdog_tries > 0){
    wdt.restart();
    remaining_watchdog_tries -= 1;
    
      SPI.end();
      power_adc_disable();
      Serial.end();
      Wire.end();
      //Force the peripherals off
      am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_IOM0);
      am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_IOM1);
      am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_IOM2);
      am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_IOM3);
      am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_IOM4);
      am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_IOM5);
      am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_ADC);
      am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_UART0);
      am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_UART1);

      for (int i=0; i<1000000; i++){
        dummy_var_for_wait = !dummy_var_for_wait;
      }
      wdt.restart();

      am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_IOM0);
      am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_IOM1);
      am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_IOM2);
      am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_IOM3);
      am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_IOM4);
      am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_IOM5);
      am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_ADC);
      am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_UART0);
      am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_UART1);

      for (int i=0; i<1000000; i++){
        dummy_var_for_wait = !dummy_var_for_wait;
      }
      wdt.restart();

      Wire.begin();
      SPI.begin();
      Serial.begin(baudrate);

      for (int i=0; i<1000000; i++){
        dummy_var_for_wait = !dummy_var_for_wait;
      }
      wdt.restart();

      Serial.print(F("retry I2C "));
    
  }
  else{
    // reset
    while (true){
      ;
    }
  }
}

void resetArtemis(void)
{
  Wire.end(); //Power down I2C

  SPI.end(); //Power down SPI

  power_adc_disable(); //Power down ADC. It it started by default before setup().

  Serial.end(); //Power down UART

  //Force the peripherals off
  am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_IOM0);
  am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_IOM1);
  am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_IOM2);
  am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_IOM3);
  am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_IOM4);
  am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_IOM5);
  am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_ADC);
  am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_UART0);
  am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_UART1);

  while (1) // That's all folks! Artemis will watchdog reset in 1.25 seconds
    ;
}

void setReports(void) {
  if (!bno08x.enableReport(SH2_ROTATION_VECTOR, sample_period_microseconds)) {
    Serial.println("Could not enable rotation vector");
  }
  if (!bno08x.enableReport(SH2_ACCELEROMETER, sample_period_microseconds)) {
    Serial.println("Could not enable rotation vector");
  }
}

void setup(void) {
  wdt.start();
  
  Serial.begin(baudrate);
  while (!Serial){
    delay(10);
  }

  Serial.println();
  Serial.println("------ booted ------");
  Serial.println();

  // start wire
  Wire.end();
  delay(50);
  Wire.begin();
  delay(50);

  // we may hang at I2C begin; if this is the case, try to re-start I2C with the watchdog
  // TODO: jump back here with the watchdog?
  remaining_watchdog_tries = 10;

  // initialize
  for (int i=0; i<60;i++){
    wdt.restart();
    if (bno08x.begin_I2C(0x4B, &Wire, 0)){
      Serial.println(F("found the Artemis!"));
      break;
    }
    else{
      Serial.print("Failed to find BNO08x chip "); Serial.println(i+1);
      if (i == 19){
        Serial.println(F("reset Artemis"));
        resetArtemis();
      }
      Wire.end();
      delay(50);
      Wire.begin();
      delay(50);
      
      delay(500);
    }
  }

  // now, no more reason to allow some re-tries with the watchdog!
  remaining_watchdog_tries = 0;

  Wire.setClock(400000);
  delay(50);
  
  setReports();

  Serial.println("----- Reading events -----");
  delay(100);
}

float accel_norm;
float quat_norm;

void loop() {
  wdt.restart();

  if (bno08x.wasReset()) {
    Serial.println("sensor was reset ");
    setReports();
  }

  if (bno08x.getSensorEvent(&sensorValue)) {

    switch (sensorValue.sensorId) {
  
    case SH2_ACCELEROMETER:
      if (verbose_report){
        Serial.print("Accelerometer - x: ");
        Serial.print(sensorValue.un.accelerometer.x);
        Serial.print(" y: ");
        Serial.print(sensorValue.un.accelerometer.y);
        Serial.print(" z: ");
        Serial.println(sensorValue.un.accelerometer.z);
      }
  
       accel_norm = sqrt(
          sensorValue.un.accelerometer.x * sensorValue.un.accelerometer.x +
          sensorValue.un.accelerometer.y * sensorValue.un.accelerometer.y +
          sensorValue.un.accelerometer.z * sensorValue.un.accelerometer.z
       );
  
      if (abs(accel_norm - 9.81) > 10.0){
        Serial.println(F("**************************************************"));
        Serial.println(F("***** WARNING: VERY HIGH ACCELS, IS IT TRUE? *****"));
        Serial.println(F("**************************************************"));
        Serial.print(F("accel norm: ")); Serial.println(accel_norm);
      }
      
      break;
  
    case SH2_ROTATION_VECTOR:
      if (verbose_report){
        Serial.print("Rotation Vector - r: ");
        Serial.print(sensorValue.un.rotationVector.real);
        Serial.print(" i: ");
        Serial.print(sensorValue.un.rotationVector.i);
        Serial.print(" j: ");
        Serial.print(sensorValue.un.rotationVector.j);
        Serial.print(" k: ");
        Serial.println(sensorValue.un.rotationVector.k);
      }
  
      quat_norm = sensorValue.un.rotationVector.real * sensorValue.un.rotationVector.real
                      + sensorValue.un.rotationVector.i * sensorValue.un.rotationVector.i
                      + sensorValue.un.rotationVector.j * sensorValue.un.rotationVector.j
                      + sensorValue.un.rotationVector.k * sensorValue.un.rotationVector.k;
                      
      if (abs(quat_norm - 1.0f) > 1.0e-4){
        Serial.println(F("**********************************************************************"));
        Serial.println(F("************************ ERROR: NON UNIT QUAT ************************"));
        Serial.println(F("**********************************************************************"));
        Serial.print(F("quat norm: ")); Serial.println(quat_norm);
      }
      break;
    }
  }
}
