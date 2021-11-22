/*
 * file DFRobot_ESP_PH.cpp * @ https://github.com/GreenPonik/DFRobot_ESP_PH_BY_GREENPONIK
 *
 * Arduino library for Gravity: Analog pH Sensor / Meter Kit V2, SKU: SEN0161-V2
 * 
 * Based on the @ https://github.com/DFRobot/DFRobot_PH
 * Copyright   [DFRobot](http://www.dfrobot.com), 2018
 * Copyright   GNU Lesser General Public License
 *
 * ##################################################
 * ##################################################
 * ########## Fork on github by GreenPonik ##########
 * ############# ONLY ESP COMPATIBLE ################
 * ##################################################
 * ##################################################
 * 
 * version  V1.0
 * date  2019-05
 */

#include "Arduino.h"
#include "DFRobot_ESP_PH.h"
#include <Preferences.h>

//#define PHVALUEADDR 0x00 //the start address of the pH calibration parameters stored in the EEPROM

#define PH_8_VOLTAGE 1122
#define PH_6_VOLTAGE 1478
#define PH_5_VOLTAGE 1654
#define PH_3_VOLTAGE 2010

Preferences preferences;

/**
 * @brief Initializes the pH sensor/hardware and assigns it the proper pins
 * 
 * @param PH_PIN_in Input for pH sensor on ESP32
 * @param ESPADC_in Input for ADC from ESP32
 * @param ESPVOLTAGE_in Input for ESP32 Voltage source
 */
void DFRobot_ESP_PH::init(int PH_PIN_in, float ESPADC_in, int ESPVOLTAGE_in) {
    PH_PIN = PH_PIN_in;
    ESPADC = ESPADC_in;
    ESPVOLTAGE = ESPVOLTAGE_in;
}

/**
 * @brief Retrieves the pH value of the tank/solution
 * 
 * @param temp_in Temperature of tank(in Celsius?)
 * @return float
 */
float DFRobot_ESP_PH::getPH(float temp_in) {
    float voltage = analogRead(PH_PIN) / ESPADC * ESPVOLTAGE; // read the voltage
    this->_voltage = voltage;
    this->_temperature = temp_in;
    return readPH(voltage, temp_in); // convert voltage to pH with temperature compensation
}

/**
 * @brief Constructor that assigns default(neutral) values to pH sensor metrics
 * 
 */
DFRobot_ESP_PH::DFRobot_ESP_PH()
{
    this->_temperature = 25.0;
    this->_phValue = 7.0;
    this->_acidVoltage = 2032.44;   //buffer solution 4.0 at 25C
    this->_neutralVoltage = 1500.0; //buffer solution 7.0 at 25C
    this->_voltage = 1500.0;
}

/**
 * @brief Destructor
 * 
 */
DFRobot_ESP_PH::~DFRobot_ESP_PH()
{
}

/**
 * @brief Gets the neutral voltage of tank/solution
 * 
 * @return float 
 */
float DFRobot_ESP_PH::get_neutralVoltage(){
	return this->_neutralVoltage;
}

/**
 * @brief This is the startup function for the pH sensor. It gets everything ready so that the sensor can actually start to calibrate/read values
 * 
 */
void DFRobot_ESP_PH::begin()
{
	preferences.begin("pHVals", false);
    //check if calibration values (neutral and acid) are stored in eeprom
    this->_neutralVoltage = preferences.getFloat("voltage7", 0); //load the neutral (pH = 7.0)voltage of the pH board from the EEPROM
    if (this->_neutralVoltage == 0)
    {
        this->_neutralVoltage = 1500.0; // new EEPROM, write typical voltage
        preferences.putFloat("voltage7", this->_neutralVoltage);
    }

    this->_acidVoltage = preferences.getFloat("voltage4", 0); //load the acid (pH = 4.0) voltage of the pH board from the EEPROM
    if (this->_acidVoltage == 0)
    {
        this->_acidVoltage = 2032.44; // new EEPROM, write typical voltage
        preferences.putFloat("voltage4", this->_acidVoltage);
    }
	preferences.end();
}

