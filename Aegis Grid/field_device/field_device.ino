// Nicolas Jao - 400450150
// Ayesha Mehmood - 400403482
// Yassin Merdan - 400376619
// Hans Qin - 400461329

#include "field_device.h"

// ---- BREAK LOCATION PINS ----
#define LINE_4_S1_ALIVE          32
#define LINE_4_S2_ALIVE          35
#define LINE_4_S3_ALIVE          34
#define LINE_4_S4_ALIVE          39

// ---- DEMAND SPIKE PINS ----
#define LINE_1_DEMAND_SPIKE      12
#define LINE_2_DEMAND_SPIKE      14
#define TARGET_VOLTAGE           864

// ---- BREAK LOCATION DEADBAND ----
#define DEADBAND                 200
#define NEAR(val, target) (val >= target - DEADBAND && val <= target + DEADBAND)

// ---- ALARM THRESHOLDS ----
#define TEMP_THRESHOLD_HIGH      35.0
#define TEMP_THRESHOLD_LOW       10.0
#define HUMIDITY_THRESHOLD_HIGH  70.0
#define HUMIDITY_THRESHOLD_LOW   20.0
#define VIBRATION_THRESHOLD_HIGH 1000

// ---- ALARM OUTPUT PINS ----
#define BUZZER_PIN               19
#define LED_1_PIN                4
#define LED_2_PIN                5

// ---- ALARM PARAMETERS ----
#define ALARM_DELAY              50
#define ALARM_FREQUENCY          700

// ---- VIBRATION SENSOR PIN ----
#define VIBRATION_PIN            36

// ---- ALARM MILLIS TRACKING ----
unsigned long previousAlarmMillis = 0;
int alarmState = 0;
bool alarmActive = false;

// ---- VARIABLES ----
int S1, S2, S3, S4;
int L1, L2;
int ALARM_FLAG_1, ALARM_FLAG_2, ALARM_FLAG_3;
int vibration_peak = 0;

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

  mcp.init();
  mcp.portMode(MCP23017Port::A, 0);
  mcp.writeRegister(MCP23017Register::GPIO_A, 0x00);

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
  for (int i = 0; i < 30; i++) {
    sum += analogRead(pin);
    delay(1);
  }
  return sum / 30;
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
        alarmState = 0;
        break;
    }
  }
}

void stopAlarm() {
  if (ALARM_FLAG_1 == false && ALARM_FLAG_2 == false && ALARM_FLAG_3 == false) {
    alarmActive = false;
    noTone(BUZZER_PIN);
    digitalWrite(LED_1_PIN, LOW);
    digitalWrite(LED_2_PIN, LOW);
    alarmState = 0;
  }
}

