#ifndef SCREENMANAGER_H
#define SCREENMANAGER_H


#include <KeyPad.h>
#include "WiFiManager.h"
#include "LogManager.h"
#include "MRC522Manager.h"
#include "SPI.h"
#include <LiquidCrystal_I2C.h>
#include "BuzzerManager.h"


class ScreenManager {
public:
    // Constructor
    ScreenManager(WiFiManager* wiFiManager,LogManager* Log,MRC522Manager* mRC522Manager , LiquidCrystal_I2C* LCD,BuzzerManager* Buzz);

    // Initialization function
    void begin();

    // Screen control functions
    void clearScreen(); // Clear the screen with a specified color
    void displayText(const char* text, int x, int y); // Display text at specified coordinates
    void displayCenteredText(const char* text,int y); // Display centered text horizontally
    
    // New functions for success and failure messages
    void displaySuccessMessage(const char* message, int blinkCount, int row);
    void displayFailureMessage(const char* message, int blinkCount, int row);
    void eraseCharacter(int x, int y);

    String ShowPrompt(const char* Text,const char* Num);// return the String of what have been type in the porompt
    bool confirmInput(const char* Text);// confirm the user input
    bool confirmRecharge(uint16_t amount);// confrm user input recharg
    void displayWiFiSignal();
    void LockCardPage() ;
    void HomePage(const char* nameTop);
    void RechargePage();
    void MasterMode();
    void UserMode();
    void SecurityCheck();
    void GotoApPage();
    char SelectAction();
    String GetLastRechergAmount();
    char Kharacter = NO_KEY;
    void scrollTextOnLine(String text, int startX, int stopX, int line);
private:
    LiquidCrystal_I2C* LCD; // LCD instance
    WiFiManager* wiFiManager;
    LogManager* Log;
    MRC522Manager* mRC522Manager;
    BuzzerManager* Buzz;
    uint16_t LastAmount;
};

#endif // SCREENMANAGER_H
