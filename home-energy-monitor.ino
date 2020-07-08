#include <SPI.h>
#include <WiFi.h>
//below are for OTA
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
//end for OTA
#include "EmonLib-esp32/EmonLib.h"
//This is from https://github.com/Savjee/EmonLib-esp32 (ADC BITS changed)
#include <Arduino.h>
#include <driver/adc.h>
#include "config/config.h"
#include "config/enums.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "tasks/updateDisplay.h"
#include "tasks/fetch-time-from-ntp.h"
#include "tasks/mqtt-aws.h"
#include "tasks/wifi-connection.h"
#include "tasks/wifi-update-signalstrength.h"
#include "tasks/measure-electricity.h"
#include "tasks/mqtt-home-assistant.h"
#include "tasks/arduino-esp32-ota.h"


/**
 *  Firmware Version. Update the version on every change.
 * 
 *  Change History:
 *    1.0.0 - Basic Measurement was working, included OTA.
 *    1.0.1 - When measuring current fails, it is waiting for very long for next measurement. 
 *            So made to wait only 2sec incase of failure.
 *    1.0.2 - Made the local measurement array as double from unsigned short.
 *    1.0.3 - Calibration values changed.
 * 
 * 
 */
#define FIRMWARE_VERSION "1.0.3"




Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
DisplayValues gDisplayValues;
EnergyMonitor emon1;

// Place to store local measurements before sending them off to AWS & HA
double measurements[LOCAL_MEASUREMENTS];
double measurements_ap[LOCAL_MEASUREMENTS];
double measurements_v[LOCAL_MEASUREMENTS];
double measurements_a[LOCAL_MEASUREMENTS];
double measurements_pf[LOCAL_MEASUREMENTS];
unsigned char measureIndex = 0;


void setup()
{
  #if DEBUG == true
    Serial.begin(115200);
  #endif 
    serial_println("Booting");
    serial_print ("-------------Firmware Version:");
    serial_println(FIRMWARE_VERSION);

  // Setup the ADC
  //https://github.com/espressif/arduino-esp32
  adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11);
  adc1_config_channel_atten(ADC1_CHANNEL_7, ADC_ATTEN_DB_11);
  analogReadResolution(ADC_BITS);
  pinMode(VOL_ADC_INPUT, INPUT);
  pinMode(CUR_ADC_INPUT, INPUT);

  // i2c for the OLED panel
 // Wire.begin(5, 4); //commented by Hari (externally connected OLED doesn't work with this if connected using SDA & SCL)

  // Initialize the display  SSD1306_SWITCHCAPVCC
  //!display.begin(SSD1306_SWITCHCAPVCC, 0x3C,false,false), //SSD1306_EXTERNALVCC
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    serial_println(F("SSD1306 allocation failed"));
    delay(10*1000);
    ESP.restart();
  }

  // Init the display
  display.clearDisplay();
  display.setRotation(3);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setTextWrap(false);

  // Initialize emon library
  //emon1.current(ADC_INPUT, 30);

  emon1.voltage(VOL_ADC_INPUT, 330.30, 1.7);  // Voltage: input pin, calibration, phase_shift, Changing from 234.36 -> 330
  emon1.current(CUR_ADC_INPUT, 95.6);  // Current: Input Pin, Calibration, Changing from 90.9 to 95.6

  // ----------------------------------------------------------------
  // TASK: Connect to WiFi & keep the connection alive.
  // ----------------------------------------------------------------
  xTaskCreatePinnedToCore(
    keepWiFiAlive,
    "keepWiFiAlive",  // Task name
    5000,            // Stack size (bytes)
    NULL,             // Parameter
    1,                // Task priority
    NULL,             // Task handle
    ARDUINO_RUNNING_CORE
  );

  // ----------------------------------------------------------------
  // TASK: Connect to AWS & keep the connection alive.
  // ----------------------------------------------------------------
  #if AWS_ENABLED == true
    xTaskCreate(
      keepAWSConnectionAlive,
      "MQTT-AWS",      // Task name
      5000,            // Stack size (bytes)
      NULL,             // Parameter
      5,                // Task priority
      NULL              // Task handle
    );
  #endif

  // ----------------------------------------------------------------
  // TASK: Update the display every second
  //       This is pinned to the same core as Arduino
  //       because it would otherwise corrupt the OLED
  // ----------------------------------------------------------------
  xTaskCreatePinnedToCore(
    updateDisplay,
    "UpdateDisplay",  // Task name
    10000,            // Stack size (bytes)
    NULL,             // Parameter
    3,                // Task priority
    NULL,             // Task handle
    ARDUINO_RUNNING_CORE
  );

  // ----------------------------------------------------------------
  // Task: measure electricity consumption ;)
  // ----------------------------------------------------------------
  xTaskCreate(
    measureElectricity,
    "Measure electricity",  // Task name
    5000,                  // Stack size (bytes)
    NULL,                   // Parameter
    4,                      // Task priority
    NULL                    // Task handle
  );

  // ----------------------------------------------------------------
  // TASK: update time from NTP server.
  // ----------------------------------------------------------------
  #if NTP_TIME_SYNC_ENABLED == true
    xTaskCreate(
      fetchTimeFromNTP,
      "Update NTP time",
      5000,            // Stack size (bytes)
      NULL,             // Parameter
      2,                // Task priority // hari changed to 2
      NULL              // Task handle
    );
  #endif

  // ----------------------------------------------------------------
  // TASK: update WiFi signal strength
  // ----------------------------------------------------------------
  xTaskCreate(
    updateWiFiSignalStrength,
    "Update WiFi strength",
    1000,             // Stack size (bytes)
    NULL,             // Parameter
    2,                // Task priority
    NULL              // Task handle
  );

  #if HA_ENABLED == true
    xTaskCreate(
      HADiscovery,
      "MQTT-HA Discovery",  // Task name
      5000,                // Stack size (bytes)
      NULL,                 // Parameter
      5,                    // Task priority
      NULL                  // Task handle
    );

    xTaskCreate(
      keepHAConnectionAlive,
      "MQTT-HA Connect",
      5000,
      NULL,
      4,
      NULL
    );
  #endif

  //Arduino OTA task
  #if OTA_ENABLED == true
    xTaskCreate(
      runOTAHandler,
      "OTA Handler",  // Task name
      5000,                // Stack size (bytes)
      NULL,                 // Parameter
      4,                    // Task priority
      NULL                  // Task handle
    );
  #endif
}

void loop()
{

  vTaskDelay(10000 / portTICK_PERIOD_MS);
}
