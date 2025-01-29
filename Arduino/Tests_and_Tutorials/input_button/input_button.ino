#define input_pin 4

void setup() {
    pinMode(input_pin, OUTPUT);
    Serial.begin(9600);
    digitalWrite(input_pin, HIGH);
}

void loop() { 
    // digitalWrite(input_pin, HIGH);
    // Serial.println("HIGH");
    // delay(3000);
    // digitalWrite(input_pin, LOW);
    // Serial.println("LOW");
    // delay(3000);
}