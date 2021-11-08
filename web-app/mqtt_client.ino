#include "mqtt_fish.h"
#include <ArduinoJson.h>

//FishMqtt combines the funcinality of WiFi and MQTT
FishMqtt wiqtt; 

// WiFi creds, will be received via serial commands
char wifi_SSID[] = "Fishwifi";
char wifi_PWD[] = "fishfood";

// this is where the parsing of the subscribed topics is done
void callback(char* topic, byte* payload, unsigned int length) {

    // convert from byte array to char buffer
    char buff[30];
    int i = 0;    
    for (; i < length; i++) {
      buff[i] = (char) payload[i];
    }
    buff[i] = '\0';


    // FEEDING CMDS
    if (!strcmp(topic, "autoq/cmds/feed")) {
      Serial.println("feed the fish");
      
      int num_of_fish = atoi(buff); 

      // TODO: call servo function
    }

    // LIGHTING CMDS
    else if (!strcmp(topic, "autoq/cmds/leds")) {
      Serial.println("change led lighting");
      
      int brightness = atoi(strtok(buff, ","));
      
      int red = atoi(strtok(NULL, ","));
      int green = atoi(strtok(NULL, ","));
      int blue = atoi(strtok(NULL, ","));
      
      //TODO: call LED function
    }
    
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

  wiqtt.publishSensorVals(90.2, 7.13, 1); // temp, pH, food
  // wiqtt.loop();
  // delay(30000); // delay() causes the mqtt connection to break, thus this will break the connection
}