#include "Arduino.h"
#include <SPI.h>
#include "Adafruit_FRAM_SPI.h"

// ---------------------------------------------------------------------------------

/*
- in order to make this work, need to change a bit the Adafruit FRAM and busIO libs,
see the readme.
- heavily inspired from the examples for using SPI (from Sparkfun) and the example
for using FRAM (from Adafruit).
*/

// ---------------------------------------------------------------------------------

#define CS_PIN 10
#define SPI_SPEED 1000000
#define SPI_ORDER MSBFIRST
#define SPI_MODE SPI_MODE0

SPIClass my_spi(0); //This is default and automatically defined on RedBoard/ATP/Nano. Access using pins labeled SCK/MISO/MOSI (connects to pads 5/6/7 on module).
// SPIClass my_spi(1); //Use IO Master 1 on pads 8/9/10. See schematic of your board for pin locations.
// SPIClass mySPI(2); //Use IO Master 2 on pads 27/25/28
// SPIClass anotherSPI(3); //Use IO Master 3 on pads 42/43/44
// SPIClass SPI4(4); //Use IO Master 4 on pads 39/40/38
// SPIClass SPI5(5); //Use IO Master 5 on pads 48/49/47

// ---------------------------------------------------------------------------------

SPISettings mySettings(SPI_SPEED, SPI_ORDER, SPI_MODE);
Adafruit_FRAM_SPI fram = Adafruit_FRAM_SPI(CS_PIN, &my_spi);

uint8_t           addrSizeInBytes = 2; //Default to address size of two bytes
uint32_t          memSize;

int32_t readBack(uint32_t addr, int32_t data) {
  int32_t check = !data;
  int32_t wrapCheck, backup;
  fram.read(addr, (uint8_t*)&backup, sizeof(int32_t));
  fram.writeEnable(true);
  fram.write(addr, (uint8_t*)&data, sizeof(int32_t));
  fram.writeEnable(false);
  fram.read(addr, (uint8_t*)&check, sizeof(int32_t));
  fram.read(0, (uint8_t*)&wrapCheck, sizeof(int32_t));
  fram.writeEnable(true);
  fram.write(addr, (uint8_t*)&backup, sizeof(int32_t));
  fram.writeEnable(false);
  // Check for warparound, address 0 will work anyway
  if (wrapCheck==check)
    check = 0;
  return check;
}

bool testAddrSize(uint8_t addrSize) {
  fram.setAddressSize(addrSize);
  if (readBack(32, 0xbeefbead) == 0xbeefbead)
    return true;
  return false;
}

// ---------------------------------------------------------------------------------

void setup(){
  Serial.begin(115200);
  delay(100);
  Serial.println(F("booted"));

  // SPI.begin();
  // pinMode(CS_PIN, OUTPUT);

  if (fram.begin(addrSizeInBytes)) {
    Serial.println("Found SPI FRAM");
  } else {
    Serial.println("No SPI FRAM found ... check your connections\r\n");
    while (1);
  }

  if (testAddrSize(2))
    addrSizeInBytes = 2;
  else if (testAddrSize(3))
    addrSizeInBytes = 3;
  else if (testAddrSize(4))
    addrSizeInBytes = 4;
  else {
    Serial.println("SPI FRAM can not be read/written with any address size\r\n");
    while (1);
  }
  
  memSize = 256;
  Serial.println(F("check how many blocks are available, this may take a few secs"));
  while (readBack(memSize, memSize) == memSize) {
    memSize += 256;
    // Serial.print("Block: #"); Serial.println(memSize/256);  // disable the printing, as this is many lines...
  }
  
  Serial.print("SPI FRAM address size is ");
  Serial.print(addrSizeInBytes);
  Serial.println(" bytes.");
  Serial.println("SPI FRAM capacity appears to be..");
  Serial.print(memSize); Serial.println(" bytes");
  Serial.print(memSize/0x400); Serial.println(" kilobytes");
  Serial.print((memSize*8)/0x400); Serial.println(" kilobits");
  if (memSize >= (0x100000/8)) {
    Serial.print((memSize*8)/0x100000); Serial.println(" megabits");
  }

  // Read the first byte
  uint8_t test = fram.read8(0x0);
  Serial.print("Restarted "); Serial.print(test); Serial.println(" times");

  // Test write ++
  fram.writeEnable(true);
  fram.write8(0x0, test+1);
  fram.writeEnable(false);

  fram.writeEnable(true);
  fram.write(0x1, (uint8_t *)"FTW!", 5);
  fram.writeEnable(false);

  // dump the entire 512K of memory!
  uint8_t value;
  // for (uint16_t a = 0; a < 524288; a++) {  // this is a lot...
  for (uint16_t a = 0; a < 1024; a++) {  // let us dump a bit less!
    value = fram.read8(a);
    if ((a % 32) == 0) {
      Serial.print("\n 0x"); Serial.print(a, HEX); Serial.print(": ");
    }
    Serial.print("0x"); 
    if (value < 0x1) 
      Serial.print('0');
    Serial.print(value, HEX); Serial.print(" ");
  }
}

void loop(){

}
