#include <AFMotor.h>

AF_DCMotor motor(4);

void setup() {
  Serial.begin(9600);
	//Set initial speed of the motor & stop
	motor.setSpeed(220);
	
	motor.run(RELEASE);
}

void loop() {
	if (Serial.available() > 0) {
    String data = Serial.readStringUntil('\n');

    if (data[0] == 'i') {
      Serial.println("Conveyor");
    }

    if (data[0] == '0') {
      motor.run(RELEASE);
    } else if (data[0] == '1') {
      motor.run(FORWARD);
    }
  }

}


