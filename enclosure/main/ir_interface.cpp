#include "Arduino.h"

class ir_sensor {   
  private:
    int irPin;
    int ledPin;
    int IR_THRESHOLD;

    /*Sets LED pin on, reads voltage from ADC, sets LED pin off*/
    int readVoltage(){
      int returnVal = analogRead(irPin);
      return returnVal;
    }

  public:
    /**
     * @brief Construct a new ir sensor object
     * 
     */
    ir_sensor() {}

    
    /**
     * @brief Initialize analog pin for use with IR Sensor
     * 
     * @param irPin_in pin the IR photodiode is connected to
     * @param IR_THRESHOLD_in cutoff for object detection
     */
    void init(int irPin_in, int IR_THRESHOLD_in){
      IR_THRESHOLD = IR_THRESHOLD_in;
      irPin = irPin_in;
      pinMode(irPin, INPUT);
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
