#include "mqtt_fish.h"
#include <ArduinoJson.h>

//FishMqtt combines the funcinality of WiFi and MQTT
FishMqtt wiqtt; 

// WiFi creds, will be received via serial commands
char wifi_SSID[] = "Fishwifi";
char wifi_PWD[] = "fishfood";

// this is where the parsing of the subscribed topics is done
void callback(char* topic, byte* payload, unsigned int length) {
  // TODO: add the parsing, 
    // if the topic is "commands/servo" move the servo
    // if the topic is "commands/led" change the leds
  return;
}

void setup() {
  // put your setup code here, to run once:
  
  Serial.begin(115200);

  wiqtt.setWifiCreds(wifi_SSID, wifi_PWD); // this is my mobile hotspot
  wiqtt.connectToWifi();
  wiqtt.setupMQTT();
  wiqtt.setCallback(callback);

}

void loop() {
  // put your main code here, to run repeatedly:
  
  // serialize into json string format
  // TODO: just replace the values with the sensor values
  DynamicJsonDocument doc(1024);
  doc["timestamp"]   = 1351824120;
  doc["pH_val"] = 7.02;
  doc["temp_val"] = 85.20;
  doc["food"] = true;

  // publish to broker
  String output;
  serializeJson(doc, output);
  //Serial.println(output);
  if (!wiqtt.connected())
    wiqtt.MQTTreconnect();
  wiqtt.publish("sensor/data/output", "test"); //"{\"timestamp\":1351824120,\"pH_val\":7.02,\"temp_val\":85.2,\"food\":true}"
  
  delay(30000); // delay() causes the mqtt connection to break, thus this will break the connection
}
