// Nicolas Jao - 400450150
// Ayesha Mehmood - 400403482
// Yassin Merdan - 400376619
// Hans Qin - 400461329

#include "gateway_device.h"

void setup()
{
  Serial.begin(9600);
  WiFi.begin(ssid, pass);
  Serial.println("connected");
  client.setServer(mqtt_server, 1883);

  Wire.begin();
  
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
}

void callback(char* topic, byte* payload, unsigned int length) {   
  Serial.println("Callback called");
  
  String data_str = "";
  String topic_str = "";
  for(int i = 0; topic[i] != '\0'; i++){
    topic_str += (char)topic[i];
  }
  for (unsigned int i = 0; i < length; i++){
    data_str += (char)payload[i];
  }
  Serial.println("Data received:\n   Topic: " + topic_str + "\n   Data: " + data_str + "\n");
}

void loop()
{
  if (!client.connected())
  {
    Serial.println("Reconnecting to MQTT broker");
    client.connect(String(String(group_name) + "_" + String(gateway_device_name)).c_str());
    client.setCallback(callback);
    if(client.state() == MQTT_CONNECTED){
      Serial.println("Connected to MQTT broker");
      Serial.print("IP: ");
      Serial.println(WiFi.localIP());
      Serial.println("Wifi Signal Strength: " + String(WiFi.RSSI()) + " dB");
    } else {
      Serial.println("Failed to connect, rc = " + client.state());
    }
  }

  client.loop();

  if (Serial2.available() > 1) {
    Serial.println("Receiving Data...");
    String incoming_data = Serial2.readString();
    StaticJsonDocument<sizeof(incoming_data) + 200> json_document;
    DeserializationError error = deserializeJson(json_document, incoming_data.c_str());

    if (error) {
      Serial.println("JSON parse failed: " + String(error.c_str()));
      return;
    }

    const char* field_device_temp       = json_document[group_name][field_device_name]["Temp"];
    const char* field_device_humidity   = json_document[group_name][field_device_name]["Humidity"];
    const char* field_device_vibration  = json_document[group_name][field_device_name]["Vibration"];
    const char* field_device_l1_spike   = json_document[group_name][field_device_name]["Line1_Spike"];
    const char* field_device_l2_spike   = json_document[group_name][field_device_name]["Line2_Spike"];
    const char* field_device_l4_fault   = json_document[group_name][field_device_name]["Line4_Fault"];

    String base_topic = String(group_name) + "/" + String(field_device_name) + "/";

    client.publish((base_topic + "Temp").c_str(),        field_device_temp);
    client.publish((base_topic + "Humidity").c_str(),    field_device_humidity);
    client.publish((base_topic + "Vibration").c_str(),   field_device_vibration);
    client.publish((base_topic + "Line1_Spike").c_str(), field_device_l1_spike);
    client.publish((base_topic + "Line2_Spike").c_str(), field_device_l2_spike);
    client.publish((base_topic + "Line4_Fault").c_str(), field_device_l4_fault);

    Serial.println("Published 6 MQTT topics.");
  }
}