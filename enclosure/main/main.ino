<<<<<<< Updated upstream
#include "Arduino.h"
#include "DFRobot_ESP_PH.h"
#include "ir_interface.cpp"
#include "Servo_interface.h"
#include "LED_Array.h"
#include <OneWire.h>

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
#define IR_PIN 26 //TODO change to ESP pins
#define LED_PIN 22 //TODO change to ESP pins
#define IR_THRESHOLD 50 //TODO change to reflect values in enclosure
ir_sensor ir;

//Temperature chip
int DS18S20_Pin = 4; //DS18S20 Signal pin on digital 4
volatile float tempVal;
OneWire ds(DS18S20_Pin);  // on digital pin 4

//Servo
#define SERVO_PIN 32
#define DELAY_BETWEEN_ROTATION 1000
Servo_Interface si;

//LED array
LED_Array leds;
char currLEDcolor = 'W';

//FUNCTION PROTOTYPES
void updatePHReading(int temperature_in);
float getTemp();
void checkForMoveServo();
void checkFoodLevel();
void checkForChangeLED();

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
    updatePHReading();
    Serial.print("pH sensor: ");
    Serial.println(pHVal);
  }

  //if wireless command received, move servo (Serial Command: MOVESERVO)
  //TODO: change from parsing serial to decoding wireless message
  checkForMoveServo();

  //if wireless command received, change color of leds
  //TODO: change from parsing serial to decoding wireless message
  checkForChangeLED();
  
}

void updatePHReading(){
    voltage = analogRead(PH_PIN) / ESPADC * ESPVOLTAGE; // read the voltage
    //Serial.print("voltage:");
    //Serial.println(voltage, 4);

    pHVal = ph.readPH(voltage, tempVal); // convert voltage to pH with temperature compensation
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

void checkForMoveServo(){
  String msg_in;

  //TODO: change to parsing wireless message
  if (Serial.available() > 0) {
    msg_in = Serial.readString();
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
}

void checkFoodLevel(){
  int irVal = ir.readVoltage();
  Serial.println(irVal);
  if(irVal > IR_THRESHOLD){
    Serial.println("LOW FOOD LEVEL!");
  }
  else{
    Serial.println("Food level is good");
  }
}

void checkForChangeLED(){
    String msg_in;

  //TODO: change to parsing wireless message
  if (Serial.available() > 0) {
    msg_in = Serial.readString();
  }
  
  if (msg_in == "CHANGELED\n") {
    Serial.println("Change the LED!");
    if (currLEDcolor == 'B') {
      leds.colorTransition(CRGB::Blue, CRGB::White, 5000);
    }
    else if (currLEDcolor == 'W') {
      leds.colorTransition(CRGB::White, CRGB::Blue, 5000);
    }
    
  }
}