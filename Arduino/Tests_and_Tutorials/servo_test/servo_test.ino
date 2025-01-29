#include <Servo.h>

int servoPin = 11;
int servoPos = 0;
Servo myServo;
void setup() {
  Serial.begin(9600);
  myServo.attach(servoPin);
  
}

void loop() {
  myServo.write(25);
}