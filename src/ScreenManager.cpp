#include "ScreenManager.h"


// Keypad layout with alphanumeric keys (rows and columns)
const char keys[ROW_NUM][COLUMN_NUM] = {
    {'1', '2', '3'},  // 1: ABC, 2: DEF, 3: GHI
    {'4', '5', '6'},  // 4: JKL, 5: MNO, 6: PQR
    {'7', '8', '9'},  // 7: STU, 8: VWX, 9: YZ
    {'*', '0', '#'}   // *: special, 0: space, #: special
};

// Row and column pin configuration for keypad
byte rowPins[ROW_NUM] = {KEYPAD_ROW_1_PIN, KEYPAD_ROW_2_PIN, KEYPAD_ROW_3_PIN, KEYPAD_ROW_4_PIN};
byte colPins[COLUMN_NUM] = {KEYPAD_COL_1_PIN, KEYPAD_COL_2_PIN, KEYPAD_COL_3_PIN};

// Initialize custom keypad
Keypad keypadd = Keypad(makeKeymap(keys), rowPins, colPins, ROW_NUM, COLUMN_NUM);


// Constructor
ScreenManager::ScreenManager(WiFiManager* wiFiManager,
LogManager* Log,MRC522Manager* mRC522Manager, LiquidCrystal_I2C* LCD,BuzzerManager* Buzz) :
wiFiManager(wiFiManager),
Log(Log),mRC522Manager(mRC522Manager) ,
LCD(LCD),Buzz(Buzz){}


void ScreenManager::begin() {
	LCD->init();
    LCD->backlight();
}	


void ScreenManager::clearScreen() {
    LCD->clear();
}

void ScreenManager::displayText(const char* text, int x, int y) {
	   LCD->setCursor(x, y);
	   LCD->print(text);
}

void ScreenManager::displayCenteredText(const char* text, int y) {
    // Ensure the line index is within bounds (0 to 3 for a 20x4 LCD)
    if (y < 0 || y > 3) {
        return; // Do nothing if y is out of bounds
    }

    // Calculate the starting x position to center the text
    int textLength = strlen(text); // Get the length of the text
    int x = (20 - textLength) / 2; // Calculate the centered x position

    // Ensure x is non-negative
    if (x < 0) {
        x = 0; // If text is too long to fit, start at the first column
    }

    // Set the cursor position and print the text
    LCD->setCursor(x, y);
    LCD->print(text);
}


/**
 * @brief Displays a blinking success message on the LCD screen.
 * 
 * @param message The success message to display.
 * @param blinkCount The number of times the message should blink.
 * @param row The row (0-3) on which to display the message.
 */
void ScreenManager::displaySuccessMessage(const char* message, int blinkCount, int row) {
    // Ensure the row index is within bounds (0 to 3 for a 20x4 LCD)
    if (row < 0 || row > 3) {
        return; // Do nothing if the row is out of bounds
    }

    for (int i = 0; i < blinkCount; i++) {
        // Display the message
        LCD->clear();
        int textLength = strlen(message);
        int x = (20 - textLength) / 2; // Center the message horizontally
        if (x < 0) x = 0;             // Handle overflow if the message is too long
        LCD->setCursor(x, row);
        LCD->print(message);

        // Wait before clearing
        delay(200);

        // Clear the screen
        LCD->clear();
        delay(200);
    }
}
/**
 * @brief Displays a blinking failure message on the LCD screen.
 * 
 * @param message The failure message to display.
 * @param blinkCount The number of times the message should blink.
 * @param row The row (0-3) on which to display the message.
 */
void ScreenManager::displayFailureMessage(const char* message, int blinkCount, int row) {
    // Ensure the row index is within bounds (0 to 3 for a 20x4 LCD)
    if (row < 0 || row > 3) {
        return; // Do nothing if the row is out of bounds
    }

    for (int i = 0; i < blinkCount; i++) {
        // Display the message (simulating "red" with the text being present)
        LCD->clear();
        int textLength = strlen(message);
        int x = (20 - textLength) / 2; // Center the message horizontally
        if (x < 0) x = 0;             // Handle overflow if the message is too long
        LCD->setCursor(x, row);
        LCD->print(message);

        // Wait before clearing
        delay(200);

        // Clear the screen to simulate "blink effect"
        LCD->clear();
        delay(200);
    }
}


/**
 * @brief Erases a character at a specified (x, y) cursor position on an I2C LCD by overwriting it with a space.
 *
 * @param x The x-coordinate (column) where the character will be erased (0-19 for a 20x4 LCD).
 * @param y The y-coordinate (row) where the character will be erased (0-3 for a 20x4 LCD).
 */
void ScreenManager::eraseCharacter(int x, int y) {
    // Ensure the coordinates are within bounds
    if (x < 0 || x > 19 || y < 0 || y > 3) {
        return; // Do nothing if out of bounds
    }
    // Set cursor to the specified position
    LCD->setCursor(x, y);

    // Print a blank space to "erase" the character
    LCD->print(' ');
}


/**
 * @brief Displays a prompt on the LCD and captures numeric input from the keypad.
 *
 * This function displays a numeric input prompt on the LCD, allowing the user to:
 * - Enter numbers (0–9) up to a maximum of 10 characters.
 * - Delete the last character using '*'.
 * - Confirm input using '#'.
 *
 * @param promptText The prompt message to display.
 * @param currentNum The initial number to display as a reference or default.
 * @return A `String` containing the user's confirmed numeric input, or an empty string if canceled.
 */
