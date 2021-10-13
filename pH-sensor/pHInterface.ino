//Go to {Tools >> Manage Libraries...} and download DFRobot_ESP_PH_WITH_ADC_BY_GREENPONIK Library v1.2.3
#include "Arduino.h"
#include "DFRobot_ESP_PH_WITH_ADC.h"
#include "EEPROM.h"


#define ESP_PH_PIN 1 //TODO Change this later for ESP
DFRobot_ESP_PH_WITH_ADC ph;

class pH_Interface{
public:

  void init(){
    ph.begin();
  }
  
  // takes in temperature value from temperature sensor (in degrees C)
  // reads in the voltage from the pH sensor and 
  // converts it to a value on the pH scale
  void updatePHReading(int temperature){
    pH_val = ph.readPH((analogRead(ESP_PH_PIN)), temperature); //TODO: Change values in readPH function according to calibrated values
    Serial.print("pH:");
    Serial.print(pH_val, 1);
  }

  // reads the private pH member variable
  double getPHValue(){
    return pH_avg;
  }

private:
  // internally stored pH
  double pH_val;
}
pH_Interface ph_sensor;

void setup() {
  Serial.begin(115200);
  ph_sensor.init();

}

void loop() {
  // put your main code here, to run repeatedly:
  static unsigned long timepoint = millis();
  if(millis()-timepoint>5000U){ //read sensor every 5 seconds
    ph_sensor.updatePHReading(25);
    ph_sensor.getPHValue();
  }
}
