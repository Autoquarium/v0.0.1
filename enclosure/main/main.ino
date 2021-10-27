#include "Arduino.h"
#include "DFRobot_ESP_PH.h"
#include "ir_interface.cpp"

//PINS
#define PH_PIN 35    //the esp gpio data pin number

//VARIABLES
#define ESPADC 4096.0   //the esp Analog Digital Convertion value
#define ESPVOLTAGE 3300 //the esp voltage supply value
#define MSTOSECS 1000
float voltage, pHVal, tempVal = 25;
unsigned long ph_previous_time = 0, temp_previous_time = 0;
long ph_interval = 5*MSTOSECS;//*60*10; //10 minutes
long temp_interval = 5*MSTOSECS;//*60*10; //10 minutes
String msg_in;

//OBJECTS
DFRobot_ESP_PH ph;
ir_sensor ir;

//FUNCTION PROTOTYPES
void updatePHReading(int temperature_in);

void setup() {
  // put your setup code here, to run once:
  
  // init ph sensor
  Serial.begin(115200);
  ph.begin();
}

void loop() {
  unsigned long current_time = millis();


  //read from temp sensor every interval
  if(current_time - temp_previous_time >= temp_interval) {
    temp_previous_time = current_time;
    //read from temp sensor
    Serial.println("Temp sensor read!");
  }

  //read from ph sensor every interval
  if (current_time - ph_previous_time >= ph_interval) {
    ph_previous_time = current_time;
    //read from ph sensor using temp sensor values
    Serial.println("pH sensor read!");
  }

  //if wireless command received, move servo (Command: MOVESERVOX, where X is number of times to move servo)
  //TODO: change from parsing serial to parsing wireless message
  if (Serial.available() > 0){
    msg_in = Serial.readString();
  }
  if(msg_in == "MOVESERVO\n"){
      unsigned int num_feed = (unsigned int)msg_in[9];
      Serial.println("Servo moved!");
      //make sure to add in enough delay in between rotations
      msg_in = "";
  }
  
}

void updatePHReading(int temperature_in){
    voltage = analogRead(PH_PIN) / ESPADC * ESPVOLTAGE; // read the voltage
    //Serial.print("voltage:");
    //Serial.println(voltage, 4);
    
    tempVal = temperature_in;  // read your temperature sensor to execute temperature compensation
    //Serial.print("temperature:");
    //Serial.print(temperature, 1);
    //Serial.println("^C");

    pHVal = ph.readPH(voltage, tempVal); // convert voltage to pH with temperature compensation
    //Serial.print("pH:");
    //Serial.println(phValue, 4);
}
