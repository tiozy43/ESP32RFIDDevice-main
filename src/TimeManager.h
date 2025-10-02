#ifndef TIMEMANAGER_H
#define TIMEMANAGER_H

#include <NTPClient.h>
#include <WiFiUdp.h>
#include <WiFi.h>
#include "Config.h"

class TimeManager {
public:
    TimeManager(const char* ntpServer = "pool.ntp.org", long timeOffset = TIMEOFFSET, unsigned long updateInterval = 60000);
    
    void initialize();          // Initialize NTP client
    String getTimeString();     // Returns current time as "HH:MM"
    String getPreviousMinuteTimeString();
    String getDateString();     // Returns current date as "DD MON YYYY"
    String getPreviousDateString();
    void updateTime();          // Update NTP time
    String getMonthText(int month);
    

private:
    WiFiUDP ntpUDP;
    NTPClient timeClient;
};

#endif  // TIMEMANAGER_H
