#include "Arduino.h"

#define STAT_PROCESSING_VERBOSE 1
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
  Serial.println();
  Serial.println(F("test empty vector "));
  etl::vector<long, 5> vec_in;
  vec_in.clear();
  long val_out = accurate_sigma_filter(vec_in, 2.0);
  Serial.print(F("val_out=")); Serial.println(val_out);
  }

  {
  Serial.println();
  long value = 12345678;
  Serial.print(F("test too low nsigma ")); Serial.print(value); Serial.println(F(": "));
  etl::vector<long, 5> vec_in;
  vec_in.clear();
  vec_in.push_back(value-1);
  vec_in.push_back(value+1);
  long val_out = accurate_sigma_filter(vec_in, 0.5);
  Serial.print(F("val_out=")); Serial.println(val_out);
  }

  {
  Serial.println();
  long value = 12345678;
  Serial.print(F("test all same value ")); Serial.print(value); Serial.println(F(": "));
  etl::vector<long, 5> vec_in;
  vec_in.push_back(value);
  vec_in.push_back(value);
  vec_in.push_back(value);
  vec_in.push_back(value);
  vec_in.push_back(value);
  long val_out = accurate_sigma_filter(vec_in, 2.0);
  Serial.print(F("val_out=")); Serial.println(val_out);
  }

  {
  Serial.println();
  double value = 12345678.0;
  Serial.print(F("test around ")); Serial.print(value); Serial.println(F(": "));
  etl::vector<double, 5> vec_in;
  vec_in.push_back(value -15);
  vec_in.push_back(value +12);
  vec_in.push_back(value -18);
  vec_in.push_back(value +4);
  vec_in.push_back(value -7);
  double val_out = accurate_sigma_filter(vec_in, 2.0);
  Serial.print(F("val_out=")); Serial.println(val_out);
  }

  {
  // an unsigned case; this should not be accepted
  /*
  Serial.println();
  unsigned long value = 12345678;
  Serial.print(F("test around ")); Serial.print(value); Serial.print(F(": "));
  etl::vector<unsigned long, 5> vec_in;
  vec_in.push_back(value -15UL);
  vec_in.push_back(value -10UL);
  vec_in.push_back(value +0UL);
  vec_in.push_back(value +10UL);
  vec_in.push_back(value +15UL);
  unsigned long val_out = sigma_filter(vec_in, 2.0);
  Serial.print(F("val_out=")); Serial.println(val_out);
  */
  }

  {
  Serial.println();
  int value = 12345678;
  Serial.print(F("test around ")); Serial.print(value); Serial.println(F(": "));
  etl::vector<int, 5> vec_in;
  vec_in.push_back(value -15);
  vec_in.push_back(value -10);
  vec_in.push_back(value +0);
  vec_in.push_back(value +10);
  vec_in.push_back(value +15);
  int val_out = accurate_sigma_filter(vec_in, 2.0);
  Serial.print(F("val_out=")); Serial.println(val_out);
  }
  
  {
  Serial.println();
  long value = 12345678;
  Serial.print(F("test with outlier ")); Serial.print(value); Serial.println(F(": "));
  etl::vector<long, 6> vec_in;
  vec_in.push_back(value -5);
  vec_in.push_back(value -10);
  vec_in.push_back(value +100000);
  vec_in.push_back(value +1);
  vec_in.push_back(value +10);
  vec_in.push_back(value +15);
  long val_out =accurate_sigma_filter(vec_in, 2.0, false);
  Serial.print(F("val_out=")); Serial.println(val_out);
  }

}

void loop(){

}
