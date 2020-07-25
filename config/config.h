#ifndef CONFIG
#define CONFIG


/**
 * Set this to false to disable Serial logging
 */
#define DEBUG true

/**
 * The name of this device (as defined in the AWS IOT console).
 * Also used to set the hostname on the network
 */
#define DEVICE_NAME "hkv-home-energy-monitor"

/**
 * ADC input pin that is used to read out the Voltage Sensor (ZMPT101B)
 */
#define VOL_ADC_INPUT 34

/**
 * ADC input pin that is used to read out the CT sensor
 */
#define CUR_ADC_INPUT 35

/**
 * The voltage of your home, used to calculate the wattage.
 * Try setting this as accurately as possible.
 * Hari: We won't be using this.
 */
#define HOME_VOLTAGE 245.0

/**
 * WiFi credentials
 */
#define WIFI_NETWORK "MyLinksys"
#define WIFI_PASSWORD "9876543210"

/**
 * Timeout for the WiFi connection. When this is reached,
 * the ESP goes into deep sleep for 30seconds to try and
 * recover.
 */
#define WIFI_TIMEOUT 20000 // 20 seconds

/**
 * How long should we wait after a failed WiFi connection
 * before trying to set one up again.
 */
#define WIFI_RECOVER_TIME_MS 20000 // 20 seconds

/**
 * Dimensions of the OLED display attached to the ESP
 */
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

/**
 * Force Emonlib to assume a 3.3V supply to the CT sensor
 */
#define emonTxV3 1


/**
 * Local measurements
 */
#define LOCAL_MEASUREMENTS 5


/**
 * The MQTT endpoint of the service we should connect to and receive messages
 * from.
 */
#define AWS_ENABLED false
#define AWS_IOT_ENDPOINT "**** YOUR AWS IOT ENDPOINT ****"
#define AWS_IOT_TOPIC "**** YOUR AWS IOT RULE ARN ****"

#define MQTT_CONNECT_DELAY 200
#define MQTT_CONNECT_TIMEOUT 20000 // 20 seconds


/**
 * Syncing time with an NTP server
 */
#define NTP_TIME_SYNC_ENABLED true
#define NTP_SERVER "pool.ntp.org"
//Offset is UTC+5:30
#define NTP_OFFSET_SECONDS 19800
#define NTP_UPDATE_INTERVAL_MS 60000

/**
 * Wether or not you want to enable Home Assistant integration
 */
#define HA_ENABLED true
#define HA_ADDRESS "ha.harivemula.com"
#define HA_PORT 1883
#define HA_USER "hemqttrw"
#define HA_PASSWORD "hemqttrw"

/**
 * Enable Over The Air Updates on ESP32
 */
#define OTA_ENABLED true
 

// Check which core Arduino is running on. This is done because updating the 
// display only works from the Arduino core.
#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif


#endif