#define BUZZER_PIN 12
#define LED_1 13
#define LED_2 14
#define DELAY 50

void setup() {
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);
  digitalWrite(LED_1, LOW);
  digitalWrite(LED_2, LOW);
}

void loop() {
  tone(BUZZER_PIN, 700);
  delay(DELAY);
  digitalWrite(LED_1, HIGH);
  digitalWrite(LED_2, LOW);
  delay(DELAY);
  digitalWrite(LED_1, LOW);
  digitalWrite(LED_2, HIGH);
  delay(DELAY);
}
