#include "MRC522Manager.h"



// Define a 16-bit authentication acknowledgment (pACK) for NTAG authentication
byte ackKey[2] = {0x12, 0x34};  // Example values, should be set according to your tag's access requirements

// Define ACCESS_KEY as an alias for pACK
#define ACCESS_KEY ackKey

// Constructor
MRC522Manager::MRC522Manager(ConfigManager* Config,MFRC522* RFID) : RFID(RFID),Config(Config) {

}
// Initialize the MFRC522
/**
 * @brief Initializes the MFRC522 module and sets up SPI.
 *
 * This function initializes the MFRC522 and prepares it to read cards.
 */
void MRC522Manager::begin() {
    Prepare(ACTIVE_KEY);//8 initialize the key
    Serial.println(F("MFRC522 initialized."));
}

/**
 * @brief Authenticates an RFID card using both Key A and Key B.
 *
 * This function attempts to authenticate the RFID card for a specific sector and block 
 * using two different keys: Key A and Key B. It first authenticates using Key A, and if 
 * successful, it attempts authentication using Key B. If either authentication fails, 
 * the function returns false.
 *
 * @param sector The sector of the RFID card to authenticate.
 * @param block The block within the sector to authenticate.
 * @return true If authentication with both Key A and Key B is successful.
 * @return false If authentication with either Key A or Key B fails.
 */

bool MRC522Manager::authenticateWithKeys(byte sector, byte block) {
    MFRC522::MIFARE_Key keyAAuth, keyBAuth;
    
    // Set the provided Key A and Key B for authentication
    for (byte i = 0; i < 6; i++) {
        keyAAuth.keyByte[i] = keyAuthA[i];
        keyBAuth.keyByte[i] = keyAuthB[i];
    }

    // Authenticate using Key A
    if (RFID->PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block+sector, &keyAAuth, &(RFID->uid)) != MFRC522::STATUS_OK) {
        Serial.println("Authentication failed for Key A");
        return false;
    }

    // Authenticate using Key B (optional)
    if (RFID->PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, block+sector, &keyBAuth, &(RFID->uid)) != MFRC522::STATUS_OK) {
        Serial.println("Authentication failed for Key B");
        return false;
    }

    return true;  // Authentication successful for both keys
}

/**
 * @brief Prepares the MRC522 RFID reader with a specified authentication key.
 *
 * This function initializes the authentication key (keyA) for sector access on an MRC522 RFID reader.
 * The caller can specify a custom 6-byte key; if no key is provided, it will default to 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF.
 * 
 * @param customKey A pointer to a 6-byte array representing the custom authentication key. 
 *                  If `nullptr` is passed, the default key is used.
 * 
 * @note This function is typically called during setup to initialize the reader 
 * for subsequent read or write operations.
 */
void MRC522Manager::Prepare(const byte* customKey ) {
    // Set the authentication key (use default if customKey is not provided)
    for (byte i = 0; i < 6; i++) {
        keyA.keyByte[i] = customKey ? customKey[i] : 0xFF; // Use provided key or default to 0xFF
    }

}
/**
 * @brief Secures the tag with a custom access key provided by the user.
 * 
 * This function sets a custom authentication key and applies read and write protection
 * on all pages of the tag.
 * 
 * @param userKey Array of 4 bytes representing the custom access key.
 * @param userACK Array of 2 bytes representing the custom authentication acknowledgment bytes.
 * @return true if the operation is successful, false otherwise.
 */
bool MRC522Manager::SecureTag(byte *userKey, byte *userACK) {
    // Step 1: Write the custom password to the password page (0xE5 for NTAG216)
    if (!writePage(0xE5, userKey, 4)) {
        Serial.println("Failed to write custom key.");
        return false;
    }

    // Step 2: Write the custom ACK to the ACK page (0xE6 for NTAG216)
    byte ackPage[4] = {userACK[0], userACK[1], 0x00, 0x00};
    if (!writePage(0xE6, ackPage, 4)) {
        Serial.println("Failed to write custom ACK.");
        return false;
    }

    // Step 3: Set the first protected page to 0 (protecting all pages from read and write access)
    byte protectAllPages[4] = {0x00, 0x00, 0x00, 0x00};  // Protect from page 0 onwards
    if (!writePage(0xE3, protectAllPages, 4)) {
        Serial.println("Failed to set full write protection.");
        return false;
    }

    // Step 4: Set read protection for all pages with access control
    byte accessControl[4] = {0x80, 0x00, 0x00, 0x00};  // Enables full read protection
    if (!writePage(0xE4, accessControl, 4)) {
        Serial.println("Failed to set full read protection.");
        return false;
    }

    Serial.println("Tag successfully secured with full read and write protection.");
    return true;
}


/**
 * @brief Writes data to a specific page on the card.
 * 
 * @param page The page number to write to.
 * @param data The data to be written.
 * @param len The length of the data to be written (max 4 bytes).
 * @return true if the write operation is successful, false otherwise.
 */
bool MRC522Manager::writePage(byte page, byte *data, byte len) {
    MFRC522::StatusCode status = RFID->MIFARE_Ultralight_Write(page, data, len);
    if (status != MFRC522::STATUS_OK) {
        Serial.print("MIFARE_Write() failed: ");
        Serial.println(RFID->GetStatusCodeName(status));
        return false;
    }
    return true;
}
// Check if a card is detected
/**
 * @brief Checks if a card is currently detected by the MFRC522 reader.
 *
 * This function checks for the presence of a new card and returns true if a card is detected.
 *
 * @return true if a card is detected, false otherwise.
 */
