// Nicolas Jao - 400450150
// Ayesha Mehmood - 400403482
// Yassin Merdan - 400376619
// Hans Qin - 400461329

#include "field_device.h"

// ---- ALARM THRESHOLDS ----
#define TEMP_THRESHOLD_HIGH      35.0    // Celsius
#define TEMP_THRESHOLD_LOW       10.0    // Celsius
#define HUMIDITY_THRESHOLD_HIGH  70.0    // % RH
#define HUMIDITY_THRESHOLD_LOW   20.0    // % RH
#define VIBRATION_THRESHOLD_HIGH 3000    // analog signal

// ---- ALARM OUTPUT PINS ----
#define BUZZER_PIN    12
#define LED_1_PIN     14
#define LED_2_PIN     13

// ---- ALARM PARAMETERS ----
#define ALARM_DELAY   50
#define ALARM_FREQUENCY 700

// ---- OTHER INPUT PINS ----
#define VIBRATION_PIN 36
/*
#define LINE_1_DEMAND_SPIKE 
#define LINE_3_DEMAND_SPIKE
#define LINE_4_S1_ALIVE
#define LINE_4_S2_ALIVE
#define LINE_4_S3_ALIVE
#define LINE_4_S4_ALIVE
*/

// ---- ALARM MILLIS TRACKING ----
unsigned long previousAlarmMillis = 0;
int alarmState = 0;  // 0 = buzzer, 1 = LED1, 2 = LED2
bool alarmActive = false;

void setup() {
  Serial.begin(9600);
  Serial.print("\n\n------------------------\n"
    + group_name + " : " + device_name + "\n------------------------\n\n"); 
  
  Wire.begin();
  Wire.beginTransmission(ADDR);
  Wire.endTransmission();
  delay(300);

  Serial.println("Ready for LoRa connection!");
  
  Serial_0.begin(9600, SERIAL_8N1, 16, 17);
  Transceiver.init();
  Transceiver.SetAddressH(1);
  Transceiver.SetAddressL(1);
  Transceiver.SetAirDataRate(ADR_9600);
  Transceiver.SetChannel(TRANSCEIVER_CHANNEL);
  Transceiver.SetMode(EBYTE_MODE_NORMAL);
  Transceiver.SetTransmitPower(OPT_TP20);
  Transceiver.SaveParameters(PERMANENT);
  Transceiver.PrintParameters();
  
  // Initializing IO port for MCP23017 IO expansion bus
  mcp.init();
  mcp.portMode(MCP23017Port::A, 0); // Configuring port A as OUTPUT
  mcp.writeRegister(MCP23017Register::GPIO_A, 0x00);  // Resetting port A

  // Alarm pin setup
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_1_PIN, OUTPUT);
  pinMode(LED_2_PIN, OUTPUT);
  pinMode(VIBRATION_PIN, INPUT);
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(LED_1_PIN, LOW);
  digitalWrite(LED_2_PIN, LOW);
}

int readAnalog(int pin) {
  int sum = 0;
  for (int i = 0; i < 10; i++) {
    sum += analogRead(pin);
    delay(1);
  }
  return sum / 10;
}

void updateAlarm() {
  if (!alarmActive) return;

  unsigned long currentMillis = millis();

  if (currentMillis - previousAlarmMillis >= ALARM_DELAY) {
    previousAlarmMillis = currentMillis;

    switch (alarmState) {
      case 0:
        tone(BUZZER_PIN, ALARM_FREQUENCY);
        alarmState = 1;
        break;
      case 1:
        digitalWrite(LED_1_PIN, HIGH);
        digitalWrite(LED_2_PIN, LOW);
        alarmState = 2;
        break;
      case 2:
        digitalWrite(LED_1_PIN, LOW);
        digitalWrite(LED_2_PIN, HIGH);
        alarmState = 0;  // loop back
        break;
    }
  }
}

void stopAlarm() {
  alarmActive = false;
  noTone(BUZZER_PIN);
  digitalWrite(LED_1_PIN, LOW);
  digitalWrite(LED_2_PIN, LOW);
  alarmState = 0;
}

void loop() {
  // Always update alarm every loop iteration
  updateAlarm();

  if (millis() - start_time > POLLING_PERIOD) {
    
    // Temp sensor
    Wire.beginTransmission(ADDR);
    Wire.write(TMP_CMD);
    Wire.endTransmission();
    delay(100);

    Wire.requestFrom(ADDR, 2);

    char data[2];
    if(Wire.available() == 2){
      data[0] = Wire.read();
      data[1] = Wire.read();
    }

    temp = ((data[0] * 256.0) + data[1]);
    temp_c = ((175.72 * temp) / 65536.0) - 46.85;

    // Humidity sensor
    Wire.beginTransmission(ADDR);
    Wire.write(0xF5);
    Wire.endTransmission();
    delay(500);

    Wire.requestFrom(ADDR, 2);

    if(Wire.available() == 2){
      data[0] = Wire.read();
      data[1] = Wire.read();
    }

    float humidity = ((data[0] * 256.0) + data[1]);
    humidity = ((125 * humidity) / 65536.0) - 6;

    // Vibration sensor
    int vibration = readAnalog(VIBRATION_PIN);

    // Check thresholds and trigger alarm if exceeded
    if(temp_c > TEMP_THRESHOLD_HIGH || temp_c < TEMP_THRESHOLD_LOW ||
       humidity > HUMIDITY_THRESHOLD_HIGH || humidity < HUMIDITY_THRESHOLD_LOW ||
       vibration > VIBRATION_THRESHOLD_HIGH){
      alarmActive = true;
      Serial.println("ALARM: Threshold exceeded! Temp: " + String(temp_c) + "C, Humidity: " + String(humidity) + "%, Vibration: " + String(vibration));
    } else {
      stopAlarm();
    }

    // Format data as a JSON string
    String formatted_data = "{ \"" + group_name + "\": { \"" + device_name + "\": { \"Temp\": \"" 
        + String(temp_c) + "\", \"Humidity\": \"" + String(humidity) + "\", \"Vibration\": \"" + String(vibration) + "\" } } }" + '\n';
      
    Serial.println("Prepared LoRa message: " + formatted_data);
    Serial2.println(formatted_data);
    Serial.println("LoRa sent!");
    
    start_time = millis();
  }
}