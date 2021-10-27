
// TODO: rewrite/test interface for new servo motor. 
class Servo_Interface {
public: 
  // Initialize the servo - used barebones because Servo.h was inaccurate
  void initServo(int pin) {
    pinMode(pin, OUTPUT);
    signalPin = pin;
    TCCR2A = _BV(COM2A1) | _BV(WGM21) | _BV(WGM20);
    TCCR2B = _BV(CS22) | _BV(CS21) | _BV(CS20);

    currentPos = 0;
  }

  // turns at speed in direction dir for one rotation
  // char dir should be 'l' or 'r'
  // int speed should be in the range [1, 8]
  void fullRotation(char dir, int speed) {
    
    if (dir == 'l') { // counterclockwise
      OCR2A = restPosition + speed;
    }
    else { // clockwise
      OCR2A = restPosition - speed;
    }
    delay(1780);
    stopServo();
  }

  //Stops servo from spinning 
  void stopServo() {
    OCR2A = restPosition; 
  }

  //resets servo position to zero in the quickest rotation direction - impossible with continuous servo
  void reset();

  // moves servo to the desired position by specified direction and speed - unreliable with continuous servo
  void goToPosition(int pos, char direction, int speed);

private:
  //Required Pulse: 900us-2100us
  const int leftFullSpeed = 15; // pulse width is 980us with 60Hz CLK 
  const int rightFullSpeed = 31; // pulse width 2030us with 60Hz CLK 
  const int restPosition = 23; // pulse width 1500us
  
  int currentPos;
  int signalPin;
};

Servo_Interface si;

void setup() {

  si.initServo(11);
}

void loop() {

  si.fullRotation('l', 8);
  delay(1000);

}