bool MRC522Manager::IsCardDetected() {
    return RFID->PICC_IsNewCardPresent();
}
// Read RFID card
/**
 * @brief Reads an RFID card.
 *
 * @return true if a card is detected and read successfully, false otherwise.
 */
bool MRC522Manager::readCard() {
    Prepare(ACTIVE_KEY);

    if (!RFID->PICC_IsNewCardPresent()) {
        return false;  // No new card present
    }

    if (!RFID->PICC_ReadCardSerial()) {
        return false;  // Failed to read card
    }

    // Check the card type
    MFRC522::PICC_Type piccType = RFID->PICC_GetType(RFID->uid.sak);
    if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI
        && piccType != MFRC522::PICC_TYPE_MIFARE_1K
        && piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
        Serial.println(F("This sample only works with MIFARE Classic cards."));
        return false; // Card type not supported
    }

    return true;  // Card read successfully
}

/**
 * @brief Prints the contents of a byte array in hexadecimal format.
 *
 * This function iterates through a byte array, printing each byte in hexadecimal format
 * to the Serial output. If a byte is less than 0x10, it prepends a '0' to maintain
 * consistent formatting for single-digit hex values.
 *
 * @param buffer Pointer to the byte array to be printed.
 * @param bufferSize The number of bytes in the array.
 */
void MRC522Manager::printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

/**
 * @brief Prints the contents of a byte array in decimal format.
 *
 * This function iterates through a byte array, printing each byte in decimal format
 * to the Serial output. If a byte is less than 10, it prepends a '0' to maintain
 * consistent formatting for single-digit decimal values.
 *
 * @param buffer Pointer to the byte array to be printed.
 * @param bufferSize The number of bytes in the array.
 */
void MRC522Manager::printDec(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], DEC);
  }
}

/**
 * @brief Retrieves the UID of the currently scanned RFID card.
 *
 * This function attempts to detect and read the UID of a scanned RFID card by first 
 * preparing the reader with the `USER_KEY` and checking for a new card presence via 
 * `PICC_IsNewCardPresent()`. If a card is detected, `PICC_ReadCardSerial()` reads the card's 
 * serial data. Supported card types are checked (MIFARE Classic Mini, 1K, and 4K), and 
 * unsupported types result in an empty string and a diagnostic message.
 *
 * If a supported card is detected, the UID is extracted byte-by-byte, formatted in 
 * hexadecimal with leading zeros for single-digit values, and separated by colons. 
 * After obtaining the UID, communication with the card is halted, and encryption on the 
 * reader is stopped.
 *
 * @return A formatted hexadecimal string representing the UID of the card, or an empty 
 * string if no card is present or if an unsupported card type is detected.
 */
String MRC522Manager::getCardUID() {
    String uid = "";  // Initialize UID string to empty
    Prepare(ACTIVE_KEY);
    
    // Check for the presence of a new card
    if (!RFID->PICC_IsNewCardPresent()) {
        return uid;  // No card detected
    }
    
    // Attempt to read the card's serial number
    if (!RFID->PICC_ReadCardSerial()) {
        return uid;  // Failed to read card data
    }
    
    // Determine the card type and ensure it's a supported MIFARE Classic card
    MFRC522::PICC_Type piccType = RFID->PICC_GetType(RFID->uid.sak);
    if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI
        && piccType != MFRC522::PICC_TYPE_MIFARE_1K
        && piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
        Serial.println(F("This sample only works with MIFARE Classic cards."));
        return uid;  // Unsupported card type
    }
    
    // Format UID bytes as a hexadecimal string with colon separation
    for (byte i = 0; i < RFID->uid.size; i++) {
        if (RFID->uid.uidByte[i] < 0x10) {
            uid += "0";  // Leading zero for single-digit hex values
        }
        uid += String(RFID->uid.uidByte[i], HEX);  // Append hex byte
        if (i < RFID->uid.size - 1) {
            uid += ":";  // Separate bytes with a colon
        }
    }
    
    // Halt card communication and disable encryption on the reader
    RFID->PICC_HaltA();      // Stop communication with the card
    RFID->PCD_StopCrypto1(); // Stop encryption

    Serial.print(F("Card UID: "));
    Serial.println(uid);

    return uid;  // Return the formatted UID string
}



/**
 * @brief Converts data to hexadecimal format and writes it to a specified block in a defined sector of the RFID card.
 *
 * @param sector The sector number to write to (0-15).
 * @param block The block number within the sector (0-3).
 * @param data A pointer to the data to write (16 bytes in decimal or other format).
 * @return true if the write operation was successful, false otherwise.
 */
bool MRC522Manager::writeDataToBlockHex(byte sector, byte block, byte* data) {
    byte sectorBlock = sector * 4 + block; // Calculate block number based on sector and block
    byte blockAddr = sectorBlock;         // Block address
    MFRC522::StatusCode status;
    Prepare(ACTIVE_KEY);

    // Check for the presence of a new card
    if (!RFID->PICC_IsNewCardPresent()) {
        return false; // No card detected
    }

    // Attempt to read the card's serial number
    if (!RFID->PICC_ReadCardSerial()) {
        return false; // Failed to read card data
    }

    // Determine the card type and ensure it's a supported MIFARE Classic card
    MFRC522::PICC_Type piccType = RFID->PICC_GetType(RFID->uid.sak);
    if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
        piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
        piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
        Serial.println(F("This sample only works with MIFARE Classic cards."));
        return false; // Unsupported card type
    }

    // Authenticate with the first key (A key) of the sector
    status = RFID->PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockAddr, &keyA, &(RFID->uid));
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("Authentication failed: "));
        Serial.println(RFID->GetStatusCodeName(status));
        return false; // Authentication failed
    }

    // Prepare hexadecimal data buffer
    byte hexData[16];
    for (int i = 0; i < 16; i++) {
        hexData[i] = data[i] & 0xFF; // Ensure the data is in valid hexadecimal format
    }

    // Log the converted hexadecimal data
    Serial.print(F("Writing hexadecimal data to sector "));
    Serial.print(sector);
    Serial.print(F(", block "));
    Serial.print(block);
    Serial.print(F(": "));
    for (int i = 0; i < 16; i++) {
        Serial.print(hexData[i], HEX);
        Serial.print(" ");
    }
    Serial.println();

    // Write the hexadecimal data to the specified block
    status = RFID->MIFARE_Write(blockAddr, hexData, 16); // Write 16 bytes of hex data
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("Write failed: "));
        Serial.println(RFID->GetStatusCodeName(status));
        return false; // Write failed
    }

    // Halt card communication and disable encryption on the reader
    RFID->PICC_HaltA();      // Stop communication with the card
    RFID->PCD_StopCrypto1(); // Stop encryption
    delay(10);

    return true; // Write successful
}

