#include "LogManager.h"

/**
 * @brief Constructor for the LogManager class.
 * 
 * Initializes any necessary parameters or resources for logging.
 */
LogManager::LogManager() {
  // Constructor body (could initialize anything related to LogManager here)
}

/**
 * @brief Destructor for the LogManager class.
 * 
 * Cleans up any allocated resources (if any) when the LogManager object is destroyed.
 */
LogManager::~LogManager() {}

/**
 * @brief Initializes the SPIFFS file system and checks if the log file exists.
 * 
 * If the log file does not exist, it creates a new one with an empty structure.
 */
void LogManager::begin() {
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS initialization failed!");
    return;
  }

  if (!logFileExists()) {
    createLogFile();
  }
}

/**
 * @brief Checks if the log file exists in the SPIFFS file system.
 * 
 * @return true if the log file exists, false otherwise.
 */
bool LogManager::logFileExists() {
  return SPIFFS.exists(logFilePath);
}

/**
 * @brief Creates a new log file with an empty structure if the log file does not already exist.
 * 
 * This function opens a new log file in write mode, creates an initial empty log structure, 
 * and then closes the file.
 */
void LogManager::createLogFile() {
  File logFile = SPIFFS.open(logFilePath, FILE_WRITE);
  if (!logFile) {
    Serial.println("Failed to create log file!");
    return;
  }

  // Create the initial log structure
  createLogStructure();
  logFile.close();
}

/**
 * @brief Helper function to create an empty log structure in the log file.
 * 
 * This function initializes the JSON structure with an empty array of log entries.
 */
void LogManager::createLogStructure() {
  StaticJsonDocument<512> doc;
  JsonArray logEntries = doc.createNestedArray("logEntries");
  
  File logFile = SPIFFS.open(logFilePath, FILE_WRITE);
  if (logFile) {
    serializeJson(doc, logFile);
    logFile.close();
  }
}

/**
 * @brief Adds a new log entry to the JSON log file.
 * 
 * This function adds a new log entry containing details like timestamp, action, device state,
 * card ID, and other relevant information. If the log file exists, it appends the entry; otherwise,
 * it creates a new log file.
 * 
 * @param action The action performed (e.g., "Recharge").
 * @param deviceState The current state of the device (e.g., "unlocked").
 * @param cardId The ID of the RFID card involved in the action.
 * @param amount The amount (used for recharge or other actions).
 * @param balanceAfterRecharge The balance after a recharge (if applicable).
 * @param storedNumber A number stored in the RFID card (if applicable).
 * @param status The status of the action (e.g., "Success").
 */
void LogManager::addLogEntry(const char* action, const char* deviceState, const char* cardId,
                             float amount, float balanceAfterRecharge, const char* storedNumber, const char* status) {
  
  // Get the current timestamp from inherited TimeManager methods
  String timestamp = getDateString() + " " + getTimeString();

  // Prepare the log entry
  StaticJsonDocument<512> doc;
  JsonObject entry = doc.createNestedObject();
  entry["timestamp"] = timestamp;
  entry["device state"] = deviceState;
  entry["action"] = action;
  entry["cardId"] = cardId;
  entry["status"] = status;

  if (amount > 0) entry["amount"] = amount;
  if (balanceAfterRecharge > 0) entry["balanceAfterRecharge"] = balanceAfterRecharge;
  if (strlen(storedNumber) > 0) entry["storedNumber"] = storedNumber;

  // Open the log file and append the new entry
  File logFile = SPIFFS.open(logFilePath, FILE_APPEND);
  if (logFile) {
    StaticJsonDocument<512> currentDoc;
    DeserializationError error = deserializeJson(currentDoc, logFile);
    
    if (error) {
      Serial.println("Failed to read log file!");
      return;
    }
    
    JsonArray logEntries = currentDoc["logEntries"];
    logEntries.add(entry);

    logFile.close();

    // Re-open the file to overwrite the updated log data
    logFile = SPIFFS.open(logFilePath, FILE_WRITE);
    if (logFile) {
      serializeJson(currentDoc, logFile);
      logFile.close();
    }
  }
}

// Variation: Add a "Recharge" log entry
void LogManager::addRechargeLogEntry(const char* deviceState, const char* cardId, float amount, float balanceAfterRecharge, const char* status) {
  addLogEntry("Recharge", deviceState, cardId, amount, balanceAfterRecharge, "", status);
}

// Variation: Add a "Store Numbers" log entry
void LogManager::addStoreNumbersLogEntry(const char* cardId, const char* status, const char* storedNumbers[], size_t numStoredNumbers) {
  // Prepare the log entry with specific action and cardId
  String timestamp = getDateString() + " " + getTimeString();
  
  StaticJsonDocument<512> doc;
  JsonObject entry = doc.createNestedObject();
  entry["timestamp"] = timestamp;
  entry["action"] = "Store Numbers";
  entry["cardId"] = cardId;
  entry["status"] = status;

  // Add the stored numbers array
  JsonArray storedNumbersArray = entry.createNestedArray("storedNumbers");
  for (size_t i = 0; i < numStoredNumbers; ++i) {
    storedNumbersArray.add(storedNumbers[i]);
  }

  // Open the log file and append the new entry
  File logFile = SPIFFS.open(logFilePath, FILE_APPEND);
  if (logFile) {
    StaticJsonDocument<512> currentDoc;
    DeserializationError error = deserializeJson(currentDoc, logFile);
    
    if (error) {
      Serial.println("Failed to read log file!");
      return;
    }
    
    JsonArray logEntries = currentDoc["logEntries"];
    logEntries.add(entry);

    logFile.close();

    // Re-open the file to overwrite the updated log data
    logFile = SPIFFS.open(logFilePath, FILE_WRITE);
    if (logFile) {
      serializeJson(currentDoc, logFile);
      logFile.close();
    }
  }
}