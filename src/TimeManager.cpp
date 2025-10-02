#include "TimeManager.h"

// Helper function to convert month number to text
String TimeManager ::getMonthText(int month) {
    const char* months[] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN", 
                            "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};
    return months[month - 1]; // month is 1-based
}
/**
 * @brief Constructor for the TimeManager class.
 * @param ntpServer NTP server address (default: "pool.ntp.org").
 * @param timeOffset Time offset from UTC in seconds (default: 0).
 * @param updateInterval Interval in milliseconds to update time (default: 60000).
 */
TimeManager::TimeManager(const char* ntpServer, long timeOffset, unsigned long updateInterval)
    : timeClient(ntpUDP, ntpServer, timeOffset, updateInterval) {}

/**
 * @brief Initializes the NTP client.
 */
void TimeManager::initialize() {
    timeClient.begin();
    timeClient.update();
}

/**
 * @brief Updates the NTP time by syncing with the server.
 */
void TimeManager::updateTime() {
    if (WiFi.status() == WL_CONNECTED) {
        timeClient.update();
    }
}


/**
 * @brief Gets the current time in "HH:MM" format.
 * @return String representing the current time.
 */
String TimeManager::getTimeString() {
    if (WiFi.status() == WL_CONNECTED) {
        timeClient.update(); // Sync time

        // Get the epoch time
        unsigned long epochTime = timeClient.getEpochTime();

        // Convert epoch time to tm structure
        time_t rawtime = epochTime;  // Cast epoch time to time_t
        struct tm *timeinfo = localtime(&rawtime); // Convert to local time

        // Extract time components
        int hours = timeinfo->tm_hour; // Hours in 24-hour format
        int minutes = timeinfo->tm_min; // Minutes

        // Format time string
        String timeStr = (hours < 10 ? "0" : "") + String(hours) + ":" + (minutes < 10 ? "0" : "") + String(minutes);
        return timeStr;
    }
    return "No WiFi";
}
/**
 * @brief Gets the previous time (current time minus 1 minute) in "HH:MM" format.
 * @return String representing the time one minute before the current time.
 */
String TimeManager::getPreviousMinuteTimeString() {
    if (WiFi.status() == WL_CONNECTED) {
        timeClient.update(); // Sync time

        // Get the epoch time
        unsigned long epochTime = timeClient.getEpochTime();

        // Convert epoch time to tm structure
        time_t rawtime = epochTime;  // Cast epoch time to time_t
        struct tm *timeinfo = localtime(&rawtime); // Convert to local time

        // Subtract one minute
        timeinfo->tm_min -= 1;

        // Normalize the time structure (handle underflow, such as crossing to a new hour or day)
        mktime(timeinfo);

        // Extract the new time components
        int hours = timeinfo->tm_hour; // Hours in 24-hour format
        int minutes = timeinfo->tm_min; // Minutes

        // Format the previous time string
        String previousTimeStr = (hours < 10 ? "0" : "") + String(hours) + ":" + (minutes < 10 ? "0" : "") + String(minutes);
        return previousTimeStr;
    }
    return "No WiFi";
}
/**
 * @brief Gets the current date in "DD MON YYYY" format.
 * @return String representing the current date.
 */
String TimeManager::getDateString() {
    if (WiFi.status() == WL_CONNECTED) {
        timeClient.update(); // Sync time

        // Get the epoch time
        unsigned long epochTime = timeClient.getEpochTime();

        // Convert epoch time to tm structure
        time_t rawtime = epochTime;  // Cast epoch time to time_t
        struct tm *timeinfo = localtime(&rawtime); // Convert to local time

        // Extract date components
        int day = timeinfo->tm_mday;    // Day of the month
        int month = timeinfo->tm_mon + MOISOFFSET; // Month (0-based, add 1)
        int year = timeinfo->tm_year + YEAROFFSET; // Year (years since 1900)

        // Format date string
        String dateStr = String(day) + " " + getMonthText(month) + " " + String(year);
        return dateStr;
    }
    return "No WiFi";
}
/**
 * @brief Gets the previous date (current date minus 1 day) in "DD MON YYYY" format.
 * @return String representing the date one day before the current date.
 */
String TimeManager::getPreviousDateString() {
    if (WiFi.status() == WL_CONNECTED) {
        timeClient.update(); // Sync time

        // Get the epoch time
        unsigned long epochTime = timeClient.getEpochTime();

        // Convert epoch time to tm structure
        time_t rawtime = epochTime;  // Cast epoch time to time_t
        struct tm *timeinfo = localtime(&rawtime); // Convert to local time

        // Subtract one day
        timeinfo->tm_mday -= 1;

        // Normalize the time structure (handle underflow, such as crossing to a new month or year)
        mktime(timeinfo);

        // Extract the new date components
        int day = timeinfo->tm_mday;    // Day of the month
        int month = timeinfo->tm_mon + MOISOFFSET; // Month (0-based, add 1)
        int year = timeinfo->tm_year + YEAROFFSET; // Year (years since 1900)

        // Format the previous date string
        String previousDateStr = String(day) + " " + getMonthText(month) + " " + String(year);
        return previousDateStr;
    }
    return "No WiFi";
}