#ifndef BUZZERMANAGER_H
#define BUZZERMANAGER_H

#include "ConfigManager.h"

class BuzzerManager {
public:
    BuzzerManager(uint8_t pin);
    void begin();
    void playSuccessTone();
    void playFailureTone();

private:
    uint8_t _pin;
};

#endif // BUZZERMANAGER_H
