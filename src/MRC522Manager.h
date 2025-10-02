#ifndef MRC522MANAGER_H
#define MRC522MANAGER_H

#include <SPI.h>
#include <MFRC522.h>
#include "ConfigManager.h"


/**
 * @class MRC522Manager
 * @brief Class for managing MFRC522 RFID reader operations.
 *
 * This class handles the interaction with the MFRC522 RFID reader, including
 * card reading, writing data, balance management, and configuration settings.
 */
class MRC522Manager {
public:
  
    MRC522Manager(ConfigManager* Config,MFRC522* RFID);                                        ///< Constructor initializing with a ConfigManager reference
    void begin();                                         ///< Initializes the MFRC522 module for operation
    bool readCard();                                      ///< Reads the RFID card and checks if it is present
    String getCardUID();                                  ///< Retrieves the UID of the scanned card for identification
    
    bool writeDataToBlock(byte sector, byte block, byte* data); ///< Writes data to a specified block in a sector of the card
    bool writeDataToBlockHex(byte sector, byte block, byte* data); ///< Writes data to a specified block in a sector of the card in Hex
    String readDataFromBlock(byte sector, byte block);///< read data from a specified block in a sector of the card
    bool lockCard();                           ///< Locks the card by writing protective data to specified blocks
    bool Recharge(uint32_t amount); ///< Recharges the balance and writes it to the card, increasing its available balance
    bool IsCardDetected();                                   ///< Checks if a card is currently detected by the reader
    bool readDataFromBlock(byte sector, byte block, byte* buffer); ///< Reads data from a specified sector and block into a buffer
    
    uint32_t GetCardBalance();                            ///< Retrieves the current balance stored on the card
    uint64_t GetBalance();                                ///< Retrieves the current balance from the device for use in transactions
    String GetIMEI();                                     ///< Retrieves the IMEI number of the device, if applicable
    String GetMACAddress();
    String GetCPOSID();                                   ///< Retrieves the CPOS ID associated with the device
    String GetMOBILE();                                   ///< Retrieves the mobile number or ID associated with the device's network module
 
    uint8_t IsMasterCard();                                  ///< Checks if the card is classified as a master card
    String GetNum01();                                    ///< Retrieves the first stored number from the NFC user card
    String GetNum02();                                    ///< Retrieves the second stored number from the NFC user card
    String GetNum03();                                    ///< Retrieves the third stored number from the NFC user card
    String GetNum04();                                    ///< Retrieves the fourth stored number from the NFC user card
 
    bool SaveNum01(String Num);                           ///< Saves a number to the first storage location on the NFC user card
    bool SaveNum02(String Num);                           ///< Saves a number to the second storage location on the NFC user card
    bool SaveNum03(String Num);                           ///< Saves a number to the third storage location on the NFC user card
    bool SaveNum04(String Num);                           ///< Saves a number to the fourth storage location on the NFC user card
    void Prepare(const byte* customKey = nullptr) ;
    bool SecureTag(byte *userKey, byte *userACK);
    void printHex(byte *buffer, byte bufferSize);
    void printDec(byte *buffer, byte bufferSize);
    int SecureCheck();
    String* GetAllPhoneNumbers();
    uint8_t cardStatusRead;
    void resetRFID();

    private:
    MFRC522* RFID;                                       ///< Create an instance of the MFRC522 class to handle RFID operations
    MFRC522::MIFARE_Key keyA;                            ///< Default key for authentication with the RFID card
    ConfigManager* Config;                               ///< Pointer to the ConfigManager for accessing configuration settings
    bool writePage(byte page, byte *data, byte len);
    // Variables for storing card-related information
    uint64_t cardBalance;    ///< The current balance stored on the RFID card
    String mobileNumber;     ///< The mobile number associated with the master card
    String imeiNumber;       ///< The IMEI number associated with the device
    String cposID;           ///< The CPOS ID associated with the card
    bool authenticateWithKeys(byte sector, byte block);///< Authentificate card with 2 keys
    // Define Key A and Key B for authentication
    byte keyAuthA[6] = AUTH_KEY_A; ///< Key A AuthKey
    byte keyAuthB[6] = AUTH_KEY_B; ///< Key B AuthKey
    };

#endif // MRC522MANAGER_H