// Write data to a specified block in the defined sector
/**
 * @brief Writes data to a specified block in a defined sector of the RFID card.
 *
 * @param sector The sector number to write to (0-15).
 * @param block The block number within the sector (0-3).
 * @param data A pointer to the data to write (16 bytes).
 * @return true if the write operation was successful, false otherwise.
 */
bool MRC522Manager::writeDataToBlock(byte sector, byte block, byte* data) {
    byte sectorBlock = sector * 4 + block; // Calculate block number based on sector and block
    byte blockAddr = sectorBlock;         // Block address
    MFRC522::StatusCode status;
    Prepare(ACTIVE_KEY);

    // Check for the presence of a new card
    if (!RFID->PICC_IsNewCardPresent()) {
        return false; // No card detected
    }

    // Attempt to read the card's serial number
    if (!RFID->PICC_ReadCardSerial()) {
        return false; // Failed to read card data
    }

    // Determine the card type and ensure it's a supported MIFARE Classic card
    MFRC522::PICC_Type piccType = RFID->PICC_GetType(RFID->uid.sak);
    if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
        piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
        piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
        Serial.println(F("This sample only works with MIFARE Classic cards."));
        return false; // Unsupported card type
    }

    if (sectorBlock == 40 || sectorBlock == 41) {
        // Authenticate the sector with the default key
        if (!authenticateWithKeys(NUM012_AUTH, 0)) {
            Serial.println("Authentication failed");
            return false; // Authentication failed
        }
    }

    if (sectorBlock == 44 || sectorBlock == 45) {
        // Authenticate the sector with the default key
        if (!authenticateWithKeys(NUM034_AUTH, 0)) {
            Serial.println("Authentication failed");
            return false; // Authentication failed
        }
    }

    if (sectorBlock == 37) {
        // Authenticate the sector with the default key
        if (!authenticateWithKeys(BALANCE_AUTH, 0)) {
            Serial.println("Authentication failed");
            return false; // Authentication failed
        }
    }

    // Write data to the specified block
    status = RFID->MIFARE_Write(blockAddr, data, 16); // Write 16 bytes of data

    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("Write failed: "));
        Serial.println(RFID->GetStatusCodeName(status));
        return false; // Write failed
    }

    // Halt card communication and disable encryption on the reader
    RFID->PICC_HaltA();      // Stop communication with the card
    RFID->PCD_StopCrypto1(); // Stop encryption
    delay(10);

    return true; // Write successful
}

/**
 * @brief Locks the card and writes data to specified blocks.
 * This function performs the following operations:
 * - Authenticates the card with the default key.
 * - Writes "LOCKED" to Sector 9, Block 0.
 * - Writes "123" to Sector 9, Block 3.
 * - Writes "OKAY.PARENT NUMB" to Sector 11, Block 2.
 * - Writes "AMP_AUTH" to Sector 15, Block 0.
 * - Writes "010101010102" to Sector 9, Block 3.
 * @return True if all operations were successful, otherwise false.
 */
