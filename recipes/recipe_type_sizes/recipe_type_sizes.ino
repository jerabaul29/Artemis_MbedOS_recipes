#include "Arduino.h"

void setup(){
    Serial.begin(1000000);
    delay(100);
    Serial.println(F("booted"));

    Serial.print(F("sizeof float: ")); Serial.println(sizeof(float));
}

void loop(){
    
}

