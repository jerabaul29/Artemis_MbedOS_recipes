#include "Arduino.h"
#include <OneWire.h>
#include "etl.h"
#include "etl/vector.h"
#include "etl/algorithm.h"

#define ONE_WIRE_PIN 35  // the data pin; I suggest to put a 100 Ohm between the pin and the sensors; this way, if a cable is cut / shorted, will not burn the pin
#define ONE_WIRE_POWER 4  // the power pin; use a digital pin to be able to switch power on and off
                          // I suggest putting a 100 ohm resistor between the pin out and the sensors; this way, if a cable is cut / shorted, will not burn the pin
                          // 100 ohm sees to work still fine

// the maximum number of sensors the sketch can handle
#define MAX_NBR_OF_SENSORS 12

OneWire ds(ONE_WIRE_PIN);  // on pin 10 (a pullup resistor is necessary; value may depend on the board (!) and the number of sensors)

using Address = byte[8];
etl::vector<uint64_t, MAX_NBR_OF_SENSORS> vector_of_ids;
etl::vector<float, MAX_NBR_OF_SENSORS> vector_of_measurements;

void address_to_uint64_t(Address & addr_in, uint64_t & uint64_result){
  uint64_result = 0;
  for (int crrt_byte_index=0; crrt_byte_index<8; crrt_byte_index++){
    uint64_result += static_cast<uint64_t>( addr_in[crrt_byte_index] ) << ( 8 * (7 - crrt_byte_index) );
  }
}

void uint64_t_to_address(uint64_t const & uint64_in, Address & addr_out){
  for (int crrt_byte_index=0; crrt_byte_index<8; crrt_byte_index++){
    addr_out[crrt_byte_index] = static_cast<byte>(uint64_in >> (8 * (7 - crrt_byte_index)) );
  }
}

void print_uint64(uint64_t const & num){
  uint32_t low = num % 0xFFFFFFFF; 
  uint32_t high = (num >> 32) % 0xFFFFFFFF;
 
  Serial.print(high);
  Serial.print(low);
}

void print_address(Address const & addr){
    for( int i = 0; i < 8; i++) {
      Serial.print(addr[i], HEX);
      Serial.write(' ');
    }
}

void look_for_sensors(etl::ivector<uint64_t> & vec_in){
  Address crrt_addr;
  uint64_t crrt_id;

  // just to check that id to addr conversion and back work
  Address test_addr;
  
  vec_in.clear();

  while (true){
    
    if ( !ds.search(crrt_addr) ) {
      Serial.println("No more addresses.");
      ds.reset_search();
      delay(250);

      break;
    }

    Serial.println(F("found new address..."));
    Serial.print("ROM =");
    print_address(crrt_addr);
    Serial.println();

    address_to_uint64_t(crrt_addr, crrt_id);
    Serial.print(F("ID = ")); print_uint64(crrt_id); Serial.println();

    // just to test that ID conversion to addr and back work
    uint64_t_to_address(crrt_id, test_addr);
    for (int byte_ind =  0; byte_ind<8; byte_ind++){
      if (!(crrt_addr[byte_ind] == test_addr[byte_ind])){
        Serial.println(F("ERROR: did not recover right address"));
        Serial.print(crrt_addr[byte_ind], HEX); Serial.print(F(" vs ")); Serial.print(test_addr[byte_ind], HEX); Serial.println();
      }
    }

    if (OneWire::crc8(crrt_addr, 7) != crrt_addr[7]) {
      Serial.println("WARNING: CRC is not valid!");
      continue;
    }
    else{
      Serial.println(F("CRC is valid"));
    }

    // the first ROM byte indicates which chip
    switch (crrt_addr[0]) {
      case 0x10:
        Serial.println("  WARNING: Chip = DS18S20");  // or old DS1820
        break;
      case 0x28:
        Serial.println("  CORRECT: Chip = DS18B20");
        vec_in.push_back(crrt_id);
        break;
      case 0x22:
        Serial.println("  WARNING: Chip = DS1822");
        break;
      default:
        Serial.println("EROR: Device is not a DS18x20 family device.");
        continue;
    } 
  }

  // we sort greater first; i.e., the sensors with greater IDs are sorted first;
  // i.e., if the thermistor string have greater IDs higher up, then the sensors are sorted from higher up to lower down
  etl::sort(vec_in.begin(), vec_in.end(), std::greater<uint64_t>());

  Serial.println(F("sorted list of IDs"));

  for (auto const & crrt_elem : vec_in){
    print_uint64(crrt_elem); Serial.println();
  }

  return;
}


