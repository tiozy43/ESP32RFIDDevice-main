#include "BuzzerManager.h"

/**
 * @brief Constructs a BuzzerManager object and initializes the buzzer pin.
 * @param pin The pin connected to the buzzer.
 */
BuzzerManager::BuzzerManager(uint8_t pin) : _pin(pin) {}

/**
 * @brief Initializes the buzzer by configuring the pin as output.
 *        Plays a short initialization tone to confirm setup.
 */
void BuzzerManager::begin() {
    pinMode(_pin, OUTPUT); // Set the pin as output
    tone(_pin, 0, 1);      // Short initialization tone (0 Hz, 1 ms)
    noTone(_pin);          // Ensure the tone is stopped
}

/**
 * @brief Plays a short, high-pitched tone to indicate success.
 */
void BuzzerManager::playSuccessTone() {
    tone(_pin, 1000, 50);  // Play a 1000 Hz tone for 50 ms
    noTone(_pin);          // Stop the tone
}

/**
 * @brief Plays a repeated, low-pitched tone to indicate failure.
 *        The tone plays three times with delays between each beep.
 */
void BuzzerManager::playFailureTone() {
    for (int i = 0; i < 3; i++) {
        tone(_pin, 500, 50); // Play a 500 Hz tone for 50 ms
        noTone(_pin);        // Stop the tone
        delay(110);          // Wait between tones
    }
}
