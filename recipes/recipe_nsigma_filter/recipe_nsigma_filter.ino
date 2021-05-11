#include "Arduino.h"
#include "statistical_processing.h"

#include "etl.h"
#include "etl/vector.h"

void setup(){
  delay(100);
  Serial.begin(1000000);
  delay(100);
  Serial.println();
  Serial.println(F("booted"));

  {
  long value = 12345678;
  Serial.print(F("test all same value ")); Serial.print(value); Serial.print(F(": "));
  etl::vector<long, 5> vec_in;
  vec_in.push_back(value);
  vec_in.push_back(value);
  vec_in.push_back(value);
  vec_in.push_back(value);
  vec_in.push_back(value);
  long val_out = sigma_filter(vec_in, 2.0);
  Serial.print(F("val_out=")); Serial.println(val_out);
  }

  {
  double value = 12345678.0;
  Serial.print(F("test around ")); Serial.print(value); Serial.print(F(": "));
  etl::vector<double, 5> vec_in;
  vec_in.push_back(value -15);
  vec_in.push_back(value +12);
  vec_in.push_back(value -18);
  vec_in.push_back(value +4);
  vec_in.push_back(value -7);
  long val_out = sigma_filter(vec_in, 2.0);
  Serial.print(F("val_out=")); Serial.println(val_out);
  }
}

void loop(){

}