String ScreenManager::ShowPrompt(const char* promptText, const char* currentNum) {
    String userInput = ""; // Stores the user's input
    const int maxLength = 10; // Maximum input length
    int cursorPos = strlen(currentNum); // Start cursor after the currentNum length

    // Clear the LCD and display the prompt message
    LCD->clear();
    LCD->setCursor(0, 0);
    LCD->print(promptText);

    LCD->setCursor(0, 2);
    LCD->print(">");
    // Display the current number on the second row
    LCD->setCursor(5, 3);
    LCD->print(currentNum);

    while (true) {
        // Get a key press from the keypad
        char key = keypadd.getKey(); // Get the character from the key press

        if (key != NO_KEY) { // If a key is pressed
            if (key >= '0' && key <= '9') { // Numeric keys
                if (userInput.length() < maxLength) { // Check length limit
                    userInput += key; // Add key to the input
                    LCD->setCursor(2 + userInput.length(), 2);
                    LCD->print(key); // Display the key
                }
            } else if (key == '*') { // Delete the last character
                if (!userInput.isEmpty()) {
                    LCD->setCursor(2 + userInput.length(), 2);
                    LCD->print(" "); // Erase the character from the display
                    userInput.remove(userInput.length() - 1); // Remove the last character
                }
            } else if (key == '#') { // Confirm input
                return userInput; // Return the collected input
            }
        }

        delay(50); // Short delay to debounce key presses
    }
}



/**
 * @brief Displays a confirmation prompt on the LCD and waits for user input.
 *
 * This function displays a prompt asking the user to confirm (`#`) or cancel (`*`).
 * @param Text The message to display.
 * @return `true` if confirmed, `false` if canceled.
 */
bool ScreenManager::confirmInput(const char* Text) {
    // Clear the LCD screen and display the confirmation prompt
    LCD->clear();
    LCD->setCursor(0, 0);
    LCD->print("  Save the entry?");

    // Display the detailed prompt message
    LCD->setCursor(0, 1);
    LCD->print(">     ");
    LCD->print(Text);
    LCD->setCursor(19, 1);
    LCD->print("<");

    // Display the options at the bottom of the screen
    LCD->setCursor(0, 3);
    LCD->print("No: '*'     Yes: '#'");

    // Enter a loop to wait for user input and handle response
    while (true) {
        // Retrieve key press input from the keypad
        char key = keypadd.getKey();

        // Return true if the user presses '#', confirming the save action
        if (key == '#') {
            Buzz->playSuccessTone();
            return true;
        }

        // Return false if the user presses '*', declining the save action
        if (key == '*') {
            Buzz->playFailureTone();
            return false;
        }

        // Short delay to reduce CPU load from rapid polling
        delay(50);
    }
}

/**
 * @brief Displays a confirmation prompt for recharging a specified amount and waits for user input.
 *
 * This function displays a message on the LCD asking the user to confirm recharging the specified amount.
 * The user can press `'#'` to confirm or `'*'` to cancel. It returns `true` for confirmation and `false` for cancellation.
 *
 * @param amount The recharge amount to be confirmed, displayed on the LCD.
 * @return bool Returns `true` if confirmed with `'#'`, and `false` if canceled with `'*'`.
 */
bool ScreenManager::confirmRecharge(uint16_t amount) {
    // Clear the LCD screen
    LCD->clear();

    // Display the confirmation message (centered manually)
    LCD->setCursor(1, 0);
    LCD->print("Confirm Recharge ?");

    // Prepare the recharge amount text
    String amountText = String(amount) + " Units";
    int textLength = amountText.length();
    int padding = (20 - textLength) / 2; // Calculate padding for centering

    // Display the centered recharge amount
    LCD->setCursor(padding, 2);
    LCD->print(amountText);

    // Display the options
    LCD->setCursor(0, 3);
    LCD->print("No: '*'      Yes: '#'");
    // Await user confirmation or cancellation input
    while (true) {
        char key =  keypadd.getKey(); // Capture keypad input

        // Return true if the user confirms with '#'
        if (key == '#') {
            Buzz->playSuccessTone();
            return true;
        }

        // Return false if the user cancels with '*'
        if (key == '*') {
            Buzz->playFailureTone();
            return false;
        }

        delay(10); // Short delay to prevent rapid polling
    }
}

/**
 * @brief Retrieves and formats the Wi-Fi signal strength as a descriptive string.
 *
 * This function obtains the Wi-Fi signal strength percentage from the Wi-Fi manager
 * and returns a human-readable string that describes the signal quality with a label
 * and its percentage value.
 *
 * Signal Strength Descriptions:
 * - 0%: "No Signal"
 * - 1-20%: "Weak Signal"
 * - 21-40%: "Fair Signal"
 * - 41-60%: "Good Signal"
 * - 61-80%: "Strong Signal"
 * - 81-100%: "Excellent Signal"
 * 
 * @example Example output for different signal strengths:
 * - 0%: "No Signal (0%)"
 * - 15%: "Weak(15%)"
 * - 35%: "Fair(35%)"
 * - 55%: "Good(55%)"
 * - 75%: "Strong(75%)"
 * - 95%: "Excellent(95%)"
 */
