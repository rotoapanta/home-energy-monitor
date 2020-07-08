
#ifndef TASK_ARDUINO_OTA
#define TASK_ARDUINO_OTA

#if OTA_ENABLED == true
    #include <WiFi.h>
    #include <ESPmDNS.h>
    #include <WiFiUdp.h>
    #include <ArduinoOTA.h>
    #include "../config/enums.h"
    #include "../config/config.h"

    /**
     * Task: Whenever Wifi is connected, start the Arduino OTA.
     */
    void enableOTA(void * parameter){
        //Run this method only when the Wifi Connected
        if(!WiFi.isConnected()){
            serial_println("[OTA] WIFI Not Connected, can not initialize OTA..");            
            vTaskDelete(NULL);
        }
        serial_println("[OTA] Wifi Connected, initialize OTA...");
        // Hostname defaults to esp3232-[MAC]
        ArduinoOTA.setHostname( DEVICE_NAME );

        // No authentication by default
        // ArduinoOTA.setPassword("admin");

        ArduinoOTA
            .onStart([]() {
            String type;
            if (ArduinoOTA.getCommand() == U_FLASH)
                type = "sketch";
            else // U_SPIFFS
                type = "filesystem";

            // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
            serial_println("[OTA] Start updating " + type);
            })
            .onEnd([]() {
            serial_println("\n[OTA] End");
            })
            .onProgress([](unsigned int progress, unsigned int total) {
            Serial.printf("[OTA] Progress: %u%%\r", (progress / (total / 100)));
            })
            .onError([](ota_error_t error) {
            Serial.printf("[OTA] Error[%u]: ", error);
            if (error == OTA_AUTH_ERROR) Serial.println("[OTA] Auth Failed");
            else if (error == OTA_BEGIN_ERROR) Serial.println("[OTA] Begin Failed");
            else if (error == OTA_CONNECT_ERROR) Serial.println("[OTA] Connect Failed");
            else if (error == OTA_RECEIVE_ERROR) Serial.println("[OTA] Receive Failed");
            else if (error == OTA_END_ERROR) Serial.println("[OTA] End Failed");
            });

        ArduinoOTA.begin();
        serial_println(F("[OTA] Initialized and connected."));
        
        // Task is done!, TODO: Do we really need to delete it here for OTA task?
        vTaskDelete(NULL);
                    
    }

    /**
     * Every 5 sec run the OTA Handler
     */
    void runOTAHandler(void * parameter){
        for(;;){
            
            if(!WiFi.isConnected()){
                vTaskDelay(1000 / portTICK_PERIOD_MS);
                continue;
            }
            serial_println("[OTA] handle..");
            ArduinoOTA.handle();
            vTaskDelay(5000 / portTICK_PERIOD_MS);
        }
    }




#endif
#endif


