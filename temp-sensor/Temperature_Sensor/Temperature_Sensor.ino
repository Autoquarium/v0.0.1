#include <OneWire.h> 
#include <SoftwareSerial.h>

int DS18S20_Pin = 4; //DS18S20 Signal pin on digital 2
char tmpstring[10];

//Temperature chip i/o
OneWire ds(DS18S20_Pin);  // on digital pin 2

SoftwareSerial display(3, 2);

void setup(void) {
  Serial.begin(9600);
  display.begin(9600);
  pinMode(DS18S20_Pin, INPUT);
  display.write(254); // move cursor to beginning of first line
  display.write(128);

  display.write("                "); // clear display
  display.write("                ");
}

void loop(void) {
  float temperature = getTemp();
  int tmp = (int) temperature;

  Serial.println(tmp);

  sprintf(tmpstring, "%3d F", tmp);
  display.write(tmpstring);

  display.write(254); // move cursor to beginning of first line
  display.write(128);

  delay(200); //just here to slow down the output so it is easier to read
  
}


float getTemp(){
  //returns the temperature from one DS18S20 in DEG Celsius

  byte data[12];
  byte addr[8];

  //Serial.println(ds.search(addr));

  for(int i = 0; i < 8; ++i)
  {
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