bool MRC522Manager::lockCard() {
    String uid = "";  // Initialize UID string to empty
    Prepare(ACTIVE_KEY);
    
    // Check for the presence of a new card
    if (!RFID->PICC_IsNewCardPresent()) {
        return uid;  // No card detected
    }
    
    // Attempt to read the card's serial number
    if (!RFID->PICC_ReadCardSerial()) {
        return uid;  // Failed to read card data
    }
    
    // Determine the card type and ensure it's a supported MIFARE Classic card
    MFRC522::PICC_Type piccType = RFID->PICC_GetType(RFID->uid.sak);
    if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI
        && piccType != MFRC522::PICC_TYPE_MIFARE_1K
        && piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
        Serial.println(F("This sample only works with MIFARE Classic cards."));
        return uid;  // Unsupported card type
    }
    
    // Format UID bytes as a hexadecimal string with colon separation
    for (byte i = 0; i < RFID->uid.size; i++) {
        if (RFID->uid.uidByte[i] < 0x10) {
            uid += "0";  // Leading zero for single-digit hex values
        }
        uid += String(RFID->uid.uidByte[i], HEX);  // Append hex byte
        if (i < RFID->uid.size - 1) {
            uid += ":";  // Separate bytes with a colon
        }
    };
    byte status;

    // Authenticate with the default key
    status = RFID->PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 0, &keyA, &(RFID->uid));
    if (status != MFRC522::STATUS_OK) {
        Serial.println(F("Authentication with default key failed."));
        return false; // Authentication failed
    }

    // Write the specified data to blocks
    byte dataToWrite[16];

    // Lock the card
    byte lockData[] = {'L', 'O', 'C', 'K', 'E', 'D', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    if (!writeDataToBlock(9, 0, lockData)) {
        Serial.println(F("Failed to lock card."));
        return false;
    }

    // Write data "123" to sector 9, block 3
    byte data123[] = {'1', '2', '3', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    if (!writeDataToBlock(9, 3, data123)) {
        Serial.println(F("Failed to write data 123."));
        return false;
    }

    // Write "OKAY.PARENT NUMB" to sector 11, block 2
    byte dataOkay[] = {'O', 'K', 'A', 'Y', '.', 'P', 'A', 'R', 'E', 'N', 'T', ' ', 'N', 'U', 'M', 'B'};
    if (!writeDataToBlock(11, 2, dataOkay)) {
        Serial.println(F("Failed to write OKAY.PARENT NUMB."));
        return false;
    }

    // Write "AMP_AUTH" to sector 15, block 0
    byte dataAuth[] = {'A', 'M', 'P', '_', 'A', 'U', 'T', 'H', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    if (!writeDataToBlock(15, 0, dataAuth)) {
        Serial.println(F("Failed to write AMP_AUTH."));
        return false;
    }

    // Write "010101010102" to sector 9, block 3
    byte keysData[] = {0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    if (!writeDataToBlock(9, 3, keysData)) {
        Serial.println(F("Failed to write keys to block 3."));
        return false;
    }

    // If all writes succeed, indicate success
    Serial.println(F("Success! Data written successfully."));
    return true; // All operations successful
}

/**
 * @brief Recharges the balance by writing a uint value to Sector 9, Block 1.
 * This function checks if the current balance is greater than 0 before
 * writing to the card and updating the balance in preferences.
 * Maximum Value of a 64-bit Unsigned Integer: 18,446,744,073,709,551,615 
 * 
 * @param amount The amount to be recharged and written to the card.
 * @return True if the operation was successful, otherwise false.
 */
bool MRC522Manager::Recharge(uint32_t amount) {
    // Retrieve the current balance
    uint64_t currentBalance = Config->GetULong64(BALANCE, 0); // Default to 0 if not set

    // Check if the current balance is valid
    if (currentBalance <= 0) {
        Serial.println(F("Current balance is zero or negative. Recharge not performed."));
        return false; // Balance must be greater than 0 to perform recharge
    }

    // Check if the recharge amount is valid
    if (amount > currentBalance) {
        Serial.println(F("Recharge amount exceeds current balance."));
        return false; // Recharge amount should not exceed the current balance
    }

    // Create a string with the amount to recharge, including the current card balance
    String Data = String(amount + GetCardBalance());

    // Prepare data to write
    byte dataToWrite[16] = {0};  // Create a buffer for 16 bytes

    // Convert each character of the string into a byte and store it in the buffer
    for (int i = 0; i < Data.length() && i < 15; i++) {
        dataToWrite[i] = Data[i]; // Write the amount into the byte array
    }

    // Add null terminator at the end of the data
    if (Data.length() < 16) {
        dataToWrite[Data.length()] = '\0';  // Null terminate the string
    }

    // Attempt to write the data to Sector 9, Block 1
    if (!writeDataToBlock(9, 1, dataToWrite)) {
        Serial.println(F("Failed to write amount to card."));
        return false; // Writing failed
    }

    // Update the balance in preferences
    currentBalance -= amount;  // Deduct the recharge amount from the balance
    Config->PutULong64(BALANCE, currentBalance);  // Update balance in preferences

    Serial.println(F("Recharge successful. Amount written to card."));
    return true; // Success
}


/**
 * @brief Retrieves the current balance from Sector 9, Block 1.
 * This function reads the balance stored on the card and returns it as a 64-bit unsigned integer.
 * If the read operation fails, it will return 0.
 * 
 * @return The current balance stored on the card. Returns 0 if the read operation fails.
 */
uint32_t MRC522Manager::GetCardBalance() {
    return cardBalance; // Return the retrieved balance
}


/**
 * @brief Reads data from a specified sector and block on the RFID card.
 * This function will read a block of data (16 bytes) from the card and store
 * it in the provided buffer.
 * 
 * @param sector The sector number to read from.
 * @param block The block number within the sector to read from.
 * @param buffer A pointer to the byte array where the read data will be stored.
 * @return True if the operation was successful, otherwise false.
 */
bool MRC522Manager::readDataFromBlock(byte sector, byte block, byte* buffer) {
    // Check if the card is present
    if (!RFID->PICC_IsNewCardPresent()) {
        Serial.println(F("No card present."));
        return false; // No card present
    }
    
    // Select the card
    if (!RFID->PICC_ReadCardSerial()) {
        Serial.println(F("Failed to read card serial."));
        return false; // Failed to read card serial
    }

    // Calculate the block address
    byte blockAddr = sector * 4 + block; 

    // Authenticate with the card using the default key
    Prepare(ACTIVE_KEY);

    byte status;

    // Authenticate with the default key
    status = RFID->PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockAddr, &keyA, &(RFID->uid));
    if (status != MFRC522::STATUS_OK) {
        Serial.println(F("Authentication with default key failed."));
        return false; // Authentication failed
    }

    // Read data from the block
    byte byteCount; // Declare byteCount to hold the number of bytes read
    status = RFID->MIFARE_Read(blockAddr, buffer, &byteCount);
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("Failed to read data from block. Status: "));
        Serial.println(status);
        return false; // Read failed
    }

    // Stop encryption on the card
//    RFID->PICC_HaltA();
//    RFID->PCD_StopCrypto1();

    return true; // Successful read
}

