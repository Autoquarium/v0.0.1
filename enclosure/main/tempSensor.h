// LCD libraries
#include <OneWire.h> 

class TempSensor {

  private:
    OneWire *ds;
  
  public:
    TempSensor(){
    
    }
  
    /**
      * @brief initializes the OneWire object 
      * 
      * @param pin Sets the pin that sensor reads from
    */
    void init(int pin)
    {
      pinMode(pin, INPUT);
      ds = new OneWire(pin);
    } 
  
    /**
      * @brief Get the current reading from the temperature sensor
      * 
      * @return float value of the temperature in Celsius 
      */
    float getTemp()
    {
      byte data[12];
      byte addr[8];

      for(int i = 0; i < 8; ++i)
      {
        Serial.print(addr[i]);
      }
  
      if ( !ds->search(addr)) {
        //no more sensors on chain, reset search
        ds->reset_search();
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

      ds->reset();
      ds->select(addr);
      ds->write(0x44,1); // start conversion, with parasite power on at the end

      byte present = ds->reset();
      ds->select(addr);    
      ds->write(0xBE); // Read Scratchpad

      for (int i = 0; i < 9; i++) { // we need 9 bytes
        data[i] = ds->read();
      }

      ds->reset_search();

      byte MSB = data[1];
      byte LSB = data[0];

      float tempRead = ((MSB << 8) | LSB); //using two's compliment
      float TemperatureSum = tempRead / 16;

      return (TemperatureSum * 18 + 5)/10 + 32;
    }
};
