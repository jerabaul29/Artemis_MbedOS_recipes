struct ExampleAlignment{
  float data_f;
  byte  data_b;
  unsigned long data_l;
};

void setup() {
  Serial.begin(1000000);
  delay(10);
  Serial.println(F("booted"));

  ExampleAlignment my_struct{3.45f, 'A', 1 + 2*256 + 3*256*256 + 4*256*256*256};

  char * ptr_to_struct = (char *)&my_struct;

  for (size_t ind=0; ind<sizeof(ExampleAlignment); ind++){
    Serial.println(*(ptr_to_struct + ind), HEX);
  }

  Serial.println();
}

void loop() {
  // put your main code here, to run repeatedly:

}