/**
 * @brief Retrieves the current balance from the device partition.
 * This function checks the stored balance value in preferences.
 * 
 * @return The current balance as a uint64_t value. If the balance is not set, it returns 0.
 */
uint64_t MRC522Manager::GetBalance() {
    // Retrieve the current balance from the configuration manager
    uint64_t currentBalance = Config->GetULong64(BALANCE, 0); // Default to 0 if not set

    // Optionally, print the balance for debugging purposes
    Serial.print(F("Current balance retrieved: "));
    Serial.println(currentBalance);

    return currentBalance; // Return the current balance
}

/**
 * @brief Retrieves the IMEI (International Mobile Equipment Identity) associated with the card reader.
 *
 * This function returns the IMEI as a string, which uniquely identifies the device on the network.
 * IMEI can be used for device-specific tracking or identification in various applications.
 *
 * @return String - A string representing the IMEI number of the card reader.
 */
String MRC522Manager::GetIMEI() {

    return imeiNumber; // return the imei
}

/**
 * @brief Retrieves the MAC address of the ESP32 device as a string.
 *
 * This function returns the MAC address of the ESP32 in a formatted string
 * (e.g., "XX:XX:XX:XX:XX:XX"), which can be used for network identification,
 * device tracking, or any application requiring a unique hardware identifier.
 *
 * @return String - A string representing the MAC address of the ESP32 device.
 */
String MRC522Manager::GetMACAddress() {
    // Retrieve the MAC address of the ESP32
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA); // Read the MAC address for the Wi-Fi interface
    
    // Convert the MAC address to a string format "XX:XX:XX:XX:XX:XX"
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    
    // Return the MAC address as a string
    return String(macStr);
}


/**
 * @brief Retrieves the CPOSID, which is the unique identifier for the Point of Sale (POS) device.
 *
 * This function returns the CPOSID as a string, which can be used for POS identification purposes
 * within a system or network, allowing transactions to be tied back to specific POS devices.
 *
 * @return String - A string representing the unique CPOSID of the POS device.
 */
String MRC522Manager::GetCPOSID() {
    return "NO-SIM"; //  return cpos id
}

void MRC522Manager::resetRFID() {
    // Set the reset pin as output
//    pinMode(RFID_RST_PIN, OUTPUT);

    // Perform reset sequence
    digitalWrite(RFID_RST_PIN, LOW);  // Pull reset pin LOW to reset
    delay(50);                    // Wait for 50 ms
    digitalWrite(RFID_RST_PIN, HIGH); // Set reset pin HIGH to complete reset

    // Optional: Initialize RFID module again if needed
    RFID->PCD_Init();  // Reinitialize RFID module (ensure `rfid` is globally defined)
}


/**
 * @brief Determines if the scanned card is a master card by comparing its UID with a predefined master card ID.
 *        If the card is not the master key, it will try to read the balance and mobile number from the card.
 *        If it is the master card, it will read additional information such as CPOS_ID and IMEI.
 *
 * This function scans a MIFARE Classic card, reads its UID (Unique Identifier), and compares it with a predefined
 * master card ID (`DEFAULT_MASTER_CARD_ID`). It returns a specific status code based on the outcome:
 * - `1` if the UID matches the master card ID (master card detected and additional information read).
 * - `0` if the UID does not match (non-master card).
 * - `3` if no card is detected.
 * - `4` if the card's UID cannot be read.
 * - `5` if the detected card is not a supported MIFARE Classic type.
 * - `6` if authentication fails.
 * - `7` if data read from the card fails.
 *
 * @return uint8_t - Status code:
 *   - `1` for a master card (with CPOS_ID and IMEI read),
 *   - `0` for a non-master card,
 *   - `3` for no card detected,
 *   - `4` for read failure,
 *   - `5` for unsupported card type,
 *   - `6` for authentication failure,
 *   - `7` for read failure.
 */
