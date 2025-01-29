int lightPen=A0;
int lightVal;
int dv=50;

void setup() {
  pinMode(lightPen, INPUT);
  pinMode(7, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  lightVal=analogRead(lightPen);
  Serial.println(lightVal);
  digitalWrite(7, HIGH);
  delay(dv);
}
