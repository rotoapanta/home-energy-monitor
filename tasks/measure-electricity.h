#ifndef TASK_MEASURE_ELECTRICITY
#define TASK_MEASURE_ELECTRICITY

#include <Arduino.h>
#include "EmonLib.h"

#include "../config/config.h"
#include "../config/enums.h"
#include "mqtt-aws.h"
#include "mqtt-home-assistant.h"

extern DisplayValues gDisplayValues;
extern EnergyMonitor emon1;
extern unsigned short measurements[];
extern unsigned short measurements_ap[];
extern unsigned short measurements_v[];
extern unsigned short measurements_a[];
extern unsigned short measurements_pf[];

extern unsigned char measureIndex;

void measureElectricity(void * parameter)
{
    for(;;){
      serial_println("[ENERGY] Measuring...");
      long start = millis();
      
      emon1.calcVI(20,2000);

      double realPower       = emon1.realPower;        //extract Real Power into variable
      double apparentPower   = emon1.apparentPower;    //extract Apparent Power into variable
      double powerFactor     = emon1.powerFactor;      //extract Power Factor into Variable
      double supplyVoltage   = emon1.Vrms;             //extract Vrms into Variable
      double amps            = emon1.Irms;             //extract Irms into Variable

      serial_println("[ENERGY] measured values");

      //double amps = emon1.calcIrms(1480);
      //double watts = amps * HOME_VOLTAGE;

      gDisplayValues.amps = amps;
      gDisplayValues.watt = realPower;

      measurements[measureIndex] = realPower;
      measurements_ap[measureIndex] = apparentPower;
      measurements_v[measureIndex] = supplyVoltage;
      measurements_a[measureIndex] = amps;
      measurements_pf[measureIndex] = powerFactor;

      measureIndex++;
      Serial.print("[DEBUG] measure Index:");
      Serial.println(measureIndex);
      if(measureIndex == LOCAL_MEASUREMENTS){
          serial_println("[DEBUG] local measurement is now 30");
          #if AWS_ENABLED == true
            xTaskCreate(
              uploadMeasurementsToAWS,
              "Upload measurements to AWS",
              10000,             // Stack size (bytes)
              NULL,             // Parameter
              5,                // Task priority
              NULL              // Task handle
            );
          #endif

          #if HA_ENABLED == true
            xTaskCreate(
              sendEnergyToHA,
              "HA-MQTT Upload",
              10000,             // Stack size (bytes)
              NULL,             // Parameter
              5,                // Task priority
              NULL              // Task handle
            );
          #endif
                serial_println("[DEBUG] sent values to HA");
      measureIndex = 0;
      }


      long end = millis();
      serial_println("[DEBUG] at the end of the measurement-1");
      // Schedule the task to run again in 1 second (while
      // taking into account how long measurement took)
      vTaskDelay((1000-(end-start)) / portTICK_PERIOD_MS);
      serial_println("[DEBUG] at the end of the measurement-2");
    }    
}

#endif
