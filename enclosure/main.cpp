#include "Arduino.h"
#include "DFRobot_ESP_PH.h"
#include "ir_interface.cpp"
#include "Servo_interface.h"
#include "LED_Array.h"
#include <OneWire.h>
#include "lcd.h"
#include "fish_mqtt.h"
#include <time.h>
#include "tempSensor.h"

// software loop variables (these need to persist boots)
unsigned long prev_time = 0;
long read_interval = 1; // in minutes
bool dynamic_lighting = false;
bool send_alert = false;

// danger cutoff values
#define MAX_TEMP 90
#define MIN_TEMP 70
#define MAX_PH 9
#define MIN_PH 3

// virtual sensor flag (for testing)
int VIRTUAL_SENSOR = 0;

// pH sensor
#define ESPADC 4096.0   //the esp Analog Digital Convertion value
#define ESPVOLTAGE 3300 //the esp voltage supply value
#define PH_PIN 35    //pH sensor gpio pin
DFRobot_ESP_PH ph;

// LCD pins
#define TFT_DC 17
#define TFT_CS 15
#define TFT_RST 5
#define TFT_MISO 19         
#define TFT_MOSI 23           
#define TFT_CLK 18 
int num_of_fish = 5;
LCD lcd;
TempSensor temperature;

// ir sensor
#define IR_PIN 34 //TODO change to ESP pins
#define LED_PIN 26 //TODO change to ESP pins
#define IR_THRESHOLD 50 //TODO change to reflect values in enclosure
ir_sensor ir;

//Temperature chip
int DS18S20_Pin = 4; //DS18S20 Signal pin on digital 2

// servo
#define SERVO_PIN 32
#define DELAY_BETWEEN_ROTATION 1000
#define MIN_FEED_INTERVAL 1200
Servo_Interface si;
int previous_feed_time = -1;


// LED array
LED_Array leds;
char currLEDcolor = 'W';


// MQTT client
char* wifi_SSID = "Verizon-SM-G930V-A5BE"; // -- need to persist boots
char* wifi_PWD = "mtpg344#"; // -- need to persist boots
FishMqtt wiqtt;

// push Alert info
String alert_token  = "akiafy9jms26ojnx53bw5vvivj1s4v";
String alert_usr   = "uaeiijpxfayt5grxg85w97wkeu7gxq"; // -- need to persist boot

// getting time
char* ntpServer = "pool.ntp.org";
long  gmtOffset_sec = -5*60*60;   //Replace with your GMT offset (seconds) -- need to persist boots
int   daylightOffset_sec = 3600;  //Replace with your daylight offset (seconds) -- need to persist boots


//FUNCTION PROTOTYPES
void checkForChangeLED();
int getTime();
int getTimeDiff(int time1, int time2);


/**
 * @brief Called everytime a topic that we are subscribed to publishes data, 
 * calls the appropriate functions to perform the needed actions
 * 
 * @param topic the topic that we are subscribed to 
 * @param payload the actual data receaved
 * @param length the legnth of the data receaved
 */
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
      
      num_of_fish = atoi(buff);
      Serial.println(num_of_fish); 

      // call servo function (once every 12 hours max)
      if((previous_feed_time == -1) || (getTimeDiff(getTime(), previous_feed_time) > MIN_FEED_INTERVAL)){
        for(int i = 0; i < num_of_fish; i++) {
          si.fullRotation(1000); // TODO: make this better
        }
        previous_feed_time = getTime();
      }
      else{
        Serial.println("Unable to feed, time interval too close.");
      }
      
      // publish food level to broker
      if (ir.getFoodLevel()){
        wiqtt.publish("autoq/sensor/feed", "1");
      } else {
        wiqtt.publish("autoq/sensor/feed", "0");
      }
    }

    // LIGHTING CMDS
    else if (!strcmp(topic, "autoq/cmds/leds/bright")) {
        Serial.println("change led brightness");
        // brightness on scale from (0-100)
        int brightness = atoi(buff);
        
        // TODO: set LED brightness
        // leds.setBrightness(brightness);
    }
    else if (!strcmp(topic, "autoq/cmds/leds/color")) {
      Serial.println("change led color");   
      int r = atoi(strtok(buff, ","));
      int g = atoi(strtok(NULL, ","));
      int b = atoi(strtok(NULL, ","));
      leds.setRGBColor(r, g, b);
    }

    
    // SETTING CHANGES
    else if (!strcmp(topic, "autoq/cmds/settings/autoled")) {
        // update dynamic lighting with new value
        int val = atoi(buff);
        dynamic_lighting = val == 1;
        Serial.println("dynamic lighting");
        
    }
    else if (!strcmp(topic, "autoq/cmds/settings/rate")) {
        // update rate
        int val = atoi(buff);
        read_interval = val;
        Serial.println("new update rate");
    }
    else if (!strcmp(topic, "autoq/cmds/settings/alert")) {
        // update rate
        int val = atoi(buff);
        send_alert = val == 1;
        Serial.println("new alert setting");
    }
    
    
    else {
        Serial.println("Not a valid topic");
    }
   
    return;
}