/**
 * @brief Reads pH level of a given solution/water in fish tank
 * 
 * @param voltage // TODO
 * @param temperature temperature of the water
 * @return float 
 */
float DFRobot_ESP_PH::readPH(float voltage, float temperature) {

    float slope = (7.0 - 4.0) / ((this->_neutralVoltage - 1500.0) / 3.0 - (this->_acidVoltage - 1500.0) / 3.0);
    float intercept = 7.0 - slope * (this->_neutralVoltage - 1500.0) / 3.0;
    this->_phValue = slope * (voltage - 1500.0) / 3.0 + intercept;
    return _phValue;
}

/**
 * @brief calibrates the pH sensor given a remote command
 * 
 * @param cmd calibration command
 */
void DFRobot_ESP_PH::calibration(char *cmd) {
    strupr(cmd);

    // if received Serial CMD from the serial monitor, enter into the calibration mode
    phCalibration(cmdParse(cmd));
}


/**
 * @brief tries to search a remote command and calibrates the pH sensor if found
 * 
 */
void DFRobot_ESP_PH::calibration() {
    if (cmdSerialDataAvailable() > 0)
    {
      if(strstr(this->_cmdReceivedBuffer, "MANCALPH") != NULL){
        int v7 = 0, v4 = 0;
        Serial.println("Manual Calibration: Please enter the voltage value for pH 4");
        while(cmdSerialDataAvailable() <= 0){}
        if(strstr(this->_cmdReceivedBuffer, "EXIT") != NULL){
          return;
        }
        else{
          v7 = atoi(_cmdReceivedBuffer);
        }
        Serial.println("Manual Calibration: Please enter the voltage value for pH 7");
        while(cmdSerialDataAvailable() <= 0){}
        if(strstr(this->_cmdReceivedBuffer, "EXIT") != NULL){
          return;
        }
        else{
          v4 = atoi(_cmdReceivedBuffer);
        }
        manualCalibration(v7, v4);
      }
      else{
          phCalibration(cmdParse()); // if received Serial CMD from the serial monitor, enter into the calibration mode
      }
    }
}
/**
 * @brief manual calibration function
 * 
 */
void DFRobot_ESP_PH::manualCalibration() {
  
  if(cmdSerialDataAvailable() > 0){
    
  }
}

/**
 * @brief checks to see whether serial data is/is not available
 * 
 * @return boolean True if data is available, False otherwise
 */
boolean DFRobot_ESP_PH::cmdSerialDataAvailable()
{
    char cmdReceivedChar;
    static unsigned long cmdReceivedTimeOut = millis();
    while (Serial.available() > 0)
    {
        if (millis() - cmdReceivedTimeOut > 500U)
        {
            this->_cmdReceivedBufferIndex = 0;
            memset(this->_cmdReceivedBuffer, 0, (ReceivedBufferLength));
        }
        cmdReceivedTimeOut = millis();
        cmdReceivedChar = Serial.read();
        if (cmdReceivedChar == '\n' || this->_cmdReceivedBufferIndex == ReceivedBufferLength - 1)
        {
            this->_cmdReceivedBufferIndex = 0;
            strupr(this->_cmdReceivedBuffer);
            return true;
        }
        else
        {
            this->_cmdReceivedBuffer[this->_cmdReceivedBufferIndex] = cmdReceivedChar;
            this->_cmdReceivedBufferIndex++;
        }
    }
    return false;
}

/**
 * @brief parses a remote command
 * 
 * @param cmd input command
 * @return byte index mode
 */
byte DFRobot_ESP_PH::cmdParse(const char *cmd) {
    byte modeIndex = 0;
    if (strstr(cmd, "ENTERPH") != NULL)
    {
        modeIndex = 1;
    }
    else if (strstr(cmd, "EXITPH") != NULL)
    {
        modeIndex = 3;
    }
    else if (strstr(cmd, "CALPH") != NULL)
    {
        modeIndex = 2;
    }
    return modeIndex;
}

/**
 * @brief recieves a command and parses it
 * 
 * @return byte index mode
 */
