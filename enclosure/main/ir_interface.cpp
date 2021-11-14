#include "Arduino.h"

class ir_sensor {   
  private:
  int irPin;
  int ledPin;
  int IR_THRESHOLD;

  /*Sets LED pin on, reads voltage from ADC, sets LED pin off*/
  int readVoltage(){
    digitalWrite(ledPin, HIGH);
    delay(10);
    int returnVal = analogRead(irPin);
    delay(10);
    digitalWrite(ledPin, LOW);
    return returnVal;
  }

  public:
  ir_sensor() {
    
  }
  /*Initialize analog pin for use with IR Sensor*/
  void init(int irPin_in, int ledPin_in, int IR_THRESHOLD_in){
    IR_THRESHOLD = IR_THRESHOLD_in;
    irPin = irPin_in;
    ledPin = ledPin_in;
    pinMode(irPin, INPUT);
    pinMode(ledPin, OUTPUT);
  }


  /**
   * @brief Get the Food Level
   * 
   * @return int, 1 if full, 0 otherwise
   */
  int getFoodLevel() {
    int irVal = readVoltage();
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
};
