int lightDetector=A0;
int lightVal;
int readDelay=50;

int laser=7;

int outputLed=8;

int lightThreshold = 400;
int maxTime = 3000;
int timeSinceLastPass = 0;
int pastLightVal = 0;


void setup() {
  pinMode(lightDetector, INPUT);
  Serial.begin(9600);

  pinMode(laser, OUTPUT);
  digitalWrite(laser, LOW);
}

void loop() {
  delay(readDelay);

  lightVal=analogRead(lightDetector);
  Serial.println(timeSinceLastPass);

  //If light has passed the threshold reset timer
  if (pastLightVal < lightThreshold && lightVal > lightThreshold) {
    timeSinceLastPass = 0;
  }
  if (pastLightVal > lightThreshold && lightVal < lightThreshold) {
    timeSinceLastPass = 0;
  }

  //Error has occured as no change has happened in long time
  if (timeSinceLastPass > maxTime) {
    digitalWrite(outputLed, HIGH);
  } else {
    digitalWrite(outputLed, LOW);
  }

  pastLightVal = lightVal;
  if (timeSinceLastPass <= maxTime) { //Ensures that delay time doesn't overflow
    timeSinceLastPass += readDelay;
  }
  
}