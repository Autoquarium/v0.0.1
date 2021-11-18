/*
 * file DFRobot_ESP_PH.h * @ https://github.com/GreenPonik/DFRobot_ESP_PH_BY_GREENPONIK
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

#ifndef _DFROBOT_ESP_PH_H_
#define _DFROBOT_ESP_PH_H_

#include "Arduino.h"

#define ReceivedBufferLength 10 //length of the Serial CMD buffer

class DFRobot_ESP_PH
{
public:
    DFRobot_ESP_PH();
    ~DFRobot_ESP_PH();
   /**
     * @brief Calibrate the calibration data
     *
     * @param cmd         : ENTERPH -> enter the PH calibration mode
     *                      CALPH   -> calibrate with the standard buffer solution, two buffer solutions(4.0 and 7.0) will be automaticlly recognized
     *                      EXITPH  -> save the calibrated parameters and exit from PH calibration mode
     */
    void calibration(char *cmd); //calibration by Serial CMD
    /**
     * @brief Runs calibration sequence for pH sensor
     *        If correct command is received, enters calibration mode
     */
    void calibration();
    /**
     * @brief Runs manual calibration sequence for pH sensor
     *        If correct command is received, enters manual calibration mode.
     *        MANCALPH -> enter the PH manual calibration mode
     *        EXIT -> Exit without saving
     *        Calibration uses while loops to wait for incoming data, so it may cause other processes to misbehave.
     */
    void manualCalibration();
    /**
     * @brief Runs calibration sequence for pH sensor
     *        If correct command is received, enters calibration mode
     * 
     * @param voltage7 Voltage value of pH sensor when submerged in pH 7 buffer solution
     * @param voltage4 Voltage value of pH sensor when submerged in pH 4 buffer solution
     */
    void manualCalibration(float voltage7, float voltage4); //manually input 2-point calibration values
    /**
     * @brief Converts voltage read by the pH sensor into pH value
     *        Uses temperature measurement for a more accurate conversion
     * 
     * @param voltage Voltage value of pH sensor
     * @param temperature Temperature in degress celcius
     */
    float readPH(float voltage, float temperature); // voltage to pH value, with temperature compensation
    void begin();            	//initialization
	float get_neutralVoltage();

    
    // added below
    void init(int PH_PIN_in, float ESPADC_in, int ESPVOLTAGE_in);
    float getPH(float temp_in);

private:
    float _phValue;
    float _acidVoltage;
    float _neutralVoltage;
    float _voltage;
    float _temperature;
    
    // added below
    float ESPADC;
    int ESPVOLTAGE;
    int PH_PIN;


    char _cmdReceivedBuffer[ReceivedBufferLength]; //store the Serial CMD
    byte _cmdReceivedBufferIndex;

private:
    boolean cmdSerialDataAvailable();
    void phCalibration(byte mode); // calibration process, wirte key parameters to EEPROM
    byte cmdParse(const char *cmd);
    byte cmdParse();
};

#endif
