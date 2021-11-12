#include <ESP32Servo.h>

class Servo_Interface {
public: 

  /**
	 * @brief Constructs a new Servo_Interface object
	 * 
	 * @param pin The servo PWM signal is sent through this pin number
	 */
  void init(int pin) {
    // Allow allocation of all timers
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2);
    ESP32PWM::allocateTimer(3);
    servo.attach(pin);
    currentPos = 0;
  }

  /**
	 * @brief Rotates the motor to 180 degrees, pauses, rotates the motor to 0 degrees
	 * 
	 * @param delayIn How long the motor pauses in between rotations in milliseconds
	 */
  void fullRotation(int delayIn) {
    goToPosition(180);
    delay(delayIn);
    goToPosition(0);
  }

  /**
	 * @brief Resets the servo position to 0 degrees
	 */
  void reset() {
    goToPosition(0);
  }

  /**
	 * @brief Rotates the motor to position passed in
	 * 
	 * @param pos REQUIRES: 0 <= pos <= 180
   *            Servo motor position in degrees
	 */
  void goToPosition(int pos) {
      servo.write(pos);
      currentPos = pos;
  }

private:
  Servo servo;
  int currentPos;
};
