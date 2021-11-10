// About this class:
// FishMqtt combines the funcinality of the WiFi client and MQTT client, 
// this allows for more things to be done with less code and clutter 
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

class FishMqtt : public PubSubClient {
private:
	// MQTT broker info
	char *mqttServer =  "e948e5ec3f1b48708ce7748bdabab96e.s1.eu.hivemq.cloud";
	int mqttPort = 8883;

	// MQTT client credentials
	char *clientName = "FishClient-666";
	char *usrname = "fishusr";
	char *password = "Fish123!";

	// WiFi credentials
	char wifi_SSID[40];
	char wifi_PWD[40];

	// WiFi client for esp32 chip
	WiFiClientSecure espClient;


public:

	/**
	 * @brief Construct a new Fish Mqtt object
	 * 
	 */
	FishMqtt() : PubSubClient(espClient) {}


	/**
	 * @brief sets s the wifi SSID and password, Dose not begin wifi connection 
	 * 
	 * @param SSID_in The name of the WiFi network to connect to
	 * @param PWD_in The password of the WiFi network to connect to
	 */
	void setWifiCreds(char *SSID_in, char *PWD_in) {
  
		if (strlen(SSID_in) <= 40 && strlen(PWD_in) <= 40) {
			strcpy(wifi_SSID, SSID_in);
			strcpy(wifi_PWD, PWD_in);	
		} else {
			Serial.println("[ERROR] Could not set wiFi SSID or password");
		}
	}



	/**
	 * @brief Connects to the WiFi using the credentials provided in the class variables
	 *	Loops until connection is established
	 * 
	 */
	void connectToWifi() {
	  int status = WL_IDLE_STATUS;
	  Serial.print("Connecting to ");
	  
	  WiFi.begin(wifi_SSID, wifi_PWD);
	  Serial.println(wifi_SSID);
	  while (status != WL_CONNECTED) {
	    Serial.print(".");
	    status = WiFi.begin(wifi_SSID, wifi_PWD);
	    delay(10000);
	  }
	  Serial.println(WiFi.RSSI());
	  Serial.println("Connected to WiFi");

	  //needed to bypass verification
	  // TODO: change to something more secure (see here: https://github.com/hometinker12/ESP8266MQTTSSL)
	  espClient.setInsecure(); 
	}



	/**
	 * @brief Connects the MQTT broker specified in the setServer() call
	 * If needed, reconnects to the configured WiFi using connectToWifi()
	 *
	 */
	void MQTTreconnect() {
	  Serial.println("Connecting to MQTT Broker...");
	  while (!connected()) {
	      Serial.println("Reconnecting to MQTT Broker..");
	      if (connect(clientName, usrname, password)) {
	        Serial.println("Connected to broker.");
	        // subscribe to topic
	        subscribe("autoq/cmds/#"); //subscribes to all the commands messages triggered by the user
	        Serial.println("Subscribed to topic: commands");
	        return;
	      }
	      if(WiFi.status() != WL_CONNECTED) {
	        Serial.println("WiFI disconnected");
	        connectToWifi();
	      }
	  }
	}



	/**
	 * @brief sets the server and calls MQTTrecconnect() to connect to WiFi and the broker
	 *
	 */
	void setupMQTT() {
	  setServer(mqttServer, mqttPort);
	  delay(1500);
	  MQTTreconnect();
	}


	/**
	 * @brief This function serializes the inputted values and then 
	 * publishes the serialized string to the MQTT broker
	 * 
	 * @param tempVal the temperature value to be published
	 * @param pHVal the pH value to be published
	 * @param time the time that the data was published
	 */
   void publishSensorVals(float tempVal, float pHVal, int time) {
  
      // Serialize the sensor data
      DynamicJsonDocument doc(1024);
      doc["pH_val"] = pHVal;
      doc["temp_val"] = tempVal;
      doc["time"] = time;
      String output;
      serializeJson(doc, output);
  
  
      // publish the data to the broker
      if (!connected()) MQTTreconnect();
      publish("autoq/sensor/output", output.c_str()); //need to convert to c_string
   }


};
