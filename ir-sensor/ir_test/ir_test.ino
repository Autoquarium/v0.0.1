class ir_sensor {
  public:
  ir_sensor(){
    
  }
  /*Initialize analog pin for use with IR Sensor*/
  void init(int irPin_in, int ledPin_in){
    irPin = irPin_in;
    ledPin = ledPin_in;
    pinMode(irPin, INPUT);
    pinMode(ledPin, OUTPUT);
  }
  /*Sets LED pin on, reads voltage from ADC, sets LED pin off*/
  int readVoltage(){
    digitalWrite(ledPin, HIGH);
    delay(10);
    int returnVal = analogRead(irPin);
    delay(10);
    digitalWrite(ledPin, LOW);
    return returnVal;
  }
   
  private:
  int irPin;
  int ledPin;
};

ir_sensor ir;

void setup() {
  // put your setup code here, to run once:
  ir.init(34, 23);
  Serial.begin(115200);
}

int value;
void loop() {
  // put your main code here, to run repeatedly:
  value = ir.readVoltage();
  Serial.println(value);
}
