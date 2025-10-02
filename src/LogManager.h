#ifndef LOGMANAGER_H
#define LOGMANAGER_H

#include <ArduinoJson.h>
#include <FS.h>
#include <SPIFFS.h>
#include "TimeManager.h"

class LogManager : public TimeManager {  ///< Inherit from TimeManager
private:
  const char* logFilePath = LOGFILE_PATH; ///< Path for the log file in SPIFFS

  // Helper function to create an empty log structure
  void createLogStructure();

public:
  LogManager();
  ~LogManager();
  void begin();
  bool logFileExists();
  void createLogFile();
  void addLogEntry(const char* action, const char* deviceState, const char* cardId,
                   float amount = 0.0f, float balanceAfterRecharge = 0.0f, const char* storedNumber = "", const char* status = "Success");
  void addRechargeLogEntry(const char* deviceState, const char* cardId, float amount, float balanceAfterRecharge, const char* status);
  void addStoreNumbersLogEntry(const char* cardId, const char* status, const char* storedNumbers[], size_t numStoredNumbers);

};

#endif // LOGMANAGER_H
