// INCLUDE LIBRARIES AND FILES
#include <SoftwareSerial.h>
#include "protocol_settings.h"
#include "protocol_functions.h"

// INITIALIZE VARS
// setup transceiver modules
SoftwareSerial transmitter0_serial(11, 10);
int transmitter0_set_pin = 12;

// PROTOCOL INIT
const uint16_t PERSONAL_ADDRESS = 1; // must be unique
uint16_t BASE_ADDRESS = NULL;
uint8_t ALERT_CHANNEL = 0;


void setup() {
  // configure set pins of transmitters
  pinMode(transmitter0_set_pin, OUTPUT);
  digitalWrite(transmitter0_set_pin, HIGH);

  // begin serial communication
  Serial.begin(9600);
  transmitter0_serial.begin(9600);

  // switch to default com channel
  switchChannel(&transmitter0_serial, &transmitter0_set_pin, COM_CHANNEL);
}

void loop() {
  if (transmitter0_serial.available()) {
    Serial.write(transmitter0_serial.read());
  }
  if (Serial.available()) {
    transmitter0_serial.write(Serial.read());
  }
}