uint8_t MRC522Manager::IsMasterCard() {
    String uid = "";  // Initialize UID string to empty
    Prepare();  // Prepare the RFID module

    // Check for the presence of a new card
    if (!RFID->PICC_IsNewCardPresent()) {
        cardStatusRead = 3;
        return 3;  // No card detected
    }
    
    // Attempt to read the card's serial number
    if (!RFID->PICC_ReadCardSerial()) {
        cardStatusRead = 4;
        RFID->PCD_Init();
        return 4;  // Failed to read card data
    }

    // Determine the card type and ensure it's a supported MIFARE Classic card
    MFRC522::PICC_Type piccType = RFID->PICC_GetType(RFID->uid.sak);
    if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI
        && piccType != MFRC522::PICC_TYPE_MIFARE_1K
        && piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
        Serial.println(F("This sample only works with MIFARE Classic cards."));
        RFID->PCD_Init();
        cardStatusRead = 5;
        return 5;  // Unsupported card type
    }

    // Format UID bytes as a hexadecimal string with colon separation
    for (byte i = 0; i < RFID->uid.size; i++) {
        if (RFID->uid.uidByte[i] < 0x10) {
            uid += "0";  // Leading zero for single-digit hex values
        }
        uid += String(RFID->uid.uidByte[i], HEX);  // Append uppercase hex byte
        if (i < RFID->uid.size - 1) {
            uid += ":";  // Separate bytes with a colon
        }
    }

    // Check if the UID of the current card matches the predefined master card ID
    if (uid.equalsIgnoreCase(DEFAULT_MASTR_CARD_ID)) {
        Serial.println("IS MASTER CARD");
        // Halt card communication and disable encryption
        RFID->PICC_HaltA();      // Stop communication with the card
        RFID->PCD_StopCrypto1(); // Stop encryption
        cardStatusRead = 1;
        RFID->PICC_HaltA();      // Stop communication with the card
        RFID->PCD_StopCrypto1(); // Stop encryption
        RFID->PCD_Init();
        return 1;  // Master card found and CPOS_ID, IMEI read successfully
    }

    // If not a master card, proceed to read the balance from the card
    byte buffer[18];  // Buffer to store read data
    byte bufferSize = sizeof(buffer);
    byte sectorBlock = BALANCE_SECBLOC;  // Specify block to read for balance data

    // Authenticate the sector with the default key
    
    if (!authenticateWithKeys(BALANCE_AUTH,0)) {
        Serial.println("Authentication failed while reading balance");
        return 6;  // Authentication failed
    }

    // Read data from the specified block
    if (RFID->MIFARE_Read(sectorBlock, buffer, &bufferSize) != MFRC522::STATUS_OK) {
        Serial.println("Read failed while getting balance");
        RFID->PCD_Init();
        cardStatusRead = 7;
        return 7;  // Read failed
    }

    // Convert the byte array to a uint64_t value (e.g., card balance)
    cardBalance = 0;
    String dataString = "";

    // Convert byte array to string until the null terminator (or padding) is encountered
    for (byte i = 0; i < 16; i++) {
        if (buffer[i] == '\0') {
            break; // Stop when the null terminator is encountered
       }
       dataString += (char)buffer[i]; // Convert byte to character
    }

    // Trim any extra spaces or non-numeric characters (optional)
    dataString.trim();

    // Convert the string to a uint64_t value
    if (dataString.length() > 0) {
    cardBalance = strtoull(dataString.c_str(), NULL, 10); // Base 10 conversion
    } else {
        Serial.println(F("No valid data found on card."));
        cardBalance = 0; // Set to 0 if no valid data found
    }

    Serial.print(F("Card Balance: "));
    Serial.println(cardBalance);

    Serial.println("IS NOT MASTER CARD");
    cardStatusRead = 0;
    return 0;  // The card does not match the master card ID, balance reading was successful
}


/**
 * @brief Retrieves the mobile number or ID associated with the device's mobile network module.
 *
 * This function returns the mobile identifier (e.g., phone number or SIM ID) as a string. This 
 * information can be used for network connectivity or mobile-based operations in the system.
 *
 * @return String - A string representing the mobile ID or phone number.
 */
String MRC522Manager::GetMOBILE() {
    
     return "NO-SIM"; //return mobile
}
/**
 * @brief Performs a security check to detect and validate an RFID card.
 *
 * This function prepares the RFID module, checks for the presence of a card,
 * reads its UID, validates it against the master card ID, and determines its type.
 * 
 * @return int
 * - 1: Master card detected and validated.
 * - 3: No card detected.
 * - 4: Failed to read card data.
 * - 5: Unsupported card type.
 */
int MRC522Manager::SecureCheck() {
    String uid = ""; // Initialize UID string to empty
    Prepare();       // Prepare the RFID module

    // Check for the presence of a new card
    if (!RFID->PICC_IsNewCardPresent()) {
        cardStatusRead = 3;
        return 3; // No card detected
    }

    // Attempt to read the card's serial number
    if (!RFID->PICC_ReadCardSerial()) {
        cardStatusRead = 4;
        resetRFID();
        return 4; // Failed to read card data
    }

    // Determine the card type and ensure it's a supported MIFARE Classic card
    MFRC522::PICC_Type piccType = RFID->PICC_GetType(RFID->uid.sak);
    if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
        piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
        piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
        Serial.println(F("This sample only works with MIFARE Classic cards."));
        resetRFID();
        cardStatusRead = 5;
        return 5; // Unsupported card type
    }

    // Format UID bytes as a hexadecimal string with colon separation
    for (byte i = 0; i < RFID->uid.size; i++) {
        if (RFID->uid.uidByte[i] < 0x10) {
            uid += "0"; // Leading zero for single-digit hex values
        }
        uid += String(RFID->uid.uidByte[i], HEX); // Append uppercase hex byte
        if (i < RFID->uid.size - 1) {
            uid += ":"; // Separate bytes with a colon
        }
    }

    // Check if the UID of the current card matches the predefined master card ID
    if (uid.equalsIgnoreCase(DEFAULT_MASTR_CARD_ID)) {
        Serial.println("IS MASTER CARD");
        // Halt card communication and disable encryption
        RFID->PICC_HaltA();      // Stop communication with the card
        RFID->PCD_StopCrypto1(); // Stop encryption
        resetRFID();
        cardStatusRead = 1;
        return 1; // Master card found and validated
    }
    resetRFID();
    return 0; // Card detected but not the master card
}

/**
 * @brief Retrieves the first number associated with the device.
 *
 * This function returns a string representing the first number. It can be utilized 
 * in various network operations as needed.
 *
 * @return String - A string representing the first number.
 */
String MRC522Manager::GetNum01() {
    resetRFID();
    // Read data from Sector 10, Block 0
    
    byte sector = (NUM01_SECBLOC - (NUM01_SECBLOC % 4))/4;
    byte block = NUM01_SECBLOC % 4;

    String num = readDataFromBlock(sector, block);
    
    // If the number is not available or too short, return an empty string
    if (num.length() < 10) {
        return "";
    }

    // Return only the first 10 digits
    return num.substring(0, 10);
}


/**
 * @brief Retrieves the second number associated with the device.
 *
 * This function returns a string representing the second number. It can be utilized 
 * in various network operations as needed.
 *
 * @return String - A string representing the second number.
 */
