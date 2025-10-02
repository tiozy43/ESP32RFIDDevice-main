#ifndef CONFIG_H
#define CONFIG_H

#include <ArduinoJson.h>
#include <esp_task_wdt.h>

// ==================================================
// DEVICE Config Flags Name
// ==================================================

#define DEVICE_NAME "DEVNAM"                               ///< Name of the device
#define DEVICE_ID "DEVID"                                  ///< Unique identifier for the device
#define DEV_MASTR_CARD_ID "MSTID"                          ///< Master card identifier for the device
#define DEV_MANAGER_NAME "MANAM"                           ///< Name of the device manager

#define DEFAULT_DEVICE_NAME "RechargCredit"                ///< Default name assigned to the device
#define DEFAULT_DEVICE_ID "XXXXXXXXXXXXXXX"                ///< Default unique identifier for the device
#define DEFAULT_MASTR_CARD_ID "d3:73:fd:e3"                ///< Default master card identifier
#define DEFAULT_DEV_MANAGER_NAME "RAMECHK"                 ///< Default name for the device manager

#define APWIFIMODE_FLAG "STRAP"                            ///< Flag indicating if the device is in AP (Access Point) Wi-Fi mode
#define WIFISSID "WIFSSID"                                 ///< SSID (network name) for the Wi-Fi connection
#define WIFIPASS "WIFPASS"                                 ///< Password for the Wi-Fi connection
#define RESET_FLAG "RST"                                   ///< Flag to indicate a reset operation
#define BALANCE "BLC"                                      ///< Identifier for the balance value in device storage

#define LOGFILE_PATH "/log.json"
// ==================================================
// Configuration Constants
// ==================================================

// Default Wi-Fi Access Point (AP) Credentials
#define DEFAULT_AP_SSID "DeviceRF"                         ///< Default SSID for AP
#define DEFAULT_AP_PASSWORD "12345678"                     ///< Default password for AP

#define DEFAULT_WIFI_SSID "Techlancer"                     ///< Default SSID for Wi-Fi
#define DEFAULT_WIFI_PASSWORD "12345678"                   ///< Default password for Wi-Fi

#define DEBUGMODE 1                                        ///< Set to 1 to enable debug output, 0 to disable
#define SERIAL_BAUD_RATE 115200                            ///< Baud rate for serial communication
#define CONFIG_PARTITION "config"                          ///< Partition for configuration storage

#define LOCK_TIMEOUT 60000                                 ///< Lock timeout duration in milliseconds

// ==================================================
// AUTHENTICATION SECTORS & BLOCKS
// ==================================================

#define BALANCE_AUTH 39                                    ///< Authentication sector for balance 
#define NUM012_AUTH 43                                     ///< Authentication sector for numbers 01-02
#define NUM034_AUTH 47                                     ///< Authentication sector for numbers 03-04

#define BALANCE_SECBLOC 37                                 ///< Sector for balance storage (sector * 4 + block)
#define NUM01_SECBLOC 40                                   ///< Sector location for Number 01
#define NUM02_SECBLOC 41                                   ///< Sector location for Number 02
#define NUM03_SECBLOC 44                                   ///< Sector location for Number 03
#define NUM04_SECBLOC 45                                   ///< Sector location for Number 04

// ==================================================
// RFID RC522 Pin Configuration (using default SPI pins)
// ==================================================

#define RFID_RST_PIN 16                                    ///< Reset pin for RFID module
#define RFID_MISO_PIN 19                                   ///< MISO pin for RFID module
#define RFID_MOSI_PIN 23                                   ///< MOSI pin for RFID module
#define RFID_SCK_PIN 18                                    ///< SCK pin for RFID module
#define RFID_SDA_PIN 5                                     ///< SS/SDA pin for RFID module

// External declarations for authentication keys
extern byte defaultKey[6];    ///< Default key used for authentication in test environments
extern byte customKey[6];     ///< Secure key used for authentication in production environments

// Define aliases for authentication keys based on the environment (test or production)
#define TEST_KEY          defaultKey   ///< Alias for the default key used in testing
#define PRODUCTION_KEY    customKey    ///< Alias for the secure key used in production

// Define Key A and Key B for authentication
#define AUTH_KEY_A {0x02, 0x03, 0x01, 0x02, 0x01, 0x09}    ///< Key A for authentication
#define AUTH_KEY_B {0x02, 0x03, 0x01, 0x02, 0x01, 0x09}    ///< Key B for authentication

// Select the active authentication key based on the environment (change as needed)
#define ACTIVE_KEY        TEST_KEY     ///< Set to either TEST_KEY or PRODUCTION_KEY based on the environment

#define DEFAULT_BALANCE 2000  ///< Initial balance of the device

// ==================================================
// Keypad Pin Configuration for 4x3 Keypad
// ==================================================

// Row Pins for Keypad
#define KEYPAD_ROW_1_PIN  4   ///< Row 1 connected to GPIO 4
#define KEYPAD_ROW_2_PIN  26  ///< Row 2 connected to GPIO 26
#define KEYPAD_ROW_3_PIN  27  ///< Row 3 connected to GPIO 27
#define KEYPAD_ROW_4_PIN  14  ///< Row 4 connected to GPIO 14

// Column Pins for Keypad
#define KEYPAD_COL_1_PIN 32   ///< Column 1 connected to GPIO 32
#define KEYPAD_COL_2_PIN 33   ///< Column 2 connected to GPIO 33
#define KEYPAD_COL_3_PIN 25   ///< Column 3 connected to GPIO 25

// Define keypad dimensions
#define ROW_NUM     4       ///< Number of rows in the keypad
#define COLUMN_NUM  3       ///< Number of columns in the keypad

#define LAST_KEYPRESS_DELAY 400   ///< Delay after the last keypress (in milliseconds)
#define OUT_DELAY 1000            ///< Delay for output (in milliseconds)

// ==================================================
// LCD I2C Display Configuration
// ==================================================
#define SDA_PIN 21  ///< Custom SDA pin for I2C communication
#define SCL_PIN 22  ///< Custom SCL pin for I2C communication

// ==================================================
// LED Pin Configuration
// ==================================================
#define LED_GREEN_PIN 6   ///< LED pin for status indication (green)
#define LED_RED_PIN 9     ///< LED pin for status indication (red)

#define TIMEOFFSET 19800  ///< Time offset (seconds)
#define YEAROFFSET 1900   ///< Year offset
#define MOISOFFSET 1      ///< Month offset (January = 1)

// ==================================================
// BUZZER Pin Configuration
// ==================================================
#define BUZZ_PIN 12  ///< BUZZER pin for sound notifications

// ==================================================
// End of Configuration
// ==================================================


#endif // CONFIG_H
