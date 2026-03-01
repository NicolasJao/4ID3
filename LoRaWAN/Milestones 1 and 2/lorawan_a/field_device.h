// Nicolas Jao			400450151 
// Ayesha Mehmood		400403482
// Yassin Merdan		400376619
// Hans Qin				  400461329

// Libraries
#include <TheThingsNetwork.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

//  Macros
#define DHTPIN 2
#define DHTTYPE DHT11
#define POLLING_PERIOD 2000
#define GROUP_NAME "Group6"
#define DEVICE_NAME "Device6"

DHT_Unified dht(DHTPIN, DHTTYPE);
#define lora_serial Serial1
#define debug_serial Serial

// Set your AppEUI and AppKey
#define FREQUENCY_PLAN TTN_FP_US915
const char *app_eui = "0000000000000000";
const char *app_key = "0FBBEE93DCFFD8B8F78F8C55452F7704";
TheThingsNetwork ttn(lora_serial, debug_serial, FREQUENCY_PLAN);

unsigned long start_time = millis();
