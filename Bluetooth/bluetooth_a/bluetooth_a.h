// Nicolas Jao - 400450150
// Ayesha Mehmood - 400403482
// Yassin Merdan - 400376619

//Libraries
#include <Arduino.h>
#include <Wire.h>
#include <AsyncAPDS9306.h>

//IIC Addresses for Temperature Sensor
#define ADDR (byte)(0x40)
#define TMP_CMD (byte)(0xF3)

//Sample frequency
#define DELAY_BETWEEN_SAMPLES_MS 5000

//Device information
String group_name = "Group6";
String device_name = "Group6Device";

//Instantiating sensor object and configuration
AsyncAPDS9306 light_sensor;
const APDS9306_ALS_GAIN_t apds_gain = APDS9306_ALS_GAIN_1;
const APDS9306_ALS_MEAS_RES_t apds_time = APDS9306_ALS_MEAS_RES_16BIT_25MS;
