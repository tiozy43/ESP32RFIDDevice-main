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
