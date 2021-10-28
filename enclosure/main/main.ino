#include "Arduino.h"
#include "DFRobot_ESP_PH.h"
#include "ir_interface.cpp"
#include "Servo_interface.h"

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
#define IR_PIN 2 //TODO change to ESP pins
#define LED_PIN 1 //TODO change to ESP pins
#define IR_THRESHOLD 50 //TODO change to reflect values in enclosure
ir_sensor ir;

//Temperature chip
int DS18S20_Pin = 4; //DS18S20 Signal pin on digital 2
volatile float tempVal;
OneWire ds(DS18S20_Pin);  // on digital pin 2

//Servo
#define SERVO_PIN 9
#defin DELAY_BETWEEN_ROTATION 1000

//FUNCTION PROTOTYPES
void updatePHReading(int temperature_in);
float getTemp();
void checkForMoveServo();
void checkFoodLevel();

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
}

void loop() {
  unsigned long current_time = millis();


  //read from temp sensor every interval
  if (current_time - temp_previous_time >= temp_interval) {
    temp_previous_time = current_time;
    //read from temp sensor
    tempVal = getTemp();
    Serial.println("Temp sensor: %d", tempVal);
  }

  //read from ph sensor every interval
  if (current_time - ph_previous_time >= ph_interval) {
    ph_previous_time = current_time;
    //read from ph sensor using temp sensor values
    updatePHReading();
    Serial.println("pH sensor: %d", phVal);
  }

  //if wireless command received, move servo (Serial Command: MOVESERVO)
  //TODO: change from parsing serial to parsing wireless message
  checkForMoveServo();
  
}

void updatePHReading(){
    voltage = analogRead(PH_PIN) / ESPADC * ESPVOLTAGE; // read the voltage
    //Serial.print("voltage:");
    //Serial.println(voltage, 4);

    pHVal = ph.readPH(voltage, tempVal); // convert voltage to pH with temperature compensation
    //Serial.print("pH:");
    //Serial.println(phValue, 4);
}

float getTemp(){
  //returns the temperature from one DS18S20 in DEG Celsius

  byte data[12];
  byte addr[8];

  Serial.println(ds.search(addr));

  for (int i = 0; i < 8; ++i) {
    Serial.print(addr[i]);
  }
  
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
      Serial.print("Device is not recognized");
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
    //move servo once
    si.fullRotation(1000);
    Serial.println("Servo moved!");
    delay(DELAY_BETWEEN_ROTATION); //delay in between rotations

    //check for low food level
    checkFoodLevel();
  }
}

void checkFoodLevel(){
  if(ir.readVoltage() > IR_THRESHOLD){
    Serial.print("LOW FOOD LEVEL!");
  }
}
