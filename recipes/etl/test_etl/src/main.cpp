#include "mbed.h"

#include "etl/array.h"
#include "etl/circular_buffer.h"

//////////////////////////////////////////////////
// helper functions

template<typename T, size_t size>
void print_array(const etl::array<T, size> & array){
  Serial.print(F("array: "));

  for (auto && crrt_value : array){
    Serial.print(crrt_value);
    Serial.print(F(" "));
  }
  Serial.println();
}

template<typename T, size_t max_size>
void print_circular_buffer(const etl::circular_buffer<T, max_size> circular_buffer){
  Serial.print(F("circular buffer, max size "));
  Serial.print(circular_buffer.max_size());
  Serial.print(F(" , size "));
  Serial.print(circular_buffer.size());
  Serial.print(F(" , capacity "));
  Serial.print(circular_buffer.capacity());
}

//////////////////////////////////////////////////

int main(){

  delay(750);

  Serial.begin(115200);
  delay(5);
  Serial.println(F("booted"));

  //////////////////////////////////////////////////  
  // array
  Serial.println(F("working with array"));

  etl::array<int, 8> test_int_array;

  Serial.println(F("fill with the value 12"));
  test_int_array.fill(12);

  print_array(test_int_array);

  Serial.println(F("fill array with some values"));

  size_t crrt_index = 0;
  for (auto && crrt_elem : test_int_array){
      crrt_elem = crrt_index;
      crrt_index += 1;
  }

  print_array(test_int_array);

  Serial.print(F("access last element: "));
  Serial.println(test_int_array[test_int_array.size()-1]);

  //////////////////////////////////////////////////
  Serial.println(F("working with circular buffer"));

  etl::circular_buffer<int, 16> circular_buffer;

  print_circular_buffer(circular_buffer);

  Serial.println(F("fill with 8 elements"));

  for (int i=0; i<8; i++){
    circular_buffer.push(i);
  }

  print_circular_buffer(circular_buffer);

  ////////////////////////////////////////////////// 
  while (true){
    
  }
}