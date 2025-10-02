#include <Arduino.h>
#include "MRC522Manager.h"
#include "ScreenManager.h"
#include "WiFiManager.h"
#include <Wire.h>

// Preferences object to store non-volatile data
Preferences prefs;
LiquidCrystal_I2C LCD(0x27, 20, 4);

// Manager instances for handling various functionalities
ConfigManager* configManager = nullptr;
MRC522Manager* rfidManager = nullptr;
ScreenManager* screenManager = nullptr;
WiFiManager* wifiManager = nullptr;
TimeManager* timeManager = nullptr;
LogManager* Log = nullptr;
MFRC522 rfid(RFID_SDA_PIN, RFID_RST_PIN);
BuzzerManager* Buzz = nullptr;

bool startAP = false;  // Flag to determine if Access Point (AP) page should be shown

// Define constants
const unsigned long HOME_PAGE_INTERVAL = 60000;  // 60 seconds
unsigned long previousMillis = 0;  // Store last time homepage was displayed

/**
 * @brief Initializes the Arduino environment and application managers.
 * 
 * This setup function initializes various managers responsible for different
 * functionalities such as RFID, keypad, Wi-Fi, screen display, and time-related
 * operations. The function configures serial communication, instantiates manager
 * objects, and initializes them. The setup function also determines whether the
 * Access Point (AP) page or the homepage should be displayed.
 */
void setup() {
    // Initialize serial communication for debugging and logging
    Serial.begin(115200);

    // Initialize I2C with custom SDA and SCL pins
    Wire.begin(SDA_PIN, SCL_PIN);

    // Open Preferences in read-write mode
    prefs.begin(CONFIG_PARTITION, false);  

    // Instantiate and initialize configuration manager
    configManager = new ConfigManager(&prefs);  // Manage device configurations
    configManager->begin();

    // Retrieve AP flag setting
    startAP = configManager->GetAPFLag();      

    // Initialize Wi-Fi manager and start Wi-Fi connection process
    wifiManager = new WiFiManager(configManager);  
    wifiManager->begin();  // Start Wi-Fi manager

    // Initialize time manager for handling time-related functions
    timeManager = new TimeManager(); 
    timeManager->initialize();
    Serial.println(timeManager->getTimeString());
    Serial.println(timeManager->getDateString());

    // Initialize Log manager for handling log-related functions
    Log = new LogManager();
    Log->begin();

    // Initialize buzzer manager for handling beep sounds
    Buzz = new BuzzerManager(BUZZ_PIN);
    Buzz->begin();

    // Initialize RFID manager for handling RFID card operations
    SPI.begin(RFID_SCK_PIN, RFID_MISO_PIN, RFID_MOSI_PIN);  // Initialize SPI
    rfid.PCD_Init();  // Initialize MFRC522
    rfidManager = new MRC522Manager(configManager, &rfid); 
    rfidManager->begin();  // Start RFID manager

    // Initialize screen manager to manage display screens and UI navigation
    screenManager = new ScreenManager(wifiManager, Log, rfidManager, &LCD, Buzz); 
    screenManager->begin();
    
    // Display AP page if the startAP flag is set
    if (startAP) {
       // screenManager->GotoApPage();
    }
}

/**
 * @brief Main loop that processes user input and navigates based on conditions.
 * 
 * This loop constantly checks the status of the Wi-Fi connection and displays
 * the homepage or handles security check processes. If an RFID card is detected,
 * the system checks if it's a master or regular card, and navigates to the appropriate
 * page (e.g., master mode or recharge page). If the Wi-Fi is disconnected, the loop
 * will return to the Access Point (AP) page.
 */
void loop() {
    lock:
    // Perform initial security check on the screen if Wi-Fi is still connected
    if (wifiManager->isStillConnected()) {
        screenManager->SecurityCheck();
    }

    // Start of the loop
    Start:
    screenManager->clearScreen();  // Clear screen before the new process
    unsigned long startTime = millis(); // Record the start time for timeout management

    while (wifiManager->isStillConnected()) {
        delay(10);  // Small delay to avoid overwhelming the system

        // Check if the loop has been running for more than 1 minute (timeout)
        if (millis() - startTime > LOCK_TIMEOUT) {
            screenManager->clearScreen();
            screenManager->displayCenteredText("Timeout", 1);
            screenManager->displayCenteredText("Locking the device", 2);
            screenManager->displayCenteredText("...", 3);
            delay(2000);
            screenManager->clearScreen();
            goto lock; // Exit the loop after timeout
        }

        // Check if the RFID card is master card
        rfidManager->IsMasterCard();

        if (rfidManager->cardStatusRead == 6) {
            // Authentication failed, invalid card
            Buzz->playFailureTone();
            screenManager->clearScreen();
            screenManager->displayCenteredText("SECURITY CHECK", 0);
            screenManager->displayCenteredText(">   INVALID CARD!  <", 2);
            screenManager->displayCenteredText("____________________", 3);
            delay(3000);
            goto Start;  // Restart the process
        }

        if (rfidManager->cardStatusRead == 0) goto in;

        // Display the homepage with device manager name
        screenManager->HomePage(DEFAULT_DEV_MANAGER_NAME);

        in:
        if (rfidManager->cardStatusRead == 0) {
            screenManager->clearScreen();
            char choice = screenManager->SelectAction();
            Serial.println("The User choice is :");
            Serial.print(choice);
            Serial.println();

            if (choice == '1') {
                // Recharge page if choice is '1'
                screenManager->clearScreen();
                screenManager->RechargePage();
                rfidManager->resetRFID();
                goto Start;  // Restart the process
            } else if (choice == '2') {
                // User mode if choice is '2'
                screenManager->clearScreen();
                screenManager->UserMode();
                rfidManager->resetRFID();
                goto Start;  // Restart the process
            } else if (choice == '*') {
                // Reset RFID and return to the start if '*' is pressed
                rfidManager->resetRFID();
                goto Start;  // Restart the process
            }
        }
    }

    // If Wi-Fi is disconnected, navigate back to the AP page
    //screenManager->GotoApPage();
    goto Start;  // Go back to the start label or handle AP mode in your logic
}
