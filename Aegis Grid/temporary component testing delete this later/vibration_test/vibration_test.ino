#define VIBRATION_SENSOR 36
int value = 0;

void setup() {
  pinMode(VIBRATION_SENSOR, INPUT);
  Serial.begin(9600);
}

void loop() {
  value = analogRead(VIBRATION_SENSOR);
  Serial.println(value);
  delay(1000);
}
