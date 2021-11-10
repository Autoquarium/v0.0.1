#include <ESP32Servo.h>

class Servo_Interface {
public: 

  void initServo(int pin) {
    // Allow allocation of all timers
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2);
    ESP32PWM::allocateTimer(3);
    servo.attach(pin);
    currentPos = 0;
  }

  // rotates to 180 degrees and pauses for delayIn ms
  void fullRotation(int delayIn) {
    servo.write(180);
    currentPos = 180;
    delay(delayIn);
    servo.write(0);
    delay(delayIn);

  }

  //resets servo position to zero in the quickest rotation direction
  void reset() {
      servo.write(0);
  }

  // moves servo to the desired position by specified direction and speed - unreliable with continuous servo
  void goToPosition(int pos) {
      servo.write(pos);
      currentPos = pos;
  }

private:
  Servo servo;
  int currentPos;
};
