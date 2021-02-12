
//--------------------------------------------------------------------------------
// from example 14

// Artemis Tracker pin definitions
#define spiCS1              4  // D4 can be used as an SPI chip select or as a general purpose IO pin
#define geofencePin         10 // Input for the ZOE-M8Q's PIO14 (geofence) pin
#define busVoltagePin       13 // Bus voltage divided by 3 (Analog in)
#define iridiumSleep        17 // Iridium 9603N ON/OFF (sleep) pin: pull high to enable the 9603N
#define iridiumNA           18 // Input for the Iridium 9603N Network Available
#define LED                 19 // White LED
#define iridiumPwrEN        22 // ADM4210 ON: pull high to enable power for the Iridium 9603N
#define gnssEN              26 // GNSS Enable: pull low to enable power for the GNSS (via Q2)
#define superCapChgEN       27 // LTC3225 super capacitor charger: pull high to enable the super capacitor charger
#define superCapPGOOD       28 // Input for the LTC3225 super capacitor charger PGOOD signal
#define busVoltageMonEN     34 // Bus voltage monitor enable: pull high to enable bus voltage monitoring (via Q4 and Q3)
#define spiCS2              35 // D35 can be used as an SPI chip select or as a general purpose IO pin
#define iridiumRI           41 // Input for the Iridium 9603N Ring Indicator
// Make sure you do not have gnssEN and iridiumPwrEN enabled at the same time!
// If you do, bad things might happen to the AS179 RF switch!

void gnssON(void) // Enable power for the GNSS
{
  pinMode(gnssEN, OUTPUT); // Configure the pin which enables power for the ZOE-M8Q GNSS
  digitalWrite(gnssEN, LOW); // Disable GNSS power (HIGH = disable; LOW = enable)
}

void gnssOFF(void) // Disable power for the GNSS
{
  pinMode(gnssEN, INPUT_PULLUP); // Configure the pin which enables power for the ZOE-M8Q GNSS
  digitalWrite(gnssEN, HIGH); // Disable GNSS power (HIGH = disable; LOW = enable)
}

// Declares a Uart object called Serial using instance 1 of Apollo3 UART peripherals with RX on variant pin 25 and TX on pin 24
// (note, in this variant the pins map directly to pad, so pin === pad when talking about the pure Artemis module)
UART iridiumSerial(1, 25, 24);

#include <IridiumSBD.h> //http://librarymanager/All#IridiumSBDI2C
#define DIAGNOSTICS false // Change this to true to see IridiumSBD diagnostics
// Declare the IridiumSBD object (including the sleep (ON/OFF) and Ring Indicator pins)
IridiumSBD modem(iridiumSerial, iridiumSleep, iridiumRI);

void setup(){
  //--------------------------------------------------------------------------------
  // from example 14
  // Let's begin by setting up the I/O pins
   
  pinMode(LED, OUTPUT); // Make the LED pin an output

  gnssOFF(); // Disable power for the GNSS
  pinMode(geofencePin, INPUT); // Configure the geofence pin as an input

  pinMode(iridiumPwrEN, OUTPUT); // Configure the Iridium Power Pin (connected to the ADM4210 ON pin)
  digitalWrite(iridiumPwrEN, LOW); // Disable Iridium Power (HIGH = enable; LOW = disable)
  pinMode(superCapChgEN, OUTPUT); // Configure the super capacitor charger enable pin (connected to LTC3225 !SHDN)
  digitalWrite(superCapChgEN, LOW); // Disable the super capacitor charger (HIGH = enable; LOW = disable)
  pinMode(iridiumSleep, OUTPUT); // Iridium 9603N On/Off (Sleep) pin
  digitalWrite(iridiumSleep, LOW); // Put the Iridium 9603N to sleep (HIGH = on; LOW = off/sleep)
  pinMode(iridiumRI, INPUT); // Configure the Iridium Ring Indicator as an input
  pinMode(iridiumNA, INPUT); // Configure the Iridium Network Available as an input
  pinMode(superCapPGOOD, INPUT); // Configure the super capacitor charger PGOOD input

  pinMode(busVoltageMonEN, OUTPUT); // Make the Bus Voltage Monitor Enable an output
  digitalWrite(busVoltageMonEN, LOW); // Set it low to disable the measurement to save power
  analogReadResolution(14); //Set resolution to 14 bit
  //--------------------------------------------------------------------------------

  Serial.begin(115200);
  delay(10);
  Serial.println(F("booted"));
  delay(100);

  // Enable power for the 9603N
  Serial.println(F("Enabling 9603N power..."));
  digitalWrite(iridiumPwrEN, HIGH); // Enable Iridium Power
  delay(1000);

  // Enable the 9603N and start talking to it
  Serial.println(F("Beginning to talk to the 9603..."));

  // Start the serial port connected to the satellite modem
  iridiumSerial.begin(19200);
  delay(1000);

  // Relax timing constraints waiting for the supercap to recharge.
  modem.setPowerProfile(IridiumSBD::USB_POWER_PROFILE);

  // Begin satellite modem operation
  Serial.println(F("Starting modem..."));
  int err;
  err = modem.begin();

  // Check if the modem started correctly
  if (err != ISBD_SUCCESS){
    // If the modem failed to start, disable it and go to sleep
    Serial.print(F("***!!! modem.begin failed with error "));
    Serial.print(err);
    Serial.println(F(" !!!***"));
  }
  else{
    Serial.println(F("successful start of modem"));
  }
}

void loop(){
  
}

