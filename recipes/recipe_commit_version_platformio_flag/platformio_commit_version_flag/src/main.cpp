#include <Arduino.h>

#define STRINGIFY2(X) #X
#define STRINGIFY(X) STRINGIFY2(X)

void setup() {
  Serial.begin(9600);
  Serial.println(F("booted"));
  delay(100);

  Serial.print(F("compiled at UNIX time: "));
  Serial.println(F(STRINGIFY(BUILD_TIMESTAMP)));
  delay(100);

  Serial.print(F("built from commit: "));
  Serial.println(F(STRINGIFY(BUILD_COMMIT)));
  delay(100);

  Serial.println(F("done setup"));
}

void loop() {
  // put your main code here, to run repeatedly:
}