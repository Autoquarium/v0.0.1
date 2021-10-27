#include "Arduino.h"

class ir_sensor {
  private:
  int irPin;
  int ledPin;
  public:
  ir_sensor(){
    
  }
  /*Initialize analog pin for use with IR Sensor*/
  void init(int irPin, int ledPin){
    //TODO: Make sure this is an analog pin?
    pinMode(irPin, INPUT);
    pinMode(ledPin, OUTPUT);
  }
  /*Sets LED pin on, reads voltage from ADC, sets LED pin off*/
  int readVoltage(){
    digitalWrite(ledPin, HIGH);
    delay(10);
    return analogRead(irPin);
    delay(10);
    digitalWrite(ledPin, LOW);
  }
};
