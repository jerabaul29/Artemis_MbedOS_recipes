// with this line, do not even get the Serial output
//UART iridiumSerial(24, 25);

void setup(){
  Serial.begin(115200);
  delay(10);
  Serial.println(F("booted"));
  Serial.end();

  Serial1.begin(115200);
  delay(10);
  Serial1.end();

  Serial.begin(115200);
  delay(10);
  Serial.println(F("done with serial1"));
}

void loop(){
  
}
