#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>


// wifi stuff
const char *SSID = "Home-3034";
const char *PWD = "8157150028";
WiFiClientSecure espClient; 


void connectToWiFi() {
  int status = WL_IDLE_STATUS;
  Serial.print("Connecting to ");
  
  //WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PWD);
  Serial.println(SSID);
  while (status != WL_CONNECTED) {
    Serial.print(".");
    status = WiFi.begin(SSID, PWD);
    delay(10000);
  }
  Serial.println(WiFi.RSSI());
  Serial.println("Connected to WiFi");
  
}


// MQTT stuff
char *mqttServer =  "e948e5ec3f1b48708ce7748bdabab96e.s1.eu.hivemq.cloud";//"e948e5ec3f1b48708ce7748bdabab96e.s1.eu.hivemq.cloud";
int mqttPort = 8883;

// Client credentals
char *usrname = "fishusr";
char *password = "Fish123!";
static const char *fingerprint PROGMEM = "44 14 9A 3F C3 E9 F1 F3 84 1A B4 9F B6 4D 19 8A B2 92 31 D6";

void callback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
}


PubSubClient mqttClient(espClient); 

void setupMQTT() {

  mqttClient.setServer(mqttServer, mqttPort);
  mqttClient.setCallback(callback);
  delay(1500);
  
  Serial.println("Connecting to MQTT Broker...");
  while (!mqttClient.connected()) {
      Serial.println("Reconnecting to MQTT Broker..");
      if (mqttClient.connect("FishClient-666", "fishusr", "Fish123!")) {//, "fishusr", "Fish123!")) {
        Serial.println("Connected to broker.");
        return;
        // subscribe to topic
        // mqttClient.subscribe("/test/messages");
      }
      if(WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFI disconnected");
        connectToWiFi();
      }
  }
}


void MQTTreconnect() {
  Serial.println("Connecting to MQTT Broker...");
  while (!mqttClient.connected()) {
      Serial.println("Reconnecting to MQTT Broker..");
      if (mqttClient.connect("FishClient-666", "fishusr", "Fish123!")) {//, "fishusr", "Fish123!")) {
        Serial.println("Connected to broker.");
        return;
        // subscribe to topic
        // mqttClient.subscribe("/test/messages");
      }
      if(WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFI disconnected");
        connectToWiFi();
      }
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  connectToWiFi();
  espClient.setInsecure(); //needed to bypass verification - should change later (see here: https://github.com/hometinker12/ESP8266MQTTSSL)
  setupMQTT();

}

void loop() {
  // put your main code here, to run repeatedly:
  // get data from sensor
  
  // serialize into json string format
  DynamicJsonDocument doc(1024);
  doc["timestamp"]   = 1351824120;
  doc["pH_val"] = 7.02;
  doc["temp_val"] = 85.20;
  doc["food"] = true;
  
  // publish to broker
  String output;
  serializeJson(doc, output);
  Serial.println(output);
  if (!mqttClient.connected()) {
    MQTTreconnect();
  }
  mqttClient.publish("sensor/data/output", "{\"timestamp\":1351824120,\"pH_val\":7.02,\"temp_val\":85.2,\"food\":true}");
  delay(30000);
}
