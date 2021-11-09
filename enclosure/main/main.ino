#include "Arduino.h"
#include "DFRobot_ESP_PH.h"
#include "ir_interface.cpp"
#include "Servo_interface.h"
#include "LED_Array.h"
#include <OneWire.h>
#include "lcd.h"
#include "fish_mqtt.h"

//software loop variables
#define MSTOSECS 1000
unsigned long prev_time = 0;
long read_interval = 5*MSTOSECS;//*60*10; //10 minutes
bool dynamic_lighting = false;

// virtual sensor flag (for testing)
int VIRTUAL_SENSOR = 1;

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
LCD lcd(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);

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
wifi_SSID = "Fishwifi";
wifi_PWD = "fishfood";
FishMqtt wiqtt();


//FUNCTION PROTOTYPES
void getPH(int temperature_in);
float getTemp();
void checkForMoveServo();
int getFoodLevel();
void checkForChangeLED();
void updateDynamicLED();


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
      
      int num_of_fish = atoi(buff); 

      // TODO: call servo function
        
      // TODO: publish food level to broker
    }

    // LIGHTING CMDS
    else if (!strcmp(topic, "autoq/cmds/led/bright")) {
        Serial.println("change led brightness");
        // brightness on scale from (0-100)
        int brightness = atoi(buff);
        
        // TODO: set LED brightness
        // leds.setBrightness(brightness);
    }
    else if (!strcmp(topic, "autoq/cmds/led/color")) {
      Serial.println("change led color");
            
      int red = atoi(strtok(buff, ","));
      int green = atoi(strtok(NULL, ","));
      int blue = atoi(strtok(NULL, ","));
      
      // set LED color
      leds.setRGBColor(red, green, blue);
    }
    
    // SETTING CHANGES
    else if (!strcmp(topic, "autoq/cmds/settings")) {
        Serial.println("change settings");
        // update rate
        // update dynamic lighting with new value
    } 
    else {
        Serial.println("Not a valid topic");
    }
   
    return;
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
  ir.init(IR_PIN, LED_PIN);

  //init servo
  si.initServo(SERVO_PIN);

  //init led array
  leds.init(300);
  
  // init MQTT and wifi
  // TODO: move this to a function to receave user input via a serial cmd
  wiqtt.setWifiCreds(wifi_SSID, wifi_PWD); // setup wifi
  wiqtt.connectToWifi();
  wiqtt.setupMQTT();
  wiqtt.setCallback(callback);

}


/**
 * @brief main loop 
 * more information here: https://docs.google.com/document/d/1eHEfdXb2m5zrR4cIb2Fecp_S6VAcF3OrBk4lCq4g3QM/edit
 * 
 */
void loop() {
  unsigned long current_time = millis();

  // read sensors and publish to broker every interval
  if (current_time - prev_time >= read_interval) {
    
    // get water temperature
    float tempVal = getTemp();
    Serial.print("Temp sensor: ");
    Serial.println(tempVal);
    
    // get water pH
    float pHVal = getPH(tempVal);
    Serial.print("pH sensor: ");
    Serial.println(pHVal);
    
    // get food level
    int foodLevel = getFoodLevel();

    // display current values on LCD
    lcd.updateLCD(tempVal, pHVal, foodLevel);
    
    // publish to MQTT broker
    wiqtt.publishSensorVals(tempVal, pHVal, current_time);
    
    // update time counter
    prev_time = current_time;
  }
   
  // if the dynamic lighting option is selected
  if (dynamic_lighting) {
      // updateDynamicLED(current_time);
  }
  
  // look for incoming commands
  // checkForChangeLED(); //For testing purposes only, replace with wiqtt.loop()
  wiqtt.loop();

}


/**
 * @brief Get the current pH reading from the pH sensor
 * 
 * @param temperature_in The current water temperature
 * @return float value of the pH
 */
float getPH(float temperature_in) {

    if (VIRTUAL_SENSOR) return 7.24;

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
 * @brief Get the Food Level
 * 
 * @return int, 1 if full, 0 otherwise
 */
int getFoodLevel() {
  if (VIRTUAL_SENSOR) return 1;
  int irVal = ir.readVoltage();
  Serial.println(irVal);
  if(irVal > IR_THRESHOLD){
    Serial.println("LOW FOOD LEVEL!");
    return 0;
  }
  else{
    Serial.println("Food level is good");
    return 1;
  }
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
 * @brief this function is just for testing without MQTT stuff
 * 
 */
void checkForChangeLED(){
    String msg_in;

  //TODO: change to parsing wireless message
  if (Serial.available() > 0) {
    msg_in = Serial.readString();
    Serial.print("Serial received ");
    Serial.println(msg_in);
  }


  if (msg_in == "MOVESERVO\n") {
    Serial.println("Start moving servo.");
    //move servo once
    si.fullRotation(1000);
    Serial.println("Servo moved!");
    delay(DELAY_BETWEEN_ROTATION); //delay in between rotations

    //check for low food level
    getFoodLevel();
  }
  if (msg_in == "CHANGELED\n") {
    Serial.println("Change the LED!");
    if (currLEDcolor == 'B') {
      leds.colorTransition(0,0,100, 100,100,100, 5000);
      currLEDcolor = 'W';
    }
    else if (currLEDcolor == 'W') {
      leds.colorTransition(100,100,100, 0,0,100, 5000);
      currLEDcolor = 'B';
    }
    else {
      leds.setRGBColor(255, 0, 0);
    }
    
  }
}