void ScreenManager::displayWiFiSignal() {
    int signalStrength = wiFiManager->getSignalStrengthPercent();
	String signalText;    
    // Determine the signal strength description
    if (signalStrength <= 0) {
        signalText = "No Signal(0%)";
    } else if (signalStrength <= 20) {
        signalText = "Weak(" + String(signalStrength) + "%)";
    } else if (signalStrength <= 40) {
        signalText = "Fair(" + String(signalStrength) + "%)";
    } else if (signalStrength <= 60) {
        signalText = "Good(" + String(signalStrength) + "%)";
    } else if (signalStrength <= 80) {
        signalText = "Strong(" + String(signalStrength) + "%)";
    } else {
        signalText = "Best(" + String(signalStrength) + "%)";
    }
    
	LCD->print("Wifi >");
	LCD->print(signalText);
}

/**
 * @brief Displays the Lock Card page for card locking operation.
 * 
 * This function sets up the user interface (UI) elements for the Lock Card page, allowing the user to lock a detected card. The UI includes prompts for the user to place the card for locking, a WiFi signal strength indicator, and the current time. Additionally, it handles user input for keypad interaction and RFID card detection. 
 * 
 * The function displays a success or failure message based on the result of the lock operation and provides the option to restart the process for retrying or exiting.
 *
 * @note The function continuously runs until the user presses '*' to exit or a card is locked.
 */
void ScreenManager::LockCardPage() {

    // Label to restart the function loop for refreshing the display after each operation
    startOver:
    LCD->clear();  // Clear screen
    LCD->setCursor(0, 0);
    LCD->print("LOCK CARD"); // Display page title

    // Display current time in top-left corner
    LCD->setCursor(14, 0);
    LCD->print(Log->getTimeString().c_str());

    // Display Wi-Fi signal strength
    LCD->setCursor(0, 1);
    displayWiFiSignal();  // Display Wi-Fi signal status

    // Display instructions for placing the card
    LCD->setCursor(0, 2);
    LCD->print("    Place card"); // Instruction to place card for locking

    // Display symbols indicating action area for placing card
    LCD->setCursor(0, 3);
    LCD->print(">");  // Left indicator

    LCD->setCursor(19, 3);
    LCD->print("<");  // Right indicator

    // Main loop to check for user input and card detection
    while(true) {
        // Wait for key input from the keypad
        char character =  keypadd.getKey();
        
        // If a key is pressed (not NO_KEY)
        if (character != NO_KEY) {
            // If the '*' key is pressed, exit the Lock Card page
            if (character == '*') return;

            // Check if a card is currently detected by the RFID reader
            if (mRC522Manager->IsCardDetected()) {
                // Check if the detected card is not a master card
                if (!mRC522Manager->IsMasterCard()) {
                    // Attempt to lock the card using the lockCard method
                    if (mRC522Manager->lockCard()) {
                        // If successful, display a success message
                        LCD->setCursor(2, 3);
                        LCD->print("Locking Succeed!"); // Success message
                        goto startOver; // Restart the process to allow for new operations
                    } else {
                        // If locking fails, display a failure message
                        LCD->setCursor(2, 3);
                        LCD->print("Locking Failed!"); // Failure message
                        goto startOver; // Restart the process for retry
                    }
                }
            }
        };
        // Small delay to debounce the keypad input
        delay(200);
    }
}

/**
 * @brief Displays the Home Page layout with user-specific information and time display.
 *
 * This function arranges the Home Page UI elements, showing the user's name, current date and time,
 * WiFi status, and balance units. It also provides details of the last communication event.
 *
 * Layout Details:
 * - Sets font to FreeSansBold9pt7b for unified text appearance.
 * - Draws a white background image and color-coded rectangles for different information sections.
 * - Displays time, date, balance, and last communication information in designated areas.
 *
 * @param nameTop Character array containing the user's name for the top display section.
 * @param nameMid Character array containing a secondary name or title for the middle display section.
 */
