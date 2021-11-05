#include "Arduino.h"
#include "DFRobot_ESP_PH.h"
#include "ir_interface.cpp"
#include "Servo_interface.h"
#include "LED_Array.h"
#include <WiFi.h>         //TODO: remove this, replace with fish_mqtt interface in web-app folder 
#include <PubSubClient.h> //TODO: remove this, replace with fish_mqtt interface in web-app folder
#include <OneWire.h>

// LCD libraries
#include "Adafruit_GFX.h"     
#include "Adafruit_ILI9341.h" 
#include <Wire.h>
#include  <SPI.h>

// LCD Pins
#define TFT_DC 4
#define TFT_CS 15
#define TFT_RST 2
#define TFT_MISO 19         
#define TFT_MOSI 23           
#define TFT_CLK 18 

// Colors for LCD Display
#define black  0x0000  // 
#define white 0xFFFF  // RGB
#define red 0xF800  // R
#define green 0x3606  // G
#define water_blue 0x033F6  // B
#define yellow  0xFFE0  // RG
#define cyan  0x07FF  // GB
#define magenta 0xF81F  // RB
#define gray  0x0821  // 00001 000001 00001
#define orange 0xFB46

// LCD Initialization
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);

//software loop variables
#define MSTOSECS 1000
unsigned long ph_previous_time = 0, temp_previous_time = 0;
long ph_interval = 5*MSTOSECS;//*60*10; //10 minutes
long temp_interval = 5*MSTOSECS;//*60*10; //10 minutes

//pH sensor
#define ESPADC 4096.0   //the esp Analog Digital Convertion value
#define ESPVOLTAGE 3300 //the esp voltage supply value
#define PH_PIN 35    //pH sensor gpio pin
volatile float voltage, pHVal;
DFRobot_ESP_PH ph;

//ir sensor
#define IR_PIN 34 //TODO change to ESP pins
#define LED_PIN 23 //TODO change to ESP pins
#define IR_THRESHOLD 50 //TODO change to reflect values in enclosure
ir_sensor ir;

//Temperature chip
int DS18S20_Pin = 4; //DS18S20 Signal pin on digital 2
volatile float tempVal;
OneWire ds(DS18S20_Pin);  // on digital pin 2

//Servo
#define SERVO_PIN 32
#define DELAY_BETWEEN_ROTATION 1000
Servo_Interface si;

//LED array
LED_Array leds;
char currLEDcolor = 'W';

//FUNCTION PROTOTYPES
void getPH(int temperature_in);
float getTemp();
void checkForMoveServo();
void checkFoodLevel();
void checkForChangeLED();

// 0 = full, 1 = empty
int foodLevel = 0;

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
  
  //init LCD
  tft.begin();                      
  tft.setRotation(3);            
  tft.fillScreen(ILI9341_BLACK);
  
  Wire.begin();

  printText("AUTOQUARIUM", water_blue,30,20,4);
  printText("pH", white,40,70,3);
  printText("Temp", white,200,70,3);
  printText("Food", white,30,150,3);
  printText("Fish", white,200,150,3);
}

//For more information on software loop, see https://docs.google.com/document/d/1eHEfdXb2m5zrR4cIb2Fecp_S6VAcF3OrBk4lCq4g3QM/edit
void loop() {
  unsigned long current_time = millis();


  //read from temp sensor every interval
  if (current_time - temp_previous_time >= temp_interval) {
    temp_previous_time = current_time;
    //read from temp sensor
    tempVal = getTemp();
    Serial.print("Temp sensor: ");
    Serial.println(tempVal);
  }

  //read from ph sensor every interval
  if (current_time - ph_previous_time >= ph_interval) {
    ph_previous_time = current_time;
    //read from ph sensor using temp sensor values
    pHVal = getPH();
    Serial.print("pH sensor: ");
    Serial.println(pHVal);
    
    if(pHVal == 7)
    {
      printText(pHVal, green,40,100,3);
    }
    
    else if(phVal < 6.5 || phVal > 7.5)
    {
      printText(pHVal, red,40,100,3);
    }
    
    else
    {
      printText(pHVal, orange,40,100,3);
    }
    
    if(tempVal < 23 || tempVal > 27)
    {
      printText(tempVal, red,200,100,3);
    }
    
    else
    {
      printText(tempVal, green,200,100,3);
    }
    
    // food value
    if(foodLevel == 0)
    {
      printText("Good", green,30,180,3);
    }
    else
    {
      printText("Low", red,30,180,3);
    }
    
    // Num Fish
    printText("5", green,200,180,3);
    
  }

  //if wireless command received, move servo (Serial Command: MOVESERVO)
  //TODO: change from parsing serial to decoding wireless message
//  checkForMoveServo();

  //if wireless command received, change color of leds
  //TODO: change from parsing serial to decoding wireless message
  checkForChangeLED();
  
}

void printText(char *text, uint16_t color, int x, int y,int textSize)
{
  tft.setCursor(x, y);
  tft.setTextColor(color);
  tft.setTextSize(textSize);
  tft.setTextWrap(true);
  tft.print(text);
}

float getPH(){
    voltage = analogRead(PH_PIN) / ESPADC * ESPVOLTAGE; // read the voltage
    //Serial.print("voltage:");
    //Serial.println(voltage, 4);

    return ph.readPH(voltage, tempVal); // convert voltage to pH with temperature compensation
}

float getTemp(){
  //returns the temperature from one DS18S20 in DEG Celsius

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

void checkFoodLevel(){
  if(ir.readVoltage() > IR_THRESHOLD){
    Serial.println("LOW FOOD LEVEL!");
    foodLevel = 1;
  }
  else{
    Serial.println("Food level is good");
    foodLevel = 0;
  }
}

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
    checkFoodLevel();
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
