/* 
 * https://github.com/adafruit/Adafruit-GFX-Library
 * https://github.com/adafruit/Adafruit_ILI9341
 * https://github.com/adafruit/Adafruit_BusIO 
*/

#include <Adafruit_GFX.h> 
#include <Adafruit_ILI9341.h> 
#include  <SPI.h>


class LCD {

	private:
		// LCD properties object
		Adafruit_ILI9341 *tft;

		// Colors for LCD Display
		const int black = 0x0000;
		const int white = 0xFFFF;  // RGB
		const int red = 0xF800;  // R
		const int green = 0x3606;  // G
		const int water_blue = 0x033F6;  // B
		const int yellow  = 0xFFE0;  // RG
		const int cyan  = 0x07FF;  // GB
		const int magenta = 0xF81F;  // RB
		const int gray  = 0x0821;  // 00001 000001 00001
		const int orange = 0xFB46;

		/**
		 * @brief
		 */
		void printText(String text, uint16_t color, int x, int y,int textSize) {
			tft->setCursor(x, y);
			tft->setTextSize(textSize);
			tft->setTextWrap(true);
			tft->setTextColor(color);
			tft->print(text);
		}

	public:

		/**
		 * @brief Construct a new LCD object
		 * 
		 */
		LCD() { }


		/**
		 * @brief init the LCD screen
		 * 
		 * @param TFT_CS pin
		 * @param TFT_DC pin
		 * @param TFT_MOSI master out, slave in pin
		 * @param TFT_CLK clock pin
		 * @param TFT_RST reset pin
		 * @param TFT_MISO multiple input single output pin
		 */
		void init(int TFT_CS, int TFT_DC, int TFT_MOSI, int TFT_CLK, int TFT_RST, int TFT_MISO) {
			tft = new Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);
			tft->begin();                      
			tft->setRotation(0);            
			tft->fillScreen(ILI9341_BLACK);
	
			printText("AUTOQUARIUM", water_blue,20,20,3);
			printText("pH", white,25,90,3);
			printText("Temp", white,150,90,3);
			printText("Food", white,25,180,3);
			printText("Fish", white,150,180,3);
		}


		/**
		 * @brief Updates LCD screen with data
		 * 
		 * @param tempVal Temperature sensor measurement
		 * @param pHVal pH sensor measurement
		 * @param foodLevel Auto-feed module food level status
		 * @param numFish Number of fish in the tank
		 */
		void updateLCD(float tempVal, float pHVal, int foodLevel, int numFish) {
			tft->fillScreen(ILI9341_BLACK);
			printText("AUTOQUARIUM", water_blue,20,20,3);
			printText("pH", white,25,90,3);
			printText("Temp", white,150,90,3);
			printText("Food", white,25,180,3);
			printText("Fish", white,150,180,3);

			if(pHVal == 7) {
				printText((String)pHVal, green, 30, 120, 3);
			}
			else if(pHVal < 6.5 || pHVal > 7.5) {
				printText((String)pHVal, red, 30, 120, 3);
			}
			else {
				printText((String)pHVal, orange, 30, 120, 3);
			}

			if(tempVal < 23 || tempVal > 27) {
				printText((String)tempVal, red,160,120,3);
			}
			else {
				printText((String)tempVal, green,160,120,3);
			}

			// food value
			if(foodLevel == 1) {
				printText("Good", green,30,210,3);
			}
			else {
				printText("Low", red,30,210,3);
			}

			// Num Fish
			printText((String)numFish, green,160,210,3);
		}
};
