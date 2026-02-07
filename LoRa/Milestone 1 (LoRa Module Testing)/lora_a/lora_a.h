// Nicolas Jao - 400450150
// Ayesha Mehmood - 400403482
// Yassin Merdan - 400376619
// Hans Qin - 400461329

//Libraries
#include "EBYTE.h"
#include <HardwareSerial.h>

#define PIN_RX 16   // Serial2 RX -> EBYTE TX
#define PIN_TX 17   // Serial2 TX pin -> EBYTE RX
#define PIN_M0 27 
#define PIN_M1 26
#define PIN_AX 25
#define Serial_1 Serial2

#define TIME_PER_SEND_MS 5000
unsigned long startTime = millis();

//Transceiver frequency
#define TRANSCEIVER_CHANNEL 6 // Channel must be 0 - 64
EBYTE Transceiver(&Serial2, PIN_M0, PIN_M1, PIN_AX);

//Device details
String group_name = "Group6";
String device_name = "DeviceA";