String MRC522Manager::GetNum02() {
    resetRFID();
    // Read data from Sector 10, Block 0
    byte sector = (NUM02_SECBLOC - (NUM02_SECBLOC % 4))/4;
    byte block = NUM02_SECBLOC % 4;

    String num = readDataFromBlock(sector, block);
    
    // If the number is not available or too short, return an empty string
    if (num.length() < 10) {
        return "";
    }

    // Return only the first 10 digits
    return num.substring(0, 10);
}
/**
 * @brief Retrieves the third number associated with the device.
 *
 * This function returns a string representing the third number. It can be utilized 
 * in various network operations as needed.
 *
 * @return String - A string representing the third number.
 */
String MRC522Manager::GetNum03() {
    resetRFID();
    // Read data from Sector 10, Block 0
    byte sector = (NUM03_SECBLOC - (NUM03_SECBLOC % 4))/4;
    byte block = NUM03_SECBLOC % 4;

    String num = readDataFromBlock(sector, block);
    
    // If the number is not available or too short, return an empty string
    if (num.length() < 10) {
        return "";
    }

    // Return only the first 10 digits
    return num.substring(0, 10);
}

/**
 * @brief Retrieves the fourth number associated with the device.
 *
 * This function returns a string representing the fourth number. It can be utilized 
 * in various network operations as needed.
 *
 * @return String - A string representing the fourth number.
 */
String MRC522Manager::GetNum04() {
    resetRFID();
    // Read data from Sector 10, Block 0
    byte sector = (NUM04_SECBLOC - (NUM04_SECBLOC % 4))/4;
    byte block = NUM04_SECBLOC % 4;

    String num = readDataFromBlock(sector, block);
    
    // If the number is not available or too short, return an empty string
    if (num.length() < 10) {
        return "";
    }

    // Return only the first 10 digits
    return num.substring(0, 10);
}


/**
 * @brief Stores a user-defined number as "Num01" on the NFC card.
 *
 * This function writes the specified string to Sector 10, Block 0 of the NFC card.
 * It allows the secure storage of user data, such as a phone number or other identifying information, 
 * which can later be retrieved or updated as necessary.
 * The data is written as a sequence of bytes, and the success of the write operation is verified.
 *
 * @param Num A string representing the number to be stored on the NFC card.
 *            The string should be 16 characters or fewer, as only the first 16 bytes 
 *            will be written to the card.
 */
bool MRC522Manager::SaveNum01(String Num) {
    byte dataToWrite[16] = {0};  // Buffer to hold the data (16 bytes)
    resetRFID();
    // Convert each character of the string into a byte and store it in the buffer
    for (int i = 0; i < Num.length(); i++) {
        dataToWrite[i] = Num[i];  // Convert each character to a byte
    }

    // Attempt to write the data to Sector 10, Block 0
    if (writeDataToBlock(10, 0, dataToWrite)) {
        Serial.println(F("Number 01 saved successfully."));
        return true;
    } else {
        Serial.println(F("Failed to save Number 01."));
        return false;
    }
}

/**
 * @brief Stores a user-defined number as "Num02" on the NFC card.
 *
 * This function writes the specified string to Sector 10, Block 1 of the NFC card.
 * It provides a method to securely store additional user information, such as a second phone number.
 * The data is written as a sequence of bytes, and the write operation is verified for success.
 *
 * @param Num A string representing the number to be stored on the NFC card.
 *            The string should be 16 characters or fewer, as only the first 16 bytes 
 *            will be written to the card.
 */
bool MRC522Manager::SaveNum02(String Num) {
    byte dataToWrite[16] = {0};  // Buffer to hold the data (16 bytes)
  resetRFID();
    // Convert each character of the string into a byte and store it in the buffer
    for (int i = 0; i < Num.length(); i++) {
        dataToWrite[i] = Num[i];  // Convert each character to a byte
    }

    // Attempt to write the data to Sector 10, Block 1
    if (writeDataToBlock(10, 1, dataToWrite)) {
        Serial.println(F("Number 02 saved successfully."));
        return true;
    } else {
        Serial.println(F("Failed to save Number 02."));
        return false;
    }
}

/**
 * @brief Stores a user-defined number as "Num03" on the NFC card.
 *
 * This function writes the specified string to Sector 11, Block 0 of the NFC card.
 * It allows for the storage of a third user-related number, which can be later accessed or modified.
 * The data is written as a sequence of bytes, with verification to ensure the write operation succeeds.
 *
 * @param Num A string representing the number to be stored on the NFC card.
 *            The string should be 16 characters or fewer, as only the first 16 bytes 
 *            will be written to the card.
 */
bool MRC522Manager::SaveNum03(String Num) {
    byte dataToWrite[16] = {0};  // Buffer to hold the data (16 bytes)
  resetRFID();
    // Convert each character of the string into a byte and store it in the buffer
    for (int i = 0; i < Num.length(); i++) {
        dataToWrite[i] = Num[i];  // Convert each character to a byte
    }

    // Attempt to write the data to Sector 11, Block 0
    if (writeDataToBlock(11, 0, dataToWrite)) {
        Serial.println(F("Number 03 saved successfully."));
        return true;
    } else {
        Serial.println(F("Failed to save Number 03."));
        return false;
    }
}

/**
 * @brief Stores a user-defined number as "Num04" on the NFC card.
 *
 * This function writes the specified string to Sector 11, Block 1 of the NFC card.
 * It allows for the storage of a fourth number, which can be later accessed or updated as necessary.
 * The data is written as a sequence of bytes, and the write operation is verified for success.
 *
 * @param Num A string representing the number to be stored on the NFC card.
 *            The string should be 16 characters or fewer, as only the first 16 bytes 
 *            will be written to the card.
 */
