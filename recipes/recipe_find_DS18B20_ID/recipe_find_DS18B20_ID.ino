#include <OneWire.h>

// the digital pin, or whatever pin you want to use
// remember to use a pullup; the value may depend on the MCU used; in my case, with the Artemis, 39k works
// OneWire ds(35);  // for AGT on pin D35
OneWire ds(2);  // for RedBoard on pin 2

// ------------------------------------------------------------
// helper functions

using Address = byte[8];

void address_to_uint64_t(Address &addr_in, uint64_t &uint64_result) {
      uint64_result = 0;
  for (int crrt_byte_index = 0; crrt_byte_index < 8; crrt_byte_index++) {
        uint64_result += static_cast<uint64_t>(addr_in[crrt_byte_index])
                     << (8 * (7 - crrt_byte_index));
  }
}

void uint64_t_to_address(uint64_t const &uint64_in, Address &addr_out) {
      for (int crrt_byte_index = 0; crrt_byte_index < 8; crrt_byte_index++) {
        addr_out[crrt_byte_index] =
        static_cast<byte>(uint64_in >> (8 * (7 - crrt_byte_index)));
  }
}

void print_uint64(uint64_t const &num) {
  uint32_t low = num % 0xFFFFFFFF;
  uint32_t high = (num >> 32) % 0xFFFFFFFF;

  Serial.print(high);
  Serial.print(low);
}

void print_address(Address const &addr) {
      for (int i = 0; i < 8; i++) {
        Serial.print(addr[i], HEX);
    Serial.write(' ');
  }
}

void uint64_t_to_6bits_int_id(uint64_t const &uint64_in,
                              uint8_t &uint8_6bits_id) {
      Address crrt_address;
  uint64_t_to_address(uint64_in, crrt_address);

  // we know that:
  // the first byte is the "kind", should be 0x28
  // the next 6 bytes are the serial number
  // the LS bytes are first; then end is still 0x00 0x00 (as production has not
  // gone so far) the last byte is the CRC so, use the 6 LSBs of the LS byte of
  // the serial number (these are the ones that "rotate" fastest)

  byte byte_to_use = crrt_address[1];
  byte byte_result = byte_to_use & 0b00111111;
  uint8_6bits_id = byte_result;
}

// ------------------------------------------------------------
// sketch

void setup() {
  delay(1000);
  Serial.begin(1000000);
  delay(500);
  Serial.println(F("--- BOOTED ---"));
}

void loop() {

  Serial.println();
  Serial.println();
  Serial.println();
  Serial.println(F("--- START LOOP ---"));

  while (true) {
    byte i;
    byte present = 0;
    byte type_s;
    byte data[12];
    Address addr;
    uint64_t sensor_id;
    uint8_t sensor_id_6_bits;
    float celsius, fahrenheit;

    if (!ds.search(addr)) {
      Serial.println("No more addresses.");
      Serial.println("--- END LOOP ---");
      ds.reset_search();
      delay(5000);
      break;
    }

    Serial.print("ROM =");
    for (i = 0; i < 8; i++) {
      Serial.write(' ');
      Serial.print(addr[i], HEX);
    }

    if (OneWire::crc8(addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return;
    }
    Serial.println();

    address_to_uint64_t(addr, sensor_id);
    uint64_t_to_6bits_int_id(sensor_id, sensor_id_6_bits);

    Serial.print(F("  ID 64 bits: ")); print_uint64(sensor_id); Serial.println();
    Serial.print(F("  ID 6 LSB bits: ")); Serial.println(sensor_id_6_bits);

    // the first ROM byte indicates which chip
    switch (addr[0]) {
    case 0x10:
      Serial.println("  Chip = DS18S20"); // or old DS1820
      type_s = 1;
      break;
    case 0x28:
      Serial.println("  Chip = DS18B20");
      type_s = 0;
      break;
    case 0x22:
      Serial.println("  Chip = DS1822");
      type_s = 0;
      break;
    default:
      Serial.println("Device is not a DS18x20 family device.");
      return;
    }

    ds.reset();
    ds.select(addr);
    ds.write(0x44, 1); // start conversion, with parasite power on at the end

    delay(1000); // maybe 750ms is enough, maybe not
    // we might do a ds.depower() here, but the reset will take care of it.

    present = ds.reset();
    ds.select(addr);
    ds.write(0xBE); // Read Scratchpad

    Serial.print("  Data = ");
    Serial.print(present, HEX);
    Serial.print(" ");
    for (i = 0; i < 9; i++) { // we need 9 bytes
      data[i] = ds.read();
      Serial.print(data[i], HEX);
      Serial.print(" ");
    }
    Serial.print(" CRC=");
    Serial.print(OneWire::crc8(data, 8), HEX);
    Serial.println();

    // Convert the data to actual temperature
    // because the result is a 16 bit signed integer, it should
    // be stored to an "int16_t" type, which is always 16 bits
    // even when compiled on a 32 bit processor.
    int16_t raw = (data[1] << 8) | data[0];
    if (type_s) {
      raw = raw << 3; // 9 bit resolution default
      if (data[7] == 0x10) {
        // "count remain" gives full 12 bit resolution
        raw = (raw & 0xFFF0) + 12 - data[6];
      }
    } else {
      byte cfg = (data[4] & 0x60);
      // at lower res, the low bits are undefined, so let's zero them
      if (cfg == 0x00)
        raw = raw & ~7; // 9 bit resolution, 93.75 ms
      else if (cfg == 0x20)
        raw = raw & ~3; // 10 bit res, 187.5 ms
      else if (cfg == 0x40)
        raw = raw & ~1; // 11 bit res, 375 ms
      //// default is 12 bit resolution, 750 ms conversion time
    }
    celsius = (float)raw / 16.0;
    fahrenheit = celsius * 1.8 + 32.0;
    Serial.print("  Temperature = ");
    Serial.print(celsius);
    Serial.print(" Celsius, ");
    Serial.print(fahrenheit);
    Serial.println(" Fahrenheit");
  }
}