byte DFRobot_ESP_PH::cmdParse() {
    byte modeIndex = 0;
    if (strstr(this->_cmdReceivedBuffer, "ENTERPH") != NULL)
    {
        modeIndex = 1;
    }
    else if (strstr(this->_cmdReceivedBuffer, "EXITPH") != NULL)
    {
        modeIndex = 3;
    }
    else if (strstr(this->_cmdReceivedBuffer, "CALPH") != NULL)
    {
        modeIndex = 2;
    }
    return modeIndex;
}

/**
 * @brief Calibrates pH sensor based on provided mode
 * 
 * @param mode the mode from cmdparse
 */
void DFRobot_ESP_PH::phCalibration(byte mode) {
    char *receivedBufferPtr;
    static boolean phCalibrationFinish = 0;
    static boolean enterCalibrationFlag = 0;
    switch (mode)
    {
    case 0:
        if (enterCalibrationFlag)
        {
            Serial.println(F(">>>Command Error<<<"));
        }
        break;

    case 1:
        enterCalibrationFlag = 1;
        phCalibrationFinish = 0;
        Serial.println();
        Serial.println(F(">>>Enter PH Calibration Mode<<<"));
        Serial.println(F(">>>Please put the probe into the 4.0 or 7.0 standard buffer solution<<<"));
        Serial.println();
        break;

    case 2:
        if (enterCalibrationFlag)
        {
            if ((this->_voltage > PH_8_VOLTAGE) && (this->_voltage < PH_6_VOLTAGE))
            { // buffer solution:7.0
                Serial.println();
                Serial.print(F(">>>Buffer Solution:7.0"));
                this->_neutralVoltage = this->_voltage;
                Serial.println(F(",Send EXITPH to Save and Exit<<<"));
                Serial.println();
                phCalibrationFinish = 1;
            }
            else if ((this->_voltage > PH_5_VOLTAGE) && (this->_voltage < PH_3_VOLTAGE))
            { //buffer solution:4.0
                Serial.println();
                Serial.print(F(">>>Buffer Solution:4.0"));
                this->_acidVoltage = this->_voltage;
                Serial.println(F(",Send EXITPH to Save and Exit<<<"));
                Serial.println();
                phCalibrationFinish = 1;
            }
            else
            {
                Serial.println();
                Serial.print(F(">>>Buffer Solution Error Try Again<<<"));
                Serial.println(); // not buffer solution or faulty operation
                phCalibrationFinish = 0;
            }
        }
        break;

    case 3://store calibration value in eeprom
        if (enterCalibrationFlag)
        {
            Serial.println();
			preferences.begin("pHVals", false);
            if (phCalibrationFinish)
            {
                if ((this->_voltage > PH_8_VOLTAGE) && (this->_voltage < PH_5_VOLTAGE))
                {
                    preferences.putFloat("voltage7", this->_neutralVoltage);
					Serial.print(F("PH 7 Calibration value SAVE THIS FOR LATER: "));
					Serial.print(this->_neutralVoltage);
                }
                else if ((this->_voltage > PH_5_VOLTAGE) && (this->_voltage < PH_3_VOLTAGE))
                {
                    preferences.putFloat("voltage4", this->_acidVoltage);
					Serial.print(F("PH 4 Calibration value SAVE THIS FOR LATER: "));
					Serial.print(this->_acidVoltage);
                }
                Serial.print(F(">>>Calibration Successful"));
            }
            else
            {
                Serial.print(F(">>>Calibration Failed"));
            }
			preferences.end();
            Serial.println(F(",Exit PH Calibration Mode<<<"));
            Serial.println();
            phCalibrationFinish = 0;
            enterCalibrationFlag = 0;
        }
        break;
    }
}

/**
 * @brief Manually calibrate the pH sensor 
 * 
 * @param voltage7 voltage at pH 7
 * @param voltage4 voltage at pH 4
 */
void DFRobot_ESP_PH::manualCalibration(float voltage7, float voltage4){
	preferences.begin("pHVals", false);
	
	preferences.putFloat("voltage7", this->_neutralVoltage);
	Serial.println(F("PH 7 Calibration value saved"));
	preferences.putFloat("voltage4", this->_acidVoltage);
	Serial.println(F("PH 4 Calibration value saved"));
	
	preferences.end();
}
