#include <SoftwareSerial.h>

//--- Begin Pin Declarations ---//
const byte HC12RxdPin = 4;  // "RXD" Pin on HC12
const byte HC12TxdPin = 5;  // "TXD" Pin on HC12
const byte HC12SetPin = 6;  // "SET" Pin on HC12
const byte GPSRxdPin = 7;   // "RXD" on GPS (if available)
const byte GPSTxdPin = 8;   // "TXD" on GPS
//--- End Pin Declarations ---//

//--- Begin variable declarations ---//
char byteIn;                   // Temporary variable
String HC12ReadBuffer = "";    // Read/Write Buffer 1 -- Serial
String SerialReadBuffer = "";  // Read/Write Buffer 2 -- HC12
String GPSReadBuffer = "";     // Read/Write Buffer 3 -- GPS

boolean serialEnd = false;    // Flag for End of Serial String
boolean HC12End = false;      // Flag for End of HC12 String
boolean GPSEnd = false;       // Flag for End of GPS String
boolean commandMode = false;  // Send AT commands to remote receivers
boolean GPSLocal = true;      // send GPS local or remote flag
//--- End variable declarations ---//

// Create Software Serial Ports for HC12 & GPS
// Software Serial ports Rx and Tx are opposite the HC12 Rxd and Txd
SoftwareSerial HC12(HC12TxdPin, HC12RxdPin);
SoftwareSerial GPS(GPSTxdPin, GPSRxdPin);

void setup() {
  HC12ReadBuffer.reserve(82);    // Reserve 82 bytes for message
  SerialReadBuffer.reserve(82);  // Reserve 82 bytes for message
  GPSReadBuffer.reserve(82);     // Reserve 82 bytes for longest NMEA sentence

  pinMode(HC12SetPin, OUTPUT);     // Output High for Transparent / Low for Command
  digitalWrite(HC12SetPin, HIGH);  // Start in Transparent mode
  delay(80);                       // 80 ms delay before operation per datasheet

  Serial.begin(9600);  // Open serial port to computer at 9600 Baud
  HC12.begin(9600);    // Open software serial port to HC12 at 9600 Baud
  GPS.begin(9600);     // Open software serial port to GPS at 9600 Baud

  // --- Begin One-Time HC12 Auto-Configuration ---
  digitalWrite(HC12SetPin, LOW);  // Enter command mode
  delay(100);                     // Wait for HC12 to settle
  HC12.println("AT+DEFAULT");     // Example AT command (can change to AT+DEFAULT, etc.)
  delay(200);                     // Wait for reply

  while (HC12.available()) {
    Serial.write(HC12.read());  // Print HC12's response to serial monitor
  }

  digitalWrite(HC12SetPin, HIGH);  // Return to transparent mode
  delay(100);                      // Wait for HC12 to stabilize
  // --- End Auto-Configuration ---

  HC12.listen();  // Start listening to HC12 after setup
}


void loop() {
  while (HC12.available()) {         // If Arduino's HC12 rx buffer has data
    byteIn = HC12.read();            // Store each character in byteIn
    HC12ReadBuffer += char(byteIn);  // Write each character of byteIn to HC12ReadBuffer
    if (byteIn == '\n') {            // At the end of the line
      HC12End = true;                // Set HC12End flag to true.
    }
  }

  while (Serial.available()) {         // If Arduino's computer rx buffer has data
    byteIn = Serial.read();            // Store each character in byteIn
    SerialReadBuffer += char(byteIn);  // Write each character of byteIn to SerialReadBuffer
    if (byteIn == '\n') {              // At the end of the line
      serialEnd = true;                // Set serialEnd flag to true.
    }
  }

  while (GPS.available()) {
    byteIn = GPS.read();
    GPSReadBuffer += char(byteIn);
    Serial.print(char(byteIn));  // <--- Add this to see GPS output in Serial Monitor

    if (byteIn == '\n') {
      GPSEnd = true;
    }
  }

  if (serialEnd) {                                // Check to see if serialEnd flag is true
    if (SerialReadBuffer.startsWith("AT")) {      // Check to see if a command has been sent
      if (SerialReadBuffer.startsWith("AT+B")) {  // If it is a baud change command, delete it immediately
        SerialReadBuffer = "";
        Serial.print("Denied: Changing HC12 Baud does not change Arduino Baudrate");
      }
      HC12.print(SerialReadBuffer);    // Send local command to remote HC12 before changing settings
      delay(100);                      //
      digitalWrite(HC12SetPin, LOW);   // If true, enter command mode
      delay(100);                      // Delay before writing command
      HC12.print(SerialReadBuffer);    // Send command to HC12
      Serial.print(SerialReadBuffer);  // Send command to serial
      delay(500);                      // Wait 0.5s for reply
      digitalWrite(HC12SetPin, HIGH);  // Exit command / enter transparent mode
      delay(100);                      // Delay before proceeding
    }
    if (SerialReadBuffer.startsWith("GPS")) {
      HC12.print(SerialReadBuffer);
      GPS.listen();
      GPSLocal = true;
    }
    HC12.print(SerialReadBuffer);  // Send text to HC12 to be broadcast
    SerialReadBuffer = "";         // Clear buffer 2
    serialEnd = false;             // Reset serialEnd flag
  }

  if (HC12End) {                            // If HC12End flag is true
    if (HC12ReadBuffer.startsWith("AT")) {  // Check to see if a command was received
      digitalWrite(HC12SetPin, LOW);        // If true, enter command mode
      delay(40);                            // Delay before writing command
      HC12.print(HC12ReadBuffer);           // Send incoming command back to HC12
      Serial.println(HC12ReadBuffer);       // Send command to serial
      delay(1000);                          // Wait 0.5s for reply
      digitalWrite(HC12SetPin, HIGH);       // Exit command / enter transparent mode
      delay(80);                            // Delay before proceeding
      HC12.println("Remote Command Executed");
    }
    if (HC12ReadBuffer.startsWith("GPS")) {
      GPS.listen();
      HC12.print("Remote GPS Command Received");
      Serial.println("Switched to GPS listen mode...");
      GPSLocal = false;
      delay(2000);  // Give GPS time to send a sentence
    }
    Serial.print(HC12ReadBuffer);  // Send message to screen
    HC12ReadBuffer = "";           // Empty Buffer
    HC12End = false;               // Reset Flag
  }

  if (GPSEnd) {
    // Options include GPRMC, GPGGA, GPGLL, etc...
    if (GPSReadBuffer.startsWith("$GPRMC")) {  // Look for target GPS sentence
      if (GPSLocal) {
        Serial.print("Local  GPS:");  // Send to local serial port
        Serial.print(GPSReadBuffer);  // Send local GPS
      } else {
        HC12.print("Remote GPS:");  // Local Arduino responds to remote request
        HC12.print(GPSReadBuffer);  // Sends local GPS to remote
      }
      GPSReadBuffer = "";  // Delete target GPS sentence
      HC12.listen();       // Found target GPS sentence, start listening to HC12 again
    } else {
      GPSReadBuffer = "";  // Delete unwanted strings
    }
    GPSEnd = false;  // Reset GPS
  }
}