void ScreenManager::HomePage(const char* nameTop) {
    static unsigned long lastUpdate = 0;      // Tracks the last update time
    static unsigned long lastUpdate01 = 0;    // Tracks the last update time for case 0
    static int screenState = 0;               // Tracks the current screen state
    const unsigned long interval = 5000;      // Interval for screen change (5 seconds)

    unsigned long currentMillis = millis(); // Current time

    // Check if the interval has passed for screen state update
    if (currentMillis - lastUpdate >= interval) {
        lastUpdate = currentMillis; // Update the last update time

        // Switch through different screens
        switch (screenState) {
            case 0: // Display user and time
                // Update screen only if the interval has passed for this state
                    // Display Home title and time
                    LCD->setCursor(0, 0);
                    LCD->print("HOME");
                    LCD->setCursor(15, 0);
                    LCD->print(Log->getTimeString().c_str()); // Display current time

                    // Display user name
					LCD->setCursor(0, 1);
                	LCD->print("                    ");
                    LCD->setCursor(0, 1);
                    LCD->print("Manager> ");
                    LCD->print(nameTop);

                    // Display current date
					LCD->setCursor(0, 2);
                	LCD->print("                    ");
                    LCD->setCursor(0, 2);
                    LCD->print("Date> ");
                    LCD->print(Log->getDateString().c_str()); // Display current date

                    // Display WiFi signal status
                    LCD->setCursor(0, 3);
                    displayWiFiSignal();

                    // Move to the next state to show balance and last communication
                    screenState = 1;
					while (millis() - lastUpdate01 < interval) {
					// Handle keypad input
                    Kharacter =  keypadd.getKey();
                	if (Kharacter != NO_KEY) return;

                	// Handle MasterCard check
                	mRC522Manager->IsMasterCard();
                	if (mRC522Manager->cardStatusRead == 1 || mRC522Manager->cardStatusRead == 0) return;
                	};
					// Update the last time this screen was shown
                	lastUpdate01 = millis();
                	break;

            case 1: // Display balance and last communication with scrolling
                LCD->clear(); // Clear the screen

                // Display Home title and time again
                LCD->setCursor(0, 0);
                LCD->print("HOME");
                LCD->setCursor(15, 0);
                LCD->print(Log->getTimeString().c_str()); // Display current time

                // Display balance units
				LCD->setCursor(0, 1);
                LCD->print("                    ");
                LCD->setCursor(0, 1);
                LCD->print("Balance> ");
                LCD->print(mRC522Manager->GetBalance()); // Display balance
				LCD->setCursor(15, 1);
                LCD->print("Units");

                // Display last communication info
				LCD->setCursor(0, 2);
                LCD->print("                    ");
                LCD->setCursor(0, 2);
                LCD->print("LastComm> ");
                scrollTextOnLine(Log->getDateString() + String(" ")  + Log->getTimeString() + String(" ") + GetLastRechergAmount(), 10, 19, 2); // Scroll last communication details
                // Display WiFi signal status
                    LCD->setCursor(0, 3);
                    displayWiFiSignal();
				// Move back to the first screen state after displaying balance and communication
                screenState = 0;

                break;
        }
    }

    // Handle keypad input globally
    if (Kharacter != NO_KEY) return;
	Kharacter =  keypadd.getKey();

    // Handle MasterCard check
    if (mRC522Manager->cardStatusRead == 1 || mRC522Manager->cardStatusRead == 0) return;
	    mRC522Manager->IsMasterCard();
}


/**
 * @brief Retrieves the last recharge amount as a string.
 *
 * This function returns the last recharge amount stored in the system as a formatted string.
 * It is used to display the recharge amount in the user interface or for logging purposes.
 *
 * @return String containing the last recharge amount.
 */
String ScreenManager::GetLastRechergAmount() {
    return String(LastAmount);
}
/**
 * @brief Scrolls a text string horizontally on a specific line of the LCD.
 *
 * This function scrolls a given text string from left to right within a specified range of 
 * character positions on the LCD. The scrolling is time-controlled using `millis()` to 
 * maintain a non-blocking behavior, ensuring that other tasks or user inputs can be handled 
 * simultaneously.
 *
 * @param text The text string to scroll on the LCD.
 * @param startX The starting X position (column) for the scrolling text.
 * @param stopX The ending X position (column) for the scrolling text.
 * @param line The line number (row) on which the text will scroll.
 *
 * Scrolling Details:
 * - The text scrolls one character position at a time at intervals defined by `scrollDelay`.
 * - The function continuously clears and updates the specified range to simulate horizontal movement.
 * - Handles user input from a keypad to interrupt the scrolling process.
 *
 * Usage Example:
 * @code
 *   ScreenManager screenManager;
 *   screenManager.scrollTextOnLine("Scrolling Example", 0, 15, 1);
 * @endcode
 */
void ScreenManager::scrollTextOnLine(String text, int startX, int stopX, int line) {
  int len = text.length();
  int scrollIndex = 0;  // Track the starting character for each scroll
  int scrollPosition = startX; // Start scrolling from the left position
  unsigned long lastScrollTime = 0; // Keep track of the last scroll time
  const unsigned long scrollDelay = 500; // Scroll speed in milliseconds

  // Loop to scroll the text continuously until the text disappears completely
  while (scrollPosition < len + 5) {
	// Capture the character from the keypad
    Kharacter =  keypadd.getKey();
    if (Kharacter != NO_KEY) return;

	mRC522Manager->IsMasterCard();
	if(mRC522Manager->cardStatusRead == 1 ||  mRC522Manager->cardStatusRead == 0 ) return;
    // Check if enough time has elapsed to scroll the text
    if (millis() - lastScrollTime >= scrollDelay) {
      lastScrollTime = millis(); // Update the last scroll time

      // Clear the section from startX to stopX
      LCD->setCursor(startX, line);
      for (int i = 0; i < (stopX - startX + 1); i++) {
        LCD->print(" ");  // Print spaces to clear the section
      }

      // Display the text starting from the current scrollIndex
      LCD->setCursor(startX, line);  // Set the cursor back to startX for each frame
      for (int i = scrollIndex; i < scrollIndex + (stopX - startX + 1); i++) {
        if (i < len) {
          LCD->print(text.charAt(i));  // Print the character at the current index
        }
      }

      // Increment scrollIndex to shift the text one position to the left
      scrollIndex++;

      // When scrollIndex exceeds the length of the text, we add spaces at the end
      if (scrollIndex > len) {
        scrollIndex = len; // Keep printing spaces after the text has ended
      }

      // Move the text one position to the right on the LCD
      scrollPosition++;
    }
  }
}


