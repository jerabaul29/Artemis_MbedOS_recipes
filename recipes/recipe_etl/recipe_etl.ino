//#include "mbed.h"

//#include <Embedded_Template_Library.h>

#include "etl.h"

#include "etl/array.h"
#include "etl/circular_buffer.h"
#include "etl/vector.h"

#include "etl/mean.h"
#include "etl/standard_deviation.h"

//////////////////////////////////////////////////
// helper functions

template<typename T, size_t size>
void print_vector(const etl::vector<T, size> & vector){
  Serial.print(F("vector: "));

  for (auto && crrt_value : vector){
    Serial.print(crrt_value);
    Serial.print(F(" "));
  }
  Serial.println();
  Serial.print(F("current size: "));
  Serial.print(vector.size());
  Serial.print(F(" out of capacity: "));
  Serial.print(vector.capacity());
  Serial.println();
}

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
  Serial.println();
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

  for (int i=0; i<8; i++){
    circular_buffer.push(i);
  }

  print_circular_buffer(circular_buffer);

  //////////////////////////////////////////////////
  Serial.println(F("working with vector"));

  etl::vector<int, 32> vector;

  vector.push_back(31);
  vector.push_back(32);
  vector.push_back(33);
  vector.push_back(34);

  print_vector(vector);

  //////////////////////////////////////////////////
  Serial.println(F("test of mean and std; compute on the previous vector"));

  float mean = etl::mean(vector.begin(), vector.end());
  
  Serial.print(F("got mean ")); Serial.print(mean); Serial.println(F(" expected 32.5"));

  

  ////////////////////////////////////////////////// 
  while (true){
    
  }
}
