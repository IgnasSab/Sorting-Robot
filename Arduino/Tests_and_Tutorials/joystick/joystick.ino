#define VRX_PIN  A1 // Arduino pin connected to VRX pin
#define VRY_PIN  A2 // Arduino pin connected to VRY pin
#define button 4

int xValue = 0; // To store value of the X axis
int yValue = 0; // To store value of the Y axis
int button_value;
void setup() {
  Serial.begin(9600);
  pinMode(VRY_PIN, INPUT);
  pinMode(VRX_PIN, INPUT);
  pinMode(button, INPUT);
  digitalWrite(button, HIGH);
}

void loop() {
  // read analog X and Y analog values
  xValue = analogRead(VRX_PIN);
  yValue = analogRead(VRY_PIN);
  button_value = digitalRead(button);

  // print data to Serial Monitor on Arduino IDE
  Serial.print("x = ");
  Serial.print(xValue);
  Serial.print(", y = ");
  Serial.print(yValue);
  Serial.print(", button = ");
  Serial.println(button_value);
  delay(200);
}