/**
 * @brief Displays the Recharge page layout and processes user input for selecting recharge amounts.
 *
 * This function sets up the UI for the Recharge page, showing current balance, recharge options, 
 * and the current time. It listens for user input via the keypad, allowing them to select a 
 * recharge amount. Upon confirmation, the recharge amount is processed and added to the card balance.
 *
 * Layout Details:
 * - Displays the current time, POS balance, L CARD balance, and selectable recharge options.
 * - Scrolls the recharge options text on the display using the `scrollTextOnLine` method.
 *
 * Functional Highlights:
 * - Handles keypad input for recharge options (`1`, `2`, `3`) or cancellation (`*`).
 * - Validates recharge selection via `confirmRecharge` before processing the transaction.
 * - Allows for smooth non-blocking scrolling of options while awaiting user input.
 *
 * @note The function runs an infinite loop until a valid input is received or canceled with `*`.
 */
void ScreenManager::RechargePage() {
    // Display the Recharge page header and balance details
    LCD->setCursor(0, 0);
    LCD->print("RECHARGE");
    LCD->setCursor(15, 0);
    LCD->print(Log->getTimeString().c_str()); // Display current time

    LCD->setCursor(0, 1);
    LCD->print("POS    > ");
    LCD->print(mRC522Manager->GetBalance());
    LCD->setCursor(15, 1);
    LCD->print("Units");

    LCD->setCursor(0, 2);
    LCD->print("L CARD > ");
    LCD->print(mRC522Manager->GetCardBalance());
    LCD->setCursor(15, 2);
    LCD->print("Units");

    LCD->setCursor(0, 3);
    LCD->print("Select> ");
    int Amount = 0;
    // Infinite loop to process user input and scroll options
    while (true) {
        delay(10);
        // Check for keypad input
        Kharacter = keypadd.getKey();
        if (Kharacter != NO_KEY) goto in;

        // Scroll the recharge options on line 3
        scrollTextOnLine(" 1-100 2-200 3-INPUT", 7, 19, 3);

    in:
        if (Kharacter != NO_KEY) {
            // Process the input based on the selected option
            switch (Kharacter) {
                case '1':
                    if (mRC522Manager->GetBalance() < 100) {
                        clearScreen();
                        LCD->setCursor(0, 0);
                        LCD->print("HOME");
                        LCD->setCursor(15, 0);
                        LCD->print(Log->getTimeString().c_str()); // Display current time
                        Buzz->playFailureTone();
                        displayCenteredText(">    LOW BALANCE   <", 2);
                        displayCenteredText("> RECHARGE FAILED! <", 2);
                        delay(3000);
                        return;
                    }
                    // Recharge with 100 units
                    if (confirmRecharge(100)) {
                        int HoldBal = mRC522Manager->GetCardBalance();
                        mRC522Manager->resetRFID();
                        if(mRC522Manager->Recharge(100)){
                        clearScreen();
                        LCD->setCursor(0, 0);
                        Buzz->playSuccessTone();
                        LCD->print("HOME");
                        LCD->setCursor(15, 0);
                        LCD->print(Log->getTimeString().c_str()); // Display current time
                        displayCenteredText("> RECHARGE SUCCEEDED! <", 1);
                        LCD->setCursor(0, 2);
                        LCD->print("HOLD BAL >");
                        LCD->print(HoldBal);
                        LCD->setCursor(0, 2);
                        LCD->print("Units");

                        LCD->setCursor(0, 3);
                        LCD->print("NEW BAL >");
                        LCD->print(HoldBal + 100);
                        LCD->setCursor(0, 3);
                        LCD->print("Units");
                        delay(3000);
                        } else {
                        clearScreen();
                        LCD->setCursor(0, 0);
                        LCD->print("HOME");
                        LCD->setCursor(15, 0);
                        LCD->print(Log->getTimeString().c_str()); // Display current time
                        Buzz->playFailureTone();
                        displayCenteredText(">   CARD NOT VALID   <", 1);
                        displayCenteredText(">   OR WRITE ERROR   <", 2);
                        displayCenteredText(">  RECHARGE FAILED! <", 3);
                        delay(3000);
                        }
                    }
                    return;

                case '2':
                    if (mRC522Manager->GetBalance() < 200) {
                        clearScreen();
                        LCD->setCursor(0, 0);
                        LCD->print("HOME");
                        LCD->setCursor(15, 0);
                        LCD->print(Log->getTimeString().c_str()); // Display current time
                        Buzz->playFailureTone();
                        displayCenteredText(">    LOW BALANCE   <", 2);
                        displayCenteredText("> RECHARGE FAILED! <", 2);
                        delay(3000);
                        return;
                    }
                    // Recharge with 200 units
                    if (confirmRecharge(200)) {
                        int HoldBal = mRC522Manager->GetCardBalance();
                        mRC522Manager->resetRFID();
                        if(mRC522Manager->Recharge(200)){
                        clearScreen();
                        LCD->setCursor(0, 0);
                        LCD->print("HOME");
                        LCD->setCursor(15, 0);
                        Buzz->playSuccessTone();
                        LCD->print(Log->getTimeString().c_str()); // Display current time
                        displayCenteredText("> RECHARGE SUCCEEDED! <", 1);
                        LCD->setCursor(0, 2);
                        LCD->print("HOLD BAL >");
                        LCD->print(HoldBal);
                        LCD->setCursor(15, 2);
                        LCD->print("Units");

                        LCD->setCursor(0, 3);
                        LCD->print("NEW BAL >");
                        LCD->print(HoldBal + 200);
                        LCD->setCursor(15, 3);
                        LCD->print("Units");
                        delay(3000);
                        } else {
                        clearScreen();
                        LCD->setCursor(0, 0);
                        LCD->print("HOME");
                        LCD->setCursor(15, 0);
                        LCD->print(Log->getTimeString().c_str()); // Display current time
                        Buzz->playFailureTone();
                        displayCenteredText(">   CARD NOT VALID   <", 1);
                        displayCenteredText(">   OR WRITE ERROR   <", 2);
                        displayCenteredText(">  RECHARGE FAILED! <", 3);
                        delay(3000);
                        }
                    }
                    return;

                case '3': Amount = ShowPrompt("ENTER THE AMOUNT","Units").toInt();
                if (mRC522Manager->GetBalance() < Amount) {
                        clearScreen();
                        LCD->setCursor(0, 0);
                        LCD->print("HOME");
                        LCD->setCursor(15, 0);
                        LCD->print(Log->getTimeString().c_str()); // Display current time
                        Buzz->playFailureTone();
                        displayCenteredText(">    LOW BALANCE   <", 2);
                        displayCenteredText("> RECHARGE FAILED! <", 2);
                        delay(3000);
                        return;
                    }
                    // Recharge with 200 units
                    if (confirmRecharge(Amount)) {
                        int HoldBal = mRC522Manager->GetCardBalance();
                        mRC522Manager->resetRFID();
                        if(mRC522Manager->Recharge(Amount)){
                        clearScreen();
                        LCD->setCursor(0, 0);
                        LCD->print("HOME");
                        LCD->setCursor(15, 0);
                        Buzz->playSuccessTone();
                        LCD->print(Log->getTimeString().c_str()); // Display current time
                        displayCenteredText("> RECHARGE SUCCEEDED! <", 1);

                        LCD->setCursor(0, 2);
                        LCD->print("HOLD BAL >");
                        LCD->print(HoldBal);
                        LCD->setCursor(0, 2);
                        LCD->print("Units");

                        LCD->setCursor(0, 3);
                        LCD->print("NEW BAL >");
                        LCD->print(HoldBal + Amount);
                        LCD->setCursor(15, 3);
                        LCD->print("Units");
                        delay(3000);
                        } else {
                        clearScreen();
                        LCD->setCursor(0, 0);
                        LCD->print("HOME");
                        LCD->setCursor(15, 0);
                        LCD->print(Log->getTimeString().c_str()); // Display current time
                        Buzz->playFailureTone();
                        displayCenteredText(">   CARD NOT VALID   <", 1);
                        displayCenteredText(">   OR WRITE ERROR   <", 2);
                        displayCenteredText(">  RECHARGE FAILED! <", 3);
                        delay(3000);
                        }
                    }
                    return;

                case '*':
                    // Exit the Recharge page
                    return;

                default:
                    // Handle invalid inputs (optional)
                    break;
            }
        }
    }
}