void measure_all_sensors(etl::ivector<uint64_t> const & vec_sensors_IDs, etl::ivector<float> & vector_of_measurements){
  Address crrt_address;
  vector_of_measurements.clear();
  byte present = 0;
  byte data[12];
  float celsius;
  byte crc;

  // ask each sensor to start new measurement
  Serial.println(F("ask to start conversion..."));
  for (auto & crrt_id : vec_sensors_IDs){
    uint64_t_to_address(crrt_id, crrt_address);
    print_address(crrt_address); Serial.println();

    ds.reset();
    ds.select(crrt_address);
    ds.write(0x44, 1);        // start conversion, with parasite power on at the end
    delay(10);
  }

  // wait for the conversion to take place
  // in theory 750ms is enough, but take no chance
  Serial.println(F("wait for conversions to take place"));
  delay(1000);

  // collect the output of each sensor
  Serial.println(F("collect results..."));
  for (auto & crrt_id : vec_sensors_IDs){
    uint64_t_to_address(crrt_id, crrt_address);
    print_address(crrt_address); Serial.println();

    present = ds.reset();
    ds.select(crrt_address);    
    ds.write(0xBE);         // Read Scratchpad

    Serial.print("  Data = ");
    Serial.print(present, HEX);
    Serial.print(" ");
    for ( int i = 0; i < 9; i++) {           // we need 9 bytes
      data[i] = ds.read();
      Serial.print(data[i], HEX);
      Serial.print(" ");
    }
    Serial.print(" CRC=");
    crc = OneWire::crc8(data, 8);
    Serial.print(crc, HEX);
    if (crc != data[8]){
      Serial.println(F(" ERROR: non matching CRC"));
    }
    else{
      Serial.println(F(" CRC OK"));
    }

    // Convert the data to actual temperature
    // because the result is a 16 bit signed integer, it should
    // be stored to an "int16_t" type, which is always 16 bits
    // even when compiled on a 32 bit processor.
    int16_t raw = (data[1] << 8) | data[0];
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) {raw = raw & ~7; Serial.println(F("  WARNING: 9 bit res"));}  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) {raw = raw & ~3; Serial.println(F("  WARNING: 10 bit res"));} // 10 bit res, 187.5 ms
    else if (cfg == 0x40) {raw = raw & ~1; Serial.println(F("  WARNING: 11 bit res"));} // 11 bit res, 375 ms
    else if (cfg == 0x60) {Serial.println(F("  OK: 12 bits resolution"));}
    else {Serial.println(F("  ERROR: unknown resolution config!"));}

    celsius = (float)raw / 16.0;
    Serial.print("  Temperature = ");
    Serial.print(celsius);
    Serial.print(" Celsius");
    Serial.println();
  }
}

void setup(void) {
  delay(1000);

  Serial.begin(1000000);
  delay(100);
  Serial.println(F("----- BOOTED -----"));

  // how to turn power off
  pinMode(ONE_WIRE_PIN, INPUT);
  pinMode(ONE_WIRE_POWER, INPUT);
  delay(500);
  
  // turn on power to the OneWire sensors
  pinMode(ONE_WIRE_POWER, OUTPUT);
  digitalWrite(ONE_WIRE_POWER, HIGH);
  delay(500);

  Serial.println(F("look for sensors..."));
  look_for_sensors(vector_of_ids);
  Serial.println();
}


void loop(void) {
  Serial.println(F("measure all sensors..."));
  measure_all_sensors(vector_of_ids, vector_of_measurements);
  Serial.println();
}
