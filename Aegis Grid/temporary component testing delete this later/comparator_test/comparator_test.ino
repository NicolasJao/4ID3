#define LINE_1_DEMAND_SPIKE 2
/*
#define LINE_3_DEMAND_SPIKE
#define LINE_4_S1_ALIVE
#define LINE_4_S2_ALIVE
#define LINE_4_S3_ALIVE
#define LINE_4_S4_ALIVE
*/

void setup() {
  pinMode(LINE_1_DEMAND_SPIKE, INPUT);
  Serial.begin(9600);
}

void loop() {
  Serial.println(digitalRead(LINE_1_DEMAND_SPIKE));
  delay(1000);
}
