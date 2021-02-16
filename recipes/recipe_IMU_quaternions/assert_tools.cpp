#include "assert_tools.h"

void uassert(bool condition, const char * msg){
    if (!condition){
        Serial.println(msg);
        Serial.flush();
        delay(1000);
        abort();
    }
}
