// looking at RTC functions
// based on the materials from: https://os.mbed.com/docs/mbed-os/v6.6/apis/time-apis.html

#include "mbed.h"

void setup() {
  // put your setup code here, to run once:

}

int main()
{
    constexpr uint32_t watchdog_timeout_ms = 5000;
    
    rtos::ThisThread::sleep_for(1000);
    
    char char_buffer[128];  // for generating timestamp cstrings
    
    time_t posix_timestamp;  // a unix timestamp
    
    struct tm calendar_date_time;  // a struct representing calendar time
    /*  struct tm; in normal std C++ conventions this is:
      Member  Type  Meaning Range
      tm_sec  int seconds after the minute  0-61*
      tm_min  int minutes after the hour  0-59
      tm_hour int hours since midnightseconds  0-23
      tm_mday int day of the month  1-31
      tm_mon  int months since January  0-11
      tm_year int years since 1900  
      tm_wday int days since Sunday 0-6
      tm_yday int days since January 1  0-365
      tm_isdst  int Daylight Saving Time flag 
        HOWEVER, Mbed-OS has its own "conventions", the functions here are not from std C++ but from MbedOS:
      -- reference year is 1900
      -- months are counted from 0
      -- using uint8_t type
        these conventions must be used to avoid offset and failed conversions bugs
     */
    
    // example 1: set the RTC with a UNIX timestamp (seconds since 01.01.1970 00:00:00)
    // set_time(1256729737);  // Set RTC time to posix timestamp 1256729737, i.e. Wed, 28 Oct 2009 11:35:37

    // example 2: set the RTC with a tm struct in MbedOS conventions using a call to mktime
    
    // first, write the struct in MbedOS conventions
    calendar_date_time.tm_sec  = (uint8_t)5;  // 5secs
    calendar_date_time.tm_min  = (uint8_t)6;  // 6mins
    calendar_date_time.tm_hour = (uint8_t)18; // 18hrs
    calendar_date_time.tm_mday = (uint8_t)7;  // 7th month day
    calendar_date_time.tm_mon  = (uint8_t)(8-1);  // 8th month, August; need to offset as MbeOS months start at 0
    calendar_date_time.tm_year = (uint8_t)(2006-1900); // year; need to offset, MbedOS years start at 1900

    /*  time_t mktime (struct tm * timeptr);
    This function performs the reverse translation that localtime does.
    The values of the members tm_wday and tm_yday of timeptr are ignored,
    and the values of the other members are interpreted even if out of their
    valid ranges (see struct tm). For example, tm_mday may contain values above 31,
    which are interpreted accordingly as the days that follow the last day
    of the selected month.
    
    A call to this function automatically adjusts the values of the members
    of timeptr if they are off-range or -in the case of tm_wday and tm_yday- if
    they have values that do not match the date described by the other members.
     */

    // second, generate posiz timestamp
    posix_timestamp = mktime(&calendar_date_time);
    Serial.begin(57600);
    Serial.print(F("start posix timestamp using mktime (expected: 1154973965): "));
    Serial.println((unsigned int)posix_timestamp);
    Serial.end();

    // third, set the RTC time
    // here, let the user choose to set it or not, to allow some test whether RTC survives reseting
    Serial.begin(57600);
    Serial.println(F("set the timestamp? [y/n]"));
    while (true){
      if (Serial.available() > 0){
        if (Serial.read() == 'y'){
            Serial.println(F("Set!"));
            set_time(posix_timestamp);
            break;
        }
        else{
          Serial.println(F("Do not set!"));
          break;
        }
      }
    }
    Serial.end();
     
    while (true) {
        // get the current utc time from the rtc
        
        // note about time() function:
        // You can pass in a pointer to a time_t object that time will fill up with the current time
        // (and the return value is the same one that you pointed to). If you pass in NULL, it just
        // ignores it and merely returns a new time_t object that represents the current time.
        // posix_timestamp = time(nullptr);  // use 1: now posix_timestamp is the crrt posix timestamp
        time(&posix_timestamp);  // another way to do the same thing: posix_timestamp is now the crrt posix timestamp from the RTC

        // convert into a calendar date time
        calendar_date_time = *localtime(&posix_timestamp);  // compute the calendar time from posix timestamp

        // convert into a formatted string
        strftime(char_buffer, sizeof(char_buffer), "%Y-%m-%dT%H:%M:%S%z", &calendar_date_time);

        Serial.begin(57600);
        Serial.print(F("Time as seconds since January 1, 1970: "));
        Serial.println((unsigned int)posix_timestamp);
        
        Serial.print(F("Time as a basic string: "));
        Serial.println(char_buffer);  // ctime() converts time since epoch to char representation
        
        Serial.end();

        rtos::ThisThread::sleep_for(1000);

        // different ways to restart the system; the RTC should survive system restart as long as power is not lost and no full reset is used

        // watchdog reset
        while (true){
          // Watchdog::get_instance().kick();  // uncomment to reset watchdog; this would be an endless loop
        }
        
        // RTC does not survive system reset!
        NVIC_SystemReset();
    }
}
