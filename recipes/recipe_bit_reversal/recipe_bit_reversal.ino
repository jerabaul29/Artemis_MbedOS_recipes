unsigned int some_int = 0;
unsigned int some_reversed_integer;

void print_32_bits(unsigned int some_int){
  for (int i=0; i<32; i++){
    Serial.print(0b1 & (some_int >> (31-i)));
  }
}

void setup() {
  Serial.begin(1000000);
  delay(50);
  Serial.println();
  Serial.println(F("booted"));
  Serial.println(F("here we illustrate the rbit bit reversal instruction, made available through the CMSIS __RBIT() function"));
  Serial.println();
}

void loop() {
  some_reversed_integer = __RBIT(some_int);
  print_32_bits(some_int); Serial.print(F(" | ")); print_32_bits(some_reversed_integer); Serial.println();
  some_int++;
  delay(500);
}
