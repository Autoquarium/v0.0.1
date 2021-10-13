
class ir_sensor {
  private:
  int irSensorPin;
  int ledPin
  public:
  ir_sensor(){
    
  }
  /*Initialize analog pin for use with IR Sensor*/
  void init(int irPin, int ledPin){
    //TODO: Make sure this is an analog pin?
    pinMode(irPin, INPUT);
    pinMode(ledPin, OUTPUT);
  }
  /*Sets LED pin on, reads voltage from ADC, sets LED pin off*/
  int readVoltage(){
    digitalWrite(ledPin, HIGH);
    delay(10);
    return analogRead(irPin);
    delay(10);
    digitalWrite(ledPin, LOW);
  }
}

int value;
void setup() {
  // put your setup code here, to run once:
  ir_sensor ir;
  //TODO: change these later when using ESP32
  ir.init(A0, 11);

  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  value = ir.readVoltage();
  Serial.println(value);
  if(value > 50){
    digitalWrite(13, HIGH);
  }
  else{
    digitalWrite(13, LOW);
  }
}