void ScreenManager::MasterMode() {

}
/**
 * @brief Displays action selection options and processes user input via the keypad.
 *
 * This function sets up the UI for selecting an action, displaying a menu with options for 
 * Recharge, Setting a Number, or returning to the Home page. It listens for keypad input 
 * and returns the character corresponding to the selected action.
 *
 * @return `'1'` for Recharge, `'2'` for Set Number, or `'*'` for Home.
 */
char ScreenManager::SelectAction() {
    Buzz->playSuccessTone();
    // Display header and time
    LCD->clear();
    LCD->setCursor(0, 0);
    LCD->print("SELECT ");
    LCD->setCursor(15, 0);
    LCD->print(Log->getTimeString().c_str()); // Replace with actual time function

    // Display menu options
    LCD->setCursor(0, 1);
    LCD->print("01   > RECHARGE");

    LCD->setCursor(0, 2);
    LCD->print("02   > SET NUMBER");

    LCD->setCursor(0, 3);
    LCD->print("L CARD > ");
    LCD->print(mRC522Manager->GetCardBalance());
    LCD->setCursor(15, 3);
    LCD->print("Units");

    // Initialize the character
    Kharacter = NO_KEY;

    // Loop to wait for valid input
    while (true) {
        Kharacter = keypadd.getKey(); // Capture user input
        mRC522Manager->resetRFID();
        if(!mRC522Manager->IsCardDetected()){ 
            Buzz->playFailureTone();
            return '*';}

        if (Kharacter != NO_KEY) {
            switch (Kharacter) {
                case '1': Buzz->playSuccessTone();
                    return '1'; // Recharge selected
                case '2': Buzz->playSuccessTone();
                    return '2'; // Set Number selected
                case '*': Buzz->playFailureTone();
                    return '*'; // Home selected
                default:
                    // Optionally display an error message for invalid input
                    // LCD->setCursor(0, 3);
                    // LCD->print("INVALID SELECTION");
                    break;
            }
        }
        // Optional: Add a small delay to avoid rapid polling
        delay(10);
    }
}