/**
 * @brief Get the time hr:min
 * 
 * @return current time in format HHMM
 */
int getTime(){

  String current_min;
  String current_hour;
  
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return -1;
  }
  
  current_hour = String(timeinfo.tm_hour);
  if (timeinfo.tm_min < 10) {
    current_min = String("0") + String(timeinfo.tm_min);
  } else {
    current_min = String(timeinfo.tm_min);
  }
  
  current_hour = current_hour + current_min;

  return current_hour.toInt();
}


/*
 * @brief returns time1-time2 in hour,min format
 * 
 * @params time1, time2
 */
int getTimeDiff(int time1, int time2){
  int diff = time1-time2;
  int time2_adj;
  if(diff < 0){
    time2_adj = 2400 - time2;
    diff = time1 + time2_adj;
  }
  Serial.print("Time difference = ");
  Serial.println(diff);
  return diff;
}


/**
 * @brief inital setup of devices, this function is only called once
 * 
 */
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  
  // init ph sensor
  ph.init(PH_PIN, ESPADC, ESPVOLTAGE);
  ph.begin();

  // init temp sensor
  temperature.init(DS18S20_Pin);  

  // init ir sensor
  ir.init(IR_PIN, LED_PIN, IR_THRESHOLD);

  // init servo
  si.init(SERVO_PIN);

  // init LEDs
  leds.init(200);

  // init LCD
  lcd.init(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);
    
    
  // init MQTT and wifi
  // TODO: move this to a function to receave user input via a serial cmd
  wiqtt.setWifiCreds(wifi_SSID, wifi_PWD); // setup wifi
  wiqtt.connectToWifi();
  wiqtt.setupMQTT();
  wiqtt.setCallback(callback);
    
  // setup alert
  wiqtt.setAlertCreds(alert_usr);

  // Setup clock
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

}


/**
 * @brief main loop 
 * more information here: https://docs.google.com/document/d/1eHEfdXb2m5zrR4cIb2Fecp_S6VAcF3OrBk4lCq4g3QM/edit
 * 
 */
void loop() {
  int current_time = getTime();

  if (current_time == -1) {
    Serial.println("[ERROR] could not get current time");
    wiqtt.MQTTreconnect();
    return;
  }
  // read sensors and publish to broker every interval
  if ((current_time - prev_time >= read_interval) || (current_time - prev_time < 0)) {
    
    // get water temperature
    float tempVal = temperature.getTemp();
    Serial.print("Temp sensor: ");
    Serial.println(tempVal);
    
    // get water pH
    float pHVal = ph.getPH(tempVal);
    Serial.print("pH sensor: ");
    Serial.println(pHVal);
    
    // get food level
    int foodLevel = ir.getFoodLevel();

    // display current values on LCD
    lcd.updateLCD(tempVal, pHVal, foodLevel, num_of_fish);

    // update time counter
    prev_time = current_time;
    
    // publish to MQTT broker
    wiqtt.publishSensorVals(tempVal, pHVal, current_time);
      
    // check if an alert needs to be sent
    if (send_alert) {
        dangerValueCheck(tempVal, pHVal, foodLevel);
    }
  }
   
  // if the dynamic lighting option is selected
  if (dynamic_lighting) {
    leds.updateDynamicColor(current_time);
  }
  
  // look for incoming commands
  wiqtt.loop(); // needs to be called every 15 seconds at least

}

/**
 * @brief checks if any recorded value is outside of acceabale range, sends an alert if so
 * 
 * @param tempVal water temperature
 * @param pHVal water pH level
 * @param foodLevel food level
 */
void dangerValueCheck(float tempVal, float pHVal, int foodLevel ) {

    String msg;

    // water tempurature value check
    if (tempVal >= MAX_TEMP) {
        msg = "High water temperature detected. Measured value: " + String(tempVal) + " deg-F";
        wiqtt.sendPushAlert(msg);
    }
    else if (tempVal <= MIN_TEMP) {
        msg = "Low water temperature detected. Measured value: " + String(tempVal) + " deg-F";
        wiqtt.sendPushAlert(msg);
    }
    
    // pH value check
    if (pHVal >= MAX_PH) {
        msg = "High water pH detected. Measured value: " + String(pHVal);
        wiqtt.sendPushAlert(msg);
    }
    else if (pHVal <= MIN_PH) {
        msg = "Low water pH detected. Measured value: " + String(pHVal);
        wiqtt.sendPushAlert(msg);
    }
    
    // food level check
    if (foodLevel == 0) {
        msg = "Low food level detected, refill food hopper";
        wiqtt.sendPushAlert(msg);
    }
    return;
}