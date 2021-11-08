// LCD libraries
#include "Adafruit_GFX.h"     
#include "Adafruit_ILI9341.h" 
#include <Wire.h>
#include  <SPI.h>



// Colors for LCD Display
#define black  0x0000  // 
#define white 0xFFFF  // RGB
#define red 0xF800  // R
#define green 0x3606  // G
#define water_blue 0x033F6  // B
#define yellow  0xFFE0  // RG
#define cyan  0x07FF  // GB
#define magenta 0xF81F  // RB
#define gray  0x0821  // 00001 000001 00001
#define orange 0xFB46


class LCD {

private:
	Adafruit_ILI9341 tft;


	void printText(String text, uint16_t color, int x, int y,int textSize) {
	  tft.setCursor(x, y);
	  tft.setTextSize(textSize);
	  tft.setTextWrap(true);
	  tft.setTextColor(color);
	  tft.print(text);
	}


public:
	LCD(int TFT_CS, int TFT_DC, int TFT_MOSI, int TFT_CLK, int TFT_RST, int TFT_MISO) {
		tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);
		tft.begin();                      
		tft.setRotation(3);            
		tft.fillScreen(ILI9341_BLACK);
  
		Wire.begin();
		printText("AUTOQUARIUM", water_blue,30,20,4);
		printText("pH", white,40,70,3);
		printText("Temp", white,200,70,3);
		printText("Food", white,30,150,3);
		printText("Fish", white,200,150,3);
	}



	void updateLCD(float tempVal, float pHVal, int foodLevel) {
	    // TODO: move all this LCD stuff into a new function
	    tft.fillScreen(ILI9341_BLACK);
	    printText("AUTOQUARIUM", water_blue,30,20,4);
	    printText("pH", white,40,70,3);
	    printText("Temp", white,200,70,3);
	    printText("Food", white,30,150,3);
	    printText("Fish", white,200,150,3);
	    /*printText((String)prevPHVal, black, 40, 100, 3);
	    printText((String)prevTempVal, black, 200, 100, 3);
	    printText((String)prevFoodLevel, black, 30, 180, 3);
	    printText((String)prevNumFish, black, 200, 180, 3);*/
	    if(pHVal == 7)
	    {
	      printText((String)pHVal, green, 40, 100, 3);
	    }
	    
	    else if(pHVal < 6.5 || pHVal > 7.5)
	    {
	      printText((String)pHVal, red,40,100,3);
	    }
	    
	    else
	    {
	      printText((String)pHVal, orange,40,100,3);
	    }
	    
	    if(tempVal < 23 || tempVal > 27)
	    {
	      printText((String)tempVal, red,200,100,3);
	    }
	    
	    else
	    {
	      printText((String)tempVal, green,200,100,3);
	    }
	    
	    // food value
	    if(foodLevel == 0)
	    {
	      printText("Good", green,30,180,3);
	    }
	    else
	    {
	      printText("Low", red,30,180,3);
	    }
	    
	    // Num Fish TODO
	    printText("5", green,200,180,3);
	}


};