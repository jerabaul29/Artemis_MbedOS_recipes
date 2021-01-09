// inspired from https://os.mbed.com/docs/mbed-os/v6.6/apis/watchdog.html

#include "mbed.h"

const uint32_t TIMEOUT_MS = 5000;

void setup() {
  // put your setup code here, to run once:

}

int main()
{
    Serial.begin(57600);
    Serial.println(F("Start"));
    Serial.end();
    
    Watchdog &watchdog = Watchdog::get_instance();
    watchdog.start(TIMEOUT_MS);

    while (true) {
      Serial.begin(57600);

        if (true){
          Serial.print(F("reset watchdog"));
          Watchdog::get_instance().kick();
        }
        else{
          Serial.println(F("do not reset watchdog; will bite in a few iterations..."));
        }
        
        ThisThread::sleep_for(TIMEOUT_MS / 10);
        
      Serial.end();
    }
}
