#include <SoftwareSerial.h>

const byte HC12TxdPin = 5;
const byte HC12RxdPin = 4;
const byte HC12SetPin = 6;

SoftwareSerial HC12(HC12TxdPin, HC12RxdPin);

void setup() {
  pinMode(HC12SetPin, OUTPUT);
  digitalWrite(HC12SetPin, LOW); // ENTER AT MODE

  Serial.begin(9600);
  HC12.begin(9600);
  delay(100);

  Serial.println("Enter AT commands:");
}

void loop() {
  if (Serial.available()) {
    HC12.write(Serial.read());
  }
  if (HC12.available()) {
    Serial.write(HC12.read());
  }
}
