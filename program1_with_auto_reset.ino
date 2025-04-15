#include <SoftwareSerial.h>

const byte HC12TxdPin = 5;
const byte HC12RxdPin = 4;
const byte HC12SetPin = 6;

SoftwareSerial HC12(HC12TxdPin, HC12RxdPin);

void setup() {
  Serial.begin(9600);
  pinMode(HC12SetPin, OUTPUT);hi

  // Step 1: Enter AT mode
  digitalWrite(HC12SetPin, LOW);     // Pull SET low
  delay(50);                         // Short wait

  HC12.begin(9600);                  // HC-12 default baud for AT commands
  HC12.println("AT+B9600");          // Example: set baud to 9600
  delay(100);                        // Give HC-12 time to respond

  // Read back any response
  while (HC12.available()) {
    Serial.write(HC12.read());       // Should print "OK"
  }

  // Step 2: Exit AT mode
  digitalWrite(HC12SetPin, HIGH);    // Back to normal mode
  delay(50);

  // Step 3: Start normal communication
  Serial.println("Ready to transmit data.");
}

void loop() {
  if(HC12.available()){                     // If Arduino's HC12 rx buffer has data
    Serial.write(HC12.read());              // Send the data to the computer
    }
  if(Serial.available()){                   // If Arduino's computer rx buffer has data
    HC12.write(Serial.read());              // Send that data to serial
  }
}
