#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>



class FishMqtt {

	// MQTT broker info
	char *mqttServer =  "e948e5ec3f1b48708ce7748bdabab96e.s1.eu.hivemq.cloud";
	int mqttPort = 8883;

	// MQTT client credentials
	char *clientName = "FishClient-666";
	char *usrname = "fishusr";
	char *password = "Fish123!";

	// WiFi credentials
	char SSID[40];
	char PWD[40];


	// WiFi client
	WiFiClientSecure espClient;
	
	// mqtt client
	PubSubClient mqttClient(espClient); 


public:

	FishMqtt() {
		Serial.begin(115200);
	}

	// sets the SSID and password for the WiFi network
	void setWifiCreds(char *SSID_in, char *PWD_in) {
		if (strlen(SSID_in) <= 40 && strlen(PWD_in) <= 40) {
			strcpy(SSID_in, SSID);
			strcpy(PWD_in, PWD);	
		} else {
			Serial.println("[ERROR] Could not set wiFi SSID or password");
		}
	}


	// Connects to the WiFi used the set credentials
	void connectToWifi() {
	  int status = WL_IDLE_STATUS;
	  Serial.print("Connecting to ");
	  
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


	// inital setup of MQTT client
	void setupMQTT() {
	  mqttClient.setServer(mqttServer, mqttPort);
	  mqttClient.setCallback(callback);
	  delay(1500);
	  
	  Serial.println("Connecting to MQTT Broker...");
	  while (!mqttClient.connected()) {
	      Serial.println("Reconnecting to MQTT Broker..");
	      if (mqttClient.connect(clientName, usrname, password)) {
	        Serial.println("Connected to broker.");

	        // subscribe to topic
	        mqttClient.subscribe("commands");
	        Serial.println("Subscribed to topic: commands");
	        return;
	      }
	      if(WiFi.status() != WL_CONNECTED) {
	        Serial.println("WiFI disconnected");
	        connectToWiFi();
	      }
	  }
	}


	// reconnects when connection is lost
	void MQTTreconnect() {
	  Serial.println("Connecting to MQTT Broker...");
	  while (!mqttClient.connected()) {
	      Serial.println("Reconnecting to MQTT Broker..");
	      if (mqttClient.connect(clientName, usrname, password)) {
	        Serial.println("Connected to broker.");
	        // subscribe to topic
	        mqttClient.subscribe("command"); //subscribes to all the commands messages triggered by the user
	        Serial.println("Subscribed to topic: commands");
	        return;
	      }
	      if(WiFi.status() != WL_CONNECTED) {
	        Serial.println("WiFI disconnected");
	        connectToWiFi();
	      }
	  }
}
