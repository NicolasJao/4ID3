// Nicolas Jao - 400450150
// Ayesha Mehmood - 400403482
// Yassin Merdan - 400376619
// Hans Qin - 400461329

#include "lora_a.h"

void setup() {

  Serial.begin(9600);
  Serial_1.begin(9600, SERIAL_8N1, 16, 17);

  Transceiver.init();
  Transceiver.SetAddressH(1);
  Transceiver.SetAddressL(1);
  Transceiver.SetAirDataRate(ADR_9600);
  Transceiver.SetChannel(TRANSCEIVER_CHANNEL);
  Transceiver.SetMode(EBYTE_MODE_NORMAL);
  Transceiver.SetTransmitPower(OPT_TP20);
  Transceiver.SaveParameters(PERMANENT);
  Transceiver.PrintParameters();
  
}




void loop() {
  
  if (Serial2.available() > 1) {
    Serial.println("Receving Data...");
    String incomingData = Serial2.readString();
    Serial.println("PING: " + incomingData);

  }

  if(millis() - startTime > TIME_PER_SEND_MS){
    Serial2.println(group_name + " - " + device_name);
    Serial.println("PONG SENT");
    startTime = millis();
  }

}
