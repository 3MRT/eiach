// WARNING: Not all pins on the Mega and Mega 2560
// support change interrupts, so only the following
// can be used for RX: 10, 11, 12, 13, 14, 15,
// 50, 51, 52, 53, A8 (62), A9 (63), A10 (64),
// A11 (65), A12 (66), A13 (67), A14 (68), A15 (69).
#include <SoftwareSerial.h>

// setup transceiver modules
SoftwareSerial transmitter0_serial(11, 10);
int transmitter0_set_pin = 12;

void setup() {
  // configure set pins of transmitters
  pinMode(transmitter0_set_pin, OUTPUT);
  digitalWrite(transmitter0_set_pin, HIGH);

  // begin serial communication
  Serial.begin(9600);
  transmitter0_serial.begin(9600);

  // TEST: switch channel
  atCommand(&transmitter0_serial, &transmitter0_set_pin, "AT+C020");
}

void loop() {
  if (transmitter0_serial.available()) {
    Serial.write(transmitter0_serial.read());
  }
  if (Serial.available()) {
    transmitter0_serial.write(Serial.read());
  }
}

// execute a at command on a transmitter
void atCommand(SoftwareSerial *transmitter, int *set_pin, String command) {
  digitalWrite(*set_pin, LOW);
  delay(50); // REQUIRED: possibly because of physical delay
  Serial.print("Setting new channel: ");
  (*transmitter).println(command);
  delay(50); // REQUIRED: possibly because of physical delay
  digitalWrite(*set_pin, HIGH);
}

