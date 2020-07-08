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
extern double measurements[];
extern double measurements_ap[];
extern double measurements_v[];
extern double measurements_a[];
extern double measurements_pf[];

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
  //    emon1.serialprint();
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
 //     Serial.print("[DEBUG] measure Index:");
 //     Serial.println(measureIndex);
      if(measureIndex == LOCAL_MEASUREMENTS){
   //       serial_println("[DEBUG] local measurement is now reached");
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
  //        serial_println("[DEBUG] sent values to HA");
          measureIndex = 0;
      }


      long end = millis();
/*      serial_print("[DEBUG] at the end of the measurement-1:start:");
      serial_print(start);
      serial_print(", end:");
      serial_print(end);
      serial_print(", remaining:");
      serial_print((1000-(end-start)));
      serial_print(", portTickperiod:");
      serial_print(portTICK_PERIOD_MS);
      serial_print(", delay:");
      serial_println((1000-(end-start)) / portTICK_PERIOD_MS);
*/
      // Schedule the task to run again in 1 second (while
      // taking into account how long measurement took)
      if ((1000-(end-start)) < 0) {
        //Incase the reading failed/timed out, the delay goes very high. so waiting only for 2sec
        vTaskDelay(2000 / portTICK_PERIOD_MS);
      } else {
        vTaskDelay((1000-(end-start)) / portTICK_PERIOD_MS);
      }
      
      //serial_println("[DEBUG] at the end of the measurement-2");
    }    
}

#endif
