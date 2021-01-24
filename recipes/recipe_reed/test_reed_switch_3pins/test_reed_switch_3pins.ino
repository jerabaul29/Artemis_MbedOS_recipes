#include "mbed.h"
#include <ScopedLock.h>

rtos::Thread thread_read_digital;
rtos::Thread thread_blink;

constexpr int PIN_REED_1 = 2;
constexpr int PIN_REED_2 = 3;

volatile bool reed_1_on {false};
volatile bool reed_2_on {false};

rtos::Mutex use_reed_values;
rtos::Mutex use_serial;

enum class reed_status{
  valid_1,
  valid_2,
  invalid,
};

int nbr_blink {0};

reed_status crrt_status;

void read_digital(){
  while(true){
    {
      mbed::ScopedLock<rtos::Mutex> lock_reed{use_reed_values};
    
      reed_1_on = digitalRead(PIN_REED_1);
      reed_2_on = digitalRead(PIN_REED_2);
    }
  
    {
      mbed::ScopedLock<rtos::Mutex> lock_serial{use_serial};
  
      Serial.print(F("read reed: "));
      Serial.print(reed_1_on);
      Serial.print(F(" | "));
      Serial.print(reed_2_on);
      Serial.println();
    }
  
    rtos::ThisThread::sleep_for(std::chrono::milliseconds(500));
  }
}

void blink_led(){
  while(true){
   {
    mbed::ScopedLock<rtos::Mutex> lock_reed{use_reed_values};
  
    if (reed_1_on && !reed_2_on){
      crrt_status = reed_status::valid_1;  
      nbr_blink = 1;
    }
    else if (reed_2_on && !reed_1_on){
      crrt_status = reed_status::valid_2;  
      nbr_blink = 2;
    }
    else{
      crrt_status = reed_status::invalid;  
      nbr_blink = 3;
    }
   }
  
    for (int i=0; i<nbr_blink; i++){
      digitalWrite(LED_BUILTIN, HIGH);
      rtos::ThisThread::sleep_for(std::chrono::milliseconds(50));
      digitalWrite(LED_BUILTIN, LOW);
      rtos::ThisThread::sleep_for(std::chrono::milliseconds(50));
    }
   
   rtos::ThisThread::sleep_for(std::chrono::milliseconds(500));
  }
}

void setup() {
  delay(500);
  Serial.begin(115200);
  delay(200);
  Serial.println(F("booted"));
  
  pinMode(PIN_REED_1, INPUT);
  pinMode(PIN_REED_2, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  delay(100);

  thread_read_digital.start(read_digital);
  thread_blink.start(blink_led);
}

void loop() {
  // put your main code here, to run repeatedly:

}
