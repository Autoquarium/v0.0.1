#include <FastLED.h>

//#define DATA_PIN 6 // for Arduino
#define DATA_PIN 22 // for ESP32

class LED_Array {
public:

  /**
	 * @brief Constructs a new LED_Array object
	 * 
	 * @param numLEDsIn Number of lights of the strip to be turned on
   *                  REQUIRES: numLEDsIn <= 300
	 */
  LED_Array(int numLEDsIn) {
    numLEDs = numLEDsIn;
    FastLED.addLeds<NEOPIXEL, DATA_PIN>(LEDs, numLEDs);

    setRGBColor(10, 10, 10);
  }

  /**
	 * @brief Turns off the LEDs by "fading to black"
	 * 
	 * @param delayIn Time it will take the LEDs to fade to black in milliseconds
   *                Affects the smoothness of the transition
	 */
  void fadeToBlack(int delayIn) {
    int itrs = 20;
    int itrDelay = (double)delayIn/(double)itrs;
    while (itrs > 0) {
      // half the brightness of all LEDs until dark
      fadeToBlackBy(LEDs, numLEDs, 64);
      FastLED.show();
      delay(itrDelay);
      --itrs;
    }
  }

  /**
	 * @brief Sets the hue/saturation/value of the lights
	 * 
	 * @param h hue (color range shown in FastLED documentation)
   *          REQUIRES: 0 <= h <= 255
	 * @param s saturation
   *          REQUIRES: 0 <= h <= 255
   * @param v value (brightness)
   *          REQUIRES: 0 <= h <= 255
	 */
  void setHSVColor(int h, int s, int v) {
    for (int i = 0; i < numLEDs; ++i) {
      LEDs[i] = CHSV(h,s,v);
    }
    FastLED.show();
  }

  /**
	 * @brief Sets the RGB color of the lights
   *        Note: the strip uses GRB by default but CRGB() handles the conversion
	 * 
	 * @param r REQUIRES: 0 <= r <= 255
   * @param g REQUIRES: 0 <= g <= 255
   * @param b REQUIRES: 0 <= b <= 255
	 */
  void setRGBColor(int r, int g, int b) {
    for (int i = 0; i < numLEDs; ++i) {
      LEDs[i] = CRGB(r,g,b);
    }
    FastLED.show();
  }

  /**
	 * @brief Transitions the color of the LEDs from RGB1 to RGB2
	 * 
	 * @param r1 REQUIRES: 0 <= r1 <= 255
   * @param g1 REQUIRES: 0 <= g1 <= 255
   * @param b1 REQUIRES: 0 <= b1 <= 255
	 * @param r2 REQUIRES: 0 <= r2 <= 255
   * @param g2 REQUIRES: 0 <= g2 <= 255
   * @param b2 REQUIRES: 0 <= b2 <= 255
   * @param delayIn Time it will take the LEDs to fade from RGB1 to RGB2 in milliseconds
   *                Affects the smoothness of the transition
	 */
  void colorTransition(int r1, int g1, int b1, int r2, int g2, int b2, int delayIn) {
    colorTransition(CRGB(r1, g1, b1), CRGB(r2, g2, b2), delayIn);
  }

  /**
	 * @brief Transitions the color of the LEDs from RGB1 to RGB2
	 * 
	 * @param color1 LEDs change to this color when called
   * @param color2 LEDs fade to this color over time delayIn
   * @param delayIn Time it will take the LEDs to fade from RGB1 to RGB2 in milliseconds
   *                Affects the smoothness of the transition
	 */
  void colorTransition(CRGB color1, CRGB color2, int delayIn) {
    int itrDelay = delayIn/(255/5);
    for (int percent = 0; percent <= 255; percent += 5) {
      for (int i = 0; i < numLEDs; ++i) {
        LEDs[i] = blend(color1, color2, percent);
      }
    FastLED.show();
    delay(itrDelay);
    }
  }

  /**
	 * @brief Transitions from startColorVal to endColorVal, holds endColorVal for delayIn, 
   *        then transitions back to startColorVal
	 * 
	 * @param startColorVal REQUIRES: 0 <= startColorVal[i] <= 255
   * @param endColorVal REQUIRES: 0 <= endColorVal[i] <= 255
   * @param delayIn Time that the LEDs will hold endColorVal in milliseconds
	 */
  void createColorCycle(int startColorVal[], int endColorVal[], int delayIn) {
    int transitionDelay = 2000; // 2 seconds
    colorTransition(startColorVal[0], startColorVal[1], startColorVal[2], 
                endColorVal[0], endColorVal[1], endColorVal[2], transitionDelay);
    delay(delayIn);
    colorTransition(endColorVal[0], endColorVal[1], endColorVal[2], 
                startColorVal[0], startColorVal[1], startColorVal[2], transitionDelay);
  }


 private:
  CRGB LEDs[300];
  int numLEDs;
};