bool MRC522Manager::SaveNum04(String Num) {
    byte dataToWrite[16] = {0};  // Buffer to hold the data (16 bytes)
  resetRFID();
    // Convert each character of the string into a byte and store it in the buffer
    for (int i = 0; i < Num.length(); i++) {
        dataToWrite[i] = Num[i];  // Convert each character to a byte
    }

    // Attempt to write the data to Sector 11, Block 1
    if (writeDataToBlock(11, 1, dataToWrite)) {
        Serial.println(F("Number 04 saved successfully."));
        return true;
    } else {
        Serial.println(F("Failed to save Number 04."));
        return false;
    }
}



/**
 * @brief Reads data from a specified block in a defined sector of the RFID card.
 * 
 * @param sector The sector number to read from (0-15).
 * @param block The block number within the sector (0-3).
 * @return String - The data read from the specified block as a string.
 */
String MRC522Manager::readDataFromBlock(byte sector, byte block) {
    Prepare(ACTIVE_KEY);

    // Check for the presence of a new card
    if (!RFID->PICC_IsNewCardPresent()) {
        return ""; // No card detected
    }

    // Attempt to read the card's serial number
    if (!RFID->PICC_ReadCardSerial()) {
        return ""; // Failed to read card data
    }

    // Determine the card type and ensure it's a supported MIFARE Classic card
    MFRC522::PICC_Type piccType = RFID->PICC_GetType(RFID->uid.sak);
    if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
        piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
        piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
        Serial.println(F("This sample only works with MIFARE Classic cards."));
        return ""; // Unsupported card type
    }

    byte buffer[18];          // Buffer to store read data
    byte bufferSize = sizeof(buffer);
    byte sectorBlock = sector * 4 + block; // Calculate block number based on sector and block

    if (sectorBlock == 40 || sectorBlock == 41) {
        // Authenticate the sector with the default key
        if (!authenticateWithKeys(NUM012_AUTH, 0)) {
            Serial.println("Authentication failed");
            return ""; // Authentication failed
        }
    }

    if (sectorBlock == 44 || sectorBlock == 45) {
        // Authenticate the sector with the default key
        if (!authenticateWithKeys(NUM034_AUTH, 0)) {
            Serial.println("Authentication failed");
            return ""; // Authentication failed
        }
    }

    // Read data from the block
    if (RFID->MIFARE_Read(sectorBlock, buffer, &bufferSize) != MFRC522::STATUS_OK) {
        Serial.println("Read failed");
        return "";
    }

    // Convert read data to a string
    String dataString = "";
    for (byte i = 0; i < 16; i++) {
        dataString += (char)buffer[i]; // Convert byte to character
    }

    return dataString;
}



/**
 * @brief Retrieves all phone numbers stored on the RFID card.
 *
 * This function reads the phone numbers stored on the RFID card in four separate blocks.
 * Each block contains a 10-digit phone number (or less if the data is incomplete).
 * The function returns an array of strings containing up to four phone numbers. 
 * The phone numbers are read from the following blocks:
 * - Sector 10, Block 0
 * - Sector 10, Block 1
 * - Sector 11, Block 0
 * - Sector 11, Block 1
 *
 * The function will return NULL if no card is present, if the card is not supported, or if any reading fails.
 *
 * @return String* - Array containing up to four phone numbers, each with a maximum length of 10 digits.
 */
String* MRC522Manager::GetAllPhoneNumbers() {
    Prepare(ACTIVE_KEY);
    
    // Check for the presence of a new card
    if (!RFID->PICC_IsNewCardPresent()) {
        return NULL;  // No card detected
    }

    // Attempt to read the card's serial number
    if (!RFID->PICC_ReadCardSerial()) {
        return NULL;  // Failed to read card data
    }

    // Determine the card type and ensure it's a supported MIFARE Classic card
    MFRC522::PICC_Type piccType = RFID->PICC_GetType(RFID->uid.sak);
    if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI
        && piccType != MFRC522::PICC_TYPE_MIFARE_1K
        && piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
        Serial.println(F("This sample only works with MIFARE Classic cards."));
        return NULL;  // Unsupported card type
    }

    byte buffer[18]; // Buffer to store read data
    byte bufferSize = sizeof(buffer);
    String* phoneNumbers = new String[4]; // Array to store phone numbers

    // Define the blocks we want to read (Sector 10 Block 0, Sector 10 Block 1, Sector 11 Block 0, Sector 11 Block 1)
    byte blocks[] = {40, 41, 44, 45};  // Blocks: 10:0 -> 0, 10:1 -> 1, 11:0 -> 4, 11:1 -> 5

    // Loop through the specified blocks and read data
    for (byte i = 0; i < 4; i++) {
        byte sectorBlock = blocks[i];  // Get the correct block number

        // Authenticate the sector with the default key
        if (RFID->PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, sectorBlock, &keyA, &(RFID->uid)) != MFRC522::STATUS_OK) {
            Serial.println("Authentication failed while reading");
            delete[] phoneNumbers;
            return NULL;
        }

        // Read data from the block
        if (RFID->MIFARE_Read(sectorBlock, buffer, &bufferSize) != MFRC522::STATUS_OK) {
            Serial.println("Read failed");
            delete[] phoneNumbers;
            return NULL;
        }

        // Convert the block data to string and store it in the array
        String num = "";
        for (byte j = 0; j < 16; j++) {
            num += (char)buffer[j];  // Convert byte to character
        }
        
        // Store only the first 10 digits of the phone number in the array
        phoneNumbers[i] = num.substring(0, 10);
    }

    // Halt card communication and disable encryption on the reader
    //RFID->PICC_HaltA();      // Stop communication with the card
    //RFID->PCD_StopCrypto1(); // Stop encryption
    
    return phoneNumbers;  // Return the array of phone numbers
}
