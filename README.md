📘 README (English version for GitHub)
💳 ESP32 Smart Card System with RFID, Wi-Fi & LCD
📌 Overview

This project is an embedded system for ESP32 integrating RFID (MFRC522), Wi-Fi, LCD (I2C), keypad, and buzzer.
It allows secure smart card operations including balance management, recharge, logging, and card locking.
The system provides a user interface via LCD + keypad, and supports both Wi-Fi client and Access Point (AP) modes for configuration.

✨ Features

✅ RFID card management (MFRC522) → read/write UID, balance, stored numbers.

✅ Card recharge & locking with authentication (Key A/Key B).

✅ User vs Master card modes with extended privileges for master.

✅ LCD 20x4 display + Keypad input for menus, recharges, confirmations.

✅ Buzzer feedback for success/failure events.

✅ Wi-Fi support:

Client mode (connect to router).

Access Point mode (web interface for setup & GPIO control).

✅ Logs stored in SPIFFS (JSON/text) with timestamp via NTP.


├── main.cpp              → Application entry point (system init, logic)
├── MRC522Manager.*       → RFID card operations (UID, balance, lock, numbers)
├── ScreenManager.*       → LCD + keypad UI, menus, recharge, home page
├── LogManager.*          → Logging system (SPIFFS + NTP timestamps)
├── WiFiManager.*         → Wi-Fi client/AP, web server for configuration
├── TimeManager.*         → NTP time synchronization
├── ConfigManager.*       → Persistent configuration storage
├── BuzzerManager.*       → Audio feedback (success/error tones)
⚙️ Hardware Requirements

ESP32 board.

MFRC522 RFID reader.

20x4 I2C LCD.

4x3 keypad.

Buzzer (GPIO controlled).

Access to Wi-Fi or standalone (AP mode).

🚀 Getting Started
🔧 Compilation

Open project in Arduino IDE / PlatformIO.

Install required libraries:

MFRC522, ESPAsyncWebServer, LiquidCrystal_I2C, Keypad, NTPClient, SPIFFS.

Flash firmware to ESP32.

▶️ Usage

Master Card → access advanced features (lock cards, store numbers).

User Card → recharge balance, standard operations.

Logs are saved in SPIFFS with timestamps.

Wi-Fi page available in AP mode for setup/config.

📊 Example Workflow

Power the ESP32 → LCD shows home screen with Wi-Fi signal & time.

Present an RFID card:

Master card → unlocks admin functions (store/retrieve numbers, lock cards).

User card → displays balance, recharge option.

Use keypad to confirm recharge, enter numbers, or navigate menus.

Logs stored in SPIFFS (can be retrieved via web server or serial).

📜 License

This project uses ESP32 Arduino libraries and is provided AS-IS without warranty.