/**
 * @brief Displays the User Mode screen, allowing users to view and manage their saved numbers.
 *
 * This function creates the interface for User Mode, displaying NFC card numbers stored in memory.
 * Users can update individual numbers or return to the previous menu by pressing the '*' key.
 *
 * Layout Details:
 * - Displays a list of saved numbers with corresponding action prompts.
 * - Shows the current time at the bottom as a scrolling message.
 * - Waits for keypad input to select a number for updating or exit the screen.
 */
void ScreenManager::UserMode() {
    start:;
    Buzz->playSuccessTone();
    // Display the saved numbers with corresponding actions
    LCD->clear();

    // Display first number
    LCD->setCursor(0, 0);
    LCD->print("01 > ");
    
    if (mRC522Manager->GetNum01().isEmpty()) {
        LCD->print("No Number");
    } else {
        LCD->print(mRC522Manager->GetNum01());
    }
    LCD->setCursor(15, 0);
    LCD->print("| SET");

    // Display second number
    LCD->setCursor(0, 1);
    LCD->print("02 > ");
    if (mRC522Manager->GetNum02().isEmpty()) {
        LCD->print("No Number");
    } else {
        LCD->print(mRC522Manager->GetNum02());
    }
    LCD->setCursor(15, 1);
    LCD->print("| THE");

    // Display third number
    LCD->setCursor(0, 2);
    LCD->print("03 > ");
    if (mRC522Manager->GetNum03().isEmpty()) {
        LCD->print("No Number");
    } else {
        LCD->print(mRC522Manager->GetNum03());
    }
    LCD->setCursor(15, 2);
    LCD->print("| NUM");

    // Display fourth number
    LCD->setCursor(0, 3);
    LCD->print("04 > ");
    if (mRC522Manager->GetNum04().isEmpty()) {
        LCD->print("No Number");
    } else {
        LCD->print(mRC522Manager->GetNum04());
    }
    LCD->setCursor(15, 3);
    LCD->print("|");

    // Display scrolling text with the current time
    scrollTextOnLine(
        String("     ") + String(Log->getTimeString()) + String("            "),
        16, // Start position for the text
        19, // End position for the text
        3   // Line number for scrolling text
    );
    delay(500);// wait before getting new entry
    String prompt;
    // Continuous loop to handle user input
    while (true) {
        // Check for keypad input
        Kharacter =  keypadd.getKey();
        delay(50);
        mRC522Manager->resetRFID();
        if(!mRC522Manager->IsCardDetected()) return;

        if (Kharacter != NO_KEY) {
            // If the '*' key is pressed, exit User Mode
            if (Kharacter == '*') return;

            // Handle number selection for updating
            switch (Kharacter) {
                case '1':  prompt = ShowPrompt("Enter New Number", mRC522Manager->GetNum01().c_str());
                    if(mRC522Manager->SaveNum01(prompt)){
                    clearScreen();
                        LCD->setCursor(0, 0);
                        LCD->print("USER");
                        LCD->setCursor(15, 0);
                        LCD->print(Log->getTimeString().c_str()); // Display current time
                        Buzz->playSuccessTone();
                        displayCenteredText(">     NEW NUMBER 01   <", 2);
                        displayCenteredText("> SAVED SUCCESSFULLY! <", 2);
                        delay(3000);
                        } else {
                        clearScreen();
                        LCD->setCursor(0, 0);
                        LCD->print("HOME");
                        LCD->setCursor(15, 0);
                        LCD->print(Log->getTimeString().c_str()); // Display current time
                        Buzz->playFailureTone();
                        displayCenteredText(">   CARD NOT VALID   <", 1);
                        displayCenteredText(">   OR WRITE ERROR   <", 2);
                        displayCenteredText("> OPERATION FAILED!  <", 3);
                        delay(3000);
                        };
                    goto start;
                case '2':  prompt = ShowPrompt("Enter New Number", mRC522Manager->GetNum02().c_str());
                    if(mRC522Manager->SaveNum02(prompt)){
                    clearScreen();
                        LCD->setCursor(0, 0);
                        LCD->print("USER");
                        LCD->setCursor(15, 0);
                        Buzz->playSuccessTone();
                        LCD->print(Log->getTimeString().c_str()); // Display current time
                        displayCenteredText(">     NEW NUMBER 02   <", 2);
                        displayCenteredText("> SAVED SUCCESSFULLY! <", 2);
                        delay(3000);
                        } else {
                        clearScreen();
                        LCD->setCursor(0, 0);
                        LCD->print("HOME");
                        LCD->setCursor(15, 0);
                        LCD->print(Log->getTimeString().c_str()); // Display current time
                        Buzz->playFailureTone();
                        displayCenteredText(">   CARD NOT VALID   <", 1);
                        displayCenteredText(">   OR WRITE ERROR   <", 2);
                        displayCenteredText("> OPERATION FAILED!  <", 3);
                        delay(3000);
                        };
                    goto start;
                case '3':  prompt = ShowPrompt("Enter New Number", mRC522Manager->GetNum03().c_str());
                    if(mRC522Manager->SaveNum03(prompt)){
                    clearScreen();
                        LCD->setCursor(0, 0);
                        LCD->print("USER");
                        LCD->setCursor(15, 0);
                        Buzz->playSuccessTone();
                        LCD->print(Log->getTimeString().c_str()); // Display current time
                        displayCenteredText(">     NEW NUMBER 03   <", 2);
                        displayCenteredText("> SAVED SUCCESSFULLY! <", 2);
                        delay(3000);
                        } else {
                        clearScreen();
                        LCD->setCursor(0, 0);
                        LCD->print("HOME");
                        LCD->setCursor(15, 0);
                        LCD->print(Log->getTimeString().c_str()); // Display current time
                        Buzz->playFailureTone();
                        displayCenteredText(">   CARD NOT VALID   <", 1);
                        displayCenteredText(">   OR WRITE ERROR   <", 2);
                        displayCenteredText("> OPERATION FAILED! <", 3);
                        delay(3000);
                        };
                    goto start;
                case '4':  prompt = ShowPrompt("Enter New Number", mRC522Manager->GetNum04().c_str());
                    if(mRC522Manager->SaveNum04(prompt)){
                    clearScreen();
                        LCD->setCursor(0, 0);
                        LCD->print("USER");
                        LCD->setCursor(15, 0);
                        Buzz->playSuccessTone();
                        LCD->print(Log->getTimeString().c_str()); // Display current time
                        displayCenteredText(">     NEW NUMBER 04   <", 2);
                        displayCenteredText("> SAVED SUCCESSFULLY! <", 2);
                        delay(3000);
                        } else {
                        clearScreen();
                        LCD->setCursor(0, 0);
                        LCD->print("HOME");
                        LCD->setCursor(15, 0);
                        LCD->print(Log->getTimeString().c_str()); // Display current time
                        Buzz->playFailureTone();
                        displayCenteredText(">   CARD NOT VALID   <", 1);
                        displayCenteredText(">   OR WRITE ERROR   <", 2);
                        displayCenteredText("> OPERATION FAILED! <", 3);
                        delay(3000);
                        };
                    goto start;
                default:
                    // Ignore invalid inputs and continue waiting
                    break;
            };

        }
    }
}

