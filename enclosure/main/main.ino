#include "Arduino.h"
#include "DFRobot_ESP_PH.h"
#include "ir_interface.cpp"
#include "Servo_interface.h"
#include "LED_Array.h"
#include <OneWire.h>
#include "lcd.h"
#include "fish_mqtt.h"
#include <time.h>

//software loop variables
unsigned long prev_time = 0;
long read_interval = 2; // in minutes
bool dynamic_lighting = false;

// virtual sensor flag (for testing)
int VIRTUAL_SENSOR = 0;

//pH sensor
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

//ir sensor
#define IR_PIN 34 //TODO change to ESP pins
#define LED_PIN 26 //TODO change to ESP pins
#define IR_THRESHOLD 50 //TODO change to reflect values in enclosure
ir_sensor ir;

//Temperature chip
int DS18S20_Pin = 4; //DS18S20 Signal pin on digital 2
OneWire ds(DS18S20_Pin);  // on digital pin 2

//Servo
#define SERVO_PIN 32
#define DELAY_BETWEEN_ROTATION 1000
Servo_Interface si;

//LED array
LED_Array leds;
char currLEDcolor = 'W';


// MQTT client
char* wifi_SSID = "Fishwifi";
char* wifi_PWD = "fishfood";
FishMqtt wiqtt;


// getting time
char* ntpServer = "pool.ntp.org";
long  gmtOffset_sec = -5*60*60;   //Replace with your GMT offset (seconds)
int   daylightOffset_sec = 3600;  //Replace with your daylight offset (seconds)


//FUNCTION PROTOTYPES
void getPH(int temperature_in);
float getTemp();
void checkForMoveServo();
int getFoodLevel();
void checkForChangeLED();
void updateDynamicLED();
void firstTimeSetup();
int getTime();


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

      // call servo function
      for(int i = 0; i < num_of_fish; i++) {
        si.fullRotation(1000); // TODO: make this better
      }
      
      // publish food level to broker
      if (getFoodLevel()){
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
    
    else {
        Serial.println("Not a valid topic");
    }
   
    return;
}


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



/**
 * @brief inital setup of devices, this function is only called once
 * 
 */
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  
  // init ph sensor
  ph.begin();

  // init temp sensor
  pinMode(DS18S20_Pin, INPUT);

  // init ir sensor
  ir.init(IR_PIN, LED_PIN, IR_THRESHOLD);

  //init servo
  si.initServo(SERVO_PIN);

  //init led array
  leds.init(300);

  // init LCD
  lcd.init(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);
    
  // check if connected to computer
  firstTimeSetup();
    
  // init MQTT and wifi
  // TODO: move this to a function to receave user input via a serial cmd
  wiqtt.setWifiCreds(wifi_SSID, wifi_PWD); // setup wifi
  wiqtt.connectToWifi();
  wiqtt.setupMQTT();
  wiqtt.setCallback(callback);

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
    float tempVal = getTemp();
    Serial.print("Temp sensor: ");
    Serial.println(tempVal);
    
    // get water pH
    float pHVal = getPH(tempVal);
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
  }
   
  // if the dynamic lighting option is selected
  if (dynamic_lighting) {
      // updateDynamicLED(current_time);
  }
  
  // look for incoming commands
  wiqtt.loop(); // needs to be called every 15 seconds at least

}


/**
 * @brief Get the current pH reading from the pH sensor
 * 
 * @param temperature_in The current water temperature
 * @return float value of the pH
 */
float getPH(float temperature_in) {

    if (VIRTUAL_SENSOR) return 0;

    float voltage = analogRead(PH_PIN) / ESPADC * ESPVOLTAGE; // read the voltage
    //Serial.print("voltage:");
    //Serial.println(voltage, 4);

    return ph.readPH(voltage, temperature_in); // convert voltage to pH with temperature compensation
}


/**
 * @brief Get the current reading from the temperature sensor
 * 
 * @return float value of the temperature in Celsius 
 */
float getTemp() {
  //returns the temperature from one DS18S20 in DEG Celsius

  if (VIRTUAL_SENSOR) return 80.71;

  byte data[12];
  byte addr[8];

  //Serial.println(ds.search(addr));

  /*for (int i = 0; i < 8; ++i) {
    Serial.print(addr[i]);
  }*/
  
  if ( !ds.search(addr)) {
      //no more sensors on chain, reset search
      ds.reset_search();
      return -1000;
  }

  if ( OneWire::crc8( addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return -100;
  }

  if ( addr[0] != 0x10 && addr[0] != 0x28) {
      Serial.println("Device is not recognized");
      return -10;
  }

  ds.reset();
  ds.select(addr);
  ds.write(0x44,1); // start conversion, with parasite power on at the end

  byte present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE); // Read Scratchpad

  for (int i = 0; i < 9; i++) { // we need 9 bytes
    data[i] = ds.read();
  }

  ds.reset_search();

  byte MSB = data[1];
  byte LSB = data[0];

  float tempRead = ((MSB << 8) | LSB); //using two's compliment
  float TemperatureSum = tempRead / 16;

  return (TemperatureSum * 18 + 5)/10 + 32;
}



/**
 * @brief allows for dynamic LED changes
 * 
 * @param time The current time
 */
void updateDynamicLED(int time) {
    // TODO: based on the current time, change the lights appropperly
}



/**
 * @brief first-time configuring, setups up wifi credientals, timezone, and clientID
 * 
 */
void firstTimeSetup() {
    // check for incoming serial connections
    // python program sends message to initiate connection
    
    // get user timezone
    
    
    // get the wifi SSID and password
    
    // assure that the wifi can be connected to sucessfully
    // need to save the values here: https://www.esp32.com/viewtopic.php?t=4767     
}
