/*
Beer Liquor/Mash/Boil Temperature Logger
See exelent tutorial: https://tttapa.github.io/ESP8266/Chap01%20-%20ESP8266.html
*/

#include "configuration.h"
#include "tempSensor.h"
#include "wifi.h"
#include "SPIFFS.h"
#include "webServer.h"
#include "display.h"
#include "ntp.h"
#include "thingspeak.h"



void setup() {
  // start serial port
  Serial.begin(115200);
  Serial.println("Starting");
  Serial.println("Serial set to 115200");

  // Display a ap for configure wifi connection
  if (WIFI_MODE == "MANAGER") {
    startWifiManager();
  }
  // Join a preconfigured wifi
  if (WIFI_MODE == "JOIN") {
    startWifi();
  }

  // Start ntp sync
  ntpStart();

  // Start Filesystem
  startSPIFFS();

  // Configure Web Server
  webServerStart();
  
  // Start & configure sensors
  tempSensorStart();         
  // Start & configure display                              
  if (DISPLAY_IC2_ENABLE == true) { displayStart(); }     
}


time_t lastTempUpdate = 0;                    // Last time to get temp 
time_t lastLocalLoggerUpdate = 0;             // Last time to white temp in local logger
time_t lastLocalLoggerLastRecordNumber = 0;   // Last time to white temp in local logger
time_t lastThingspeakUpdate = 0;              // Last time to white temp in local logger
boolean record = true;                        // Write in local logger

void loop() {
  // update Clock
  time_t t = now();

  // Get sensor temp & show in display
  if ( now() > (lastTempUpdate + GET_TEMP_EVERY_X_SECONDS )) {
      Serial.print(t);
      Serial.print(" ");
      // Get temperature from sensors
      getTempFromSensors();
      // Show Sensors temps in IC2 Display
      if (DISPLAY_IC2_ENABLE == true) { displayTemperatures();}  
      lastTempUpdate = t;
  }

  t = now();
  // Record temp In file
  if (record == true) {
    if (lastLocalLoggerLastRecordNumber < LOCAL_LOGGER_MAX_RECORDS_X_FILE) {
      if ( t > (lastLocalLoggerUpdate + LOCAL_LOGGER_RECORD_TEMP_EVERY_X_SECONDS )) {
          Serial.print("Write Local LOG...");
          File tempLog = SPIFFS.open("/localLogger/1.csv", "a"); // Write the time and the temperature to the csv file
          tempLog.print(year(t));
          tempLog.print("-");
          tempLog.print(month(t));
          tempLog.print("-");
          tempLog.print(day(t));
          tempLog.print(',');
          tempLog.print(hour(t));
          tempLog.print(":");
          tempLog.print(minute(t));
          tempLog.print(":");
          tempLog.print(second(t));
          tempLog.print(',');
          tempLog.print(String(tempLiquor));
          tempLog.print(',');
          tempLog.print(String(tempMash));        
          tempLog.print(',');
          tempLog.println(String(tempBoil));        
          tempLog.close();
          Serial.println("done");
          lastLocalLoggerUpdate = t;
          lastLocalLoggerLastRecordNumber = lastLocalLoggerLastRecordNumber + 1;
      }
    } else {
      Serial.println("File limit reached");
    }
  }

  t = now();
  // Record on thingspeak
  if (THINGSPEAK_ENABLE == true) {
    if (record == true) {
        if ( t > (lastThingspeakUpdate + THINGSPEAK_UPDATE_EVERY_X_SECONDS)) {
              thingspeakUpdate();
              lastThingspeakUpdate = t;
        }
    }
  }

  // Webserver
  server.handleClient();
}