void loop() {
  // Always update alarm every loop iteration
  updateAlarm();

  // Read vibration every iteration, keep peak value
  int vibration_reading = readAnalog(VIBRATION_PIN);
  if (vibration_reading > vibration_peak) {
    vibration_peak = vibration_reading;
  }

  // Read values for LINE 4 nodes
  S1 = analogRead(LINE_4_S1_ALIVE);
  S2 = analogRead(LINE_4_S2_ALIVE);
  S3 = analogRead(LINE_4_S3_ALIVE);
  S4 = analogRead(LINE_4_S4_ALIVE);

  // Read values for LINE 1 and LINE 2 nodes
  L1 = analogRead(LINE_1_DEMAND_SPIKE);
  L2 = analogRead(LINE_2_DEMAND_SPIKE);

  if (millis() - start_time > POLLING_PERIOD) {

    // Temp sensor
    Wire.beginTransmission(ADDR);
    Wire.write(TMP_CMD);
    Wire.endTransmission();
    delay(100);

    Wire.requestFrom(ADDR, 2);

    char data[2];
    if (Wire.available() == 2) {
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

    if (Wire.available() == 2) {
      data[0] = Wire.read();
      data[1] = Wire.read();
    }

    float humidity = ((data[0] * 256.0) + data[1]);
    humidity = ((125 * humidity) / 65536.0) - 6;

    // Use peak vibration value captured this polling period
    int vibration = vibration_peak;

    // Check thresholds and trigger alarm if exceeded
    if (temp_c > TEMP_THRESHOLD_HIGH || temp_c < TEMP_THRESHOLD_LOW ||
        humidity > HUMIDITY_THRESHOLD_HIGH || humidity < HUMIDITY_THRESHOLD_LOW ||
        vibration > VIBRATION_THRESHOLD_HIGH) {
      ALARM_FLAG_1 = true;
      alarmActive = true;
      Serial.println(">> ALARM: THRESHOLD EXCEEDED! Temp: " + String(temp_c) + "C, Humidity: " + String(humidity) + "%, Vibration: " + String(vibration));
    } else {
      ALARM_FLAG_1 = false;
    }

    // LINE 1 and LINE 2 demand spike monitoring
    bool L1_spike = (L1 > TARGET_VOLTAGE + DEADBAND);
    bool L2_spike = (L2 > TARGET_VOLTAGE + DEADBAND);
    Serial.println("=== LINE 1 AND LINE 2 DEMAND SPIKE STATUS ===");
    if (!L1_spike && !L2_spike) {
      Serial.println(">> LINE 1 and LINE 2 are good. No demand spike detected.");
      ALARM_FLAG_2 = false;
    } else if (L1_spike) {
      Serial.println(">> ALARM: DEMAND SPIKE on LINE 1.");
      ALARM_FLAG_2 = true;
      alarmActive = true;
    } else {
      Serial.println(">> ALARM: DEMAND SPIKE on LINE 2.");
      ALARM_FLAG_2 = true;
      alarmActive = true;
    }

    // LINE 4 break location monitoring
    String fault_location;
    Serial.println("=== LINE 4 FAULT MONITORING STATUS ===");
    if (NEAR(S1, 4095) && NEAR(S2, 2896) && NEAR(S3, 1872) && NEAR(S4, 864)) {
      Serial.println(">> All nodes nominal. Line operating normally.");
      fault_location = "NO FAULT.";
      ALARM_FLAG_3 = false;
    } else if (NEAR(S1, 4095) && NEAR(S2, 4095) && NEAR(S3, 4095) && NEAR(S4, 4095)) {
      Serial.println(">> ALARM: FAULT DETECTED. Break is at S4.");
      fault_location = "FAULT AT S4.";
      ALARM_FLAG_3 = true;
      alarmActive = true;
    } else if (NEAR(S1, 4095) && NEAR(S2, 4095) && NEAR(S3, 4095) && NEAR(S4, 0)) {
      Serial.println(">> ALARM: FAULT DETECTED. Break is at S3.");
      fault_location = "FAULT AT S3.";
      ALARM_FLAG_3 = true;
      alarmActive = true;
    } else if (NEAR(S1, 4095) && NEAR(S2, 4095) && NEAR(S3, 0) && NEAR(S4, 0)) {
      Serial.println(">> ALARM: FAULT DETECTED. Break is at S2.");
      fault_location = "FAULT AT S2.";
      ALARM_FLAG_3 = true;
      alarmActive = true;
    } else if (NEAR(S1, 4095) && NEAR(S2, 0) && NEAR(S3, 0) && NEAR(S4, 0)) {
      Serial.println(">> ALARM: FAULT DETECTED. Break is at S1.");
      fault_location = "FAULT AT S1.";
      ALARM_FLAG_3 = true;
      alarmActive = true;
    } else {
      Serial.println(">> UNKNOWN STATE. Check wiring.");
      fault_location = "UNKNOWN.";
    }

    // Format data as JSON string
    Serial.println("=== EQUIPMENT MAINTENANCE DATA ===");
    String formatted_data = "{ \"" + group_name + "\": { \"" + device_name + "\": { "
        "\"Line1_Spike\": \""  + String(L1_spike ? "true" : "false") + "\", "
        "\"Line2_Spike\": \""  + String(L2_spike ? "true" : "false") + "\", "
        "\"Temp\": \""         + String(temp_c)    + "\", "
        "\"Humidity\": \""     + String(humidity)  + "\", "
        "\"Vibration\": \""    + String(vibration) + "\", "
        "\"Line4_Fault\": \""  + fault_location    + "\" } } }" + '\n';

    Serial.println("Prepared LoRa message: " + formatted_data);
    Serial2.println(formatted_data);
    Serial.println("LoRa sent!");

    stopAlarm();

    vibration_peak = 0;  // reset peak after each polling period
    start_time = millis();
  }
}