/**
 * @brief Displays the Security Check page, prompting the user to scan the master key for access.
 *
 * This function sets up the UI layout for the Security Check page. It displays instructions,
 * checks for an RFID card, and validates if it is the master key. If the correct card is detected,
 * the device unlocks. Otherwise, it displays a failure message and retries.
 *
 * Layout Details:
 * - Displays the prompt for scanning the master key.
 * - Shows real-time status updates based on RFID card detection.
 * - Includes a restart mechanism to refresh the display for new attempts.
 */
void ScreenManager::SecurityCheck() {
start:
        // Clear the screen and display the main security check prompt
        LCD->clear();
        LCD->setCursor(0, 0);
        LCD->print("SECURITY CHECK");
		LCD->setCursor(15, 0);
        LCD->print(Log->getTimeString().c_str()); // Replace with actual time function

        // Display instructions for scanning the master key
        LCD->setCursor(0, 1);
        LCD->print("SCAN MASTER KEY");

    while (true) {

        // Check for the presence of an RFID card
        mRC522Manager->SecureCheck();  // Call IsMasterCard once

        if (mRC522Manager->cardStatusRead  == 1) {
            // If the card is valid, display a success message and unlock the device
            LCD->clear();
            //displaySuccessMessage("ACCESS GRANTED!",3, 2);
            Buzz->playSuccessTone();
            LCD->print("ACCESS GRANTED!");
            return;        // Exit the loop and Security Check function
        } else if (mRC522Manager->cardStatusRead  == 0) {
            LCD->clear();
            Buzz->playFailureTone();
            LCD->print("INVALID KEY!");
            goto start;
        }

    }
}


/**
 * @brief Displays the Access Point (AP) setup page on the screen.
 *
 * This function sets up the user interface for configuring WiFi settings in AP mode.
 * The page includes the current time, WiFi status, AP address, and instructions for user interaction.
 *
 * Layout Details:
 * - Displays the current time in the top-left corner.
 * - Shows a warning that WiFi is not set in the center.
 * - Includes the device name and AP address for reference.
 * - Shows a WiFi signal strength icon at the top-right.
 */
void ScreenManager::GotoApPage() {
    // Clear the screen
    LCD->clear();

    // Display current time at the top-left corner
    LCD->setCursor(0, 0);
    LCD->print("AP MODE");
	LCD->setCursor(15, 0);
    LCD->print(Log->getTimeString());

    // Show the WiFi status warning in the center
    LCD->setCursor(0, 1);
    LCD->print("** WIFI NOT SET **");

    // Display the Access Point (AP) address for reference
    LCD->setCursor(0, 2);
    LCD->print("AP Address: ");
    LCD->print(wiFiManager->Message);

    // Display the WiFi device name at the bottom
    LCD->setCursor(0, 3);
    LCD->print("Name: DeviceRF");

    // Continuously update the WiFi signal icon
    while (true) {
        //displayWiFiSignal(10, 1);  // Adjust position as needed
        delay(1000);              // Refresh every second
    }
}
