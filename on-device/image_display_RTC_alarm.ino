/*
   Based on Inkplate6COLOR_RTC_Alarm_With_Deep_Sleep example for Soldered Inkplate 6COLOR
*/

#ifndef ARDUINO_INKPLATECOLOR
#error "Wrong board selection for this example, please select Soldered Inkplate 6COLOR in the boards menu."
#endif

#include "HTTPClient.h" //Include library for HTTPClient
#include "WiFi.h"       //Include library for WiFi
#include "Inkplate.h"      // Include Inkplate library to the sketch
#include "driver/rtc_io.h" // Include ESP32 library for RTC pin I/O (needed for rtc_gpio_isolate() function)
#include <rom/rtc.h>       // Include ESP32 library for RTC (needed for rtc_get_reset_reason() function)

Inkplate display; // Create an object on Inkplate library

const char *image_url = ""; // URL to get image
const char ssid[] = ""; // Your WiFi SSID
const char *password = ""; // Your WiFi password

const uint16_t WIFI_RETRY_DELAY = 500; // ms
const uint16_t WIFI_MAX_RETRIES = 50;

// Define a struct to represent an AlarmTime
struct AlarmTime {
  uint8_t hour;
  uint8_t min;
};
constexpr std::array<AlarmTime, 4> alarmList = {{
    {9, 15},
    {16, 00},
    {7, 00},
    {7, 30}
  }};

RTC_DATA_ATTR int alarm_idx = 0;

void setup()
{
    Serial.begin(115200); // for debugging
    display.begin(); 
    int16_t retries, current_mod, next_alarm_mod, mins_until_next_alarm = 0; // needs to be initialized before goto

    // Check if RTC is already is set. If not, set time and date
    if (!display.rtcIsSet()) 
    {
        display.rtcSetTime(21, 9, 00); // 24H mode, ex. 13:30:00
        display.rtcSetDate(0, 28, 10, 2024); // 0 for Monday.
        // also sets epoch, despite a bit strange value 
        goto go_to_sleep;       
    }

    // Clear alarms and get time data from RTC
    display.rtcGetRtcData(); 
    display.rtcClearAlarmFlag();
    
    // Ugly workaround around day borders / inability to use rtcSetAlarmEpoch: hope the RTC code gets "next day" right
    if (display.rtcGetHour() == 23 and display.rtcGetMinute() == 59) {
      delay(61*1000);
      display.rtcGetRtcData(); 
      display.rtcSetAlarm(0, alarmList[alarm_idx].min, alarmList[alarm_idx].hour, display.rtcGetDay(), display.rtcGetWeekday());
      goto go_to_sleep_without_display; 
    }
    
    // Advance to the next alarm
    Serial.println("Setting new alarm");    
    alarm_idx++; 
    if (alarm_idx >= alarmList.size())
        alarm_idx = 0;

    // Compute minute of day ("mod")    
    current_mod = 60 * display.rtcGetHour() + display.rtcGetMinute();    
    next_alarm_mod = 60 * alarmList[alarm_idx].hour + alarmList[alarm_idx].min;
    mins_until_next_alarm = next_alarm_mod - current_mod;            

    // Set next alarm using time or epoch, depending on whether its on the same day    
    if (current_mod < next_alarm_mod) {
      display.rtcSetAlarm(0, alarmList[alarm_idx].min, alarmList[alarm_idx].hour, display.rtcGetDay(), display.rtcGetWeekday());
    } else {
      //display.rtcSetAlarmEpoch(display.rtcGetEpoch() + 60 * mins_until_next_alarm, RTC_ALARM_MATCH_DHHMMSS);
      display.rtcSetAlarm(1, 59, 23, display.rtcGetDay(), display.rtcGetWeekday());
    };

    // Connect to the WiFi network (the more elaborate "display.connectWiFi()" )
    WiFi.mode(WIFI_MODE_STA);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED && retries++ < WIFI_MAX_RETRIES)
    {
        delay(WIFI_RETRY_DELAY);
        Serial.print(".");
    }
    if (WiFi.status() != WL_CONNECTED) {
        display.setCursor(50, 75);
        display.println("WIFI ERROR");
        goto go_to_sleep;
    }
    Serial.println("\nWiFi OK! Downloading...");

    // Fetch and display the image
    if (!display.drawImage(image_url, display.PNG, 0, 0, true, false))
    { 
      delay(2000); 
      Serial.println("Starting 2nd try");
      if (!display.drawImage(image_url, display.PNG, 0, 0, true, false))
      { 
        display.setCursor(350, 50);
        display.println("Image open error");
        Serial.println("Error opening image");
      }
    }
    Serial.println("DONE");

go_to_sleep:    
    WiFi.mode(WIFI_OFF);
    printCurrentTime(); // Display current time and date
    display.display();

go_to_sleep_without_display:
    // Enable wakup from deep sleep on gpio 39 where RTC interrupt is connected
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_39, 0);

    // Start deep sleep (this function does not return). Program stops here.
    esp_deep_sleep_start();
}

void loop()
{
    // Never here! If you are using deep sleep, the whole program should be in setup() because the board restarts each
    // time. loop() must be empty!
}

constexpr std::array<char*, 7> weekday = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };
void printCurrentTime()
{
    display.setCursor(450, 5);
    display.setTextSize(1);
    display.setTextColor(INKPLATE_BLUE, INKPLATE_WHITE); // Set text color and background

    display.print(weekday[display.rtcGetWeekday()]);
    display.print(", ");
    display.print(display.rtcGetDay());
    display.print(".");
    display.print(display.rtcGetMonth());
    display.print(".");
    display.print(display.rtcGetYear());
    display.print(". ");
    print2Digits(display.rtcGetHour());
    display.print(':');
    print2Digits(display.rtcGetMinute());
    display.print(':');
    print2Digits(display.rtcGetSecond());
}

void print2Digits(uint8_t _d)
{
    if (_d < 10)
        display.print('0');
    display.print(_d, DEC);
}
