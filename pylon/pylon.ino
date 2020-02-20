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

// receiving a package
byte package[MAX_PACKAGE_LENGTH] = {0};
uint8_t package_index = 0;
long started_receiving_package = 0;

void setup() {
  // configure set pins of transmitters
  pinMode(transmitter0_set_pin, OUTPUT);
  digitalWrite(transmitter0_set_pin, HIGH);

  // begin serial communication
  Serial.begin(9600);
  Serial.println("////////// EIACH; PYLON //////////");
  transmitter0_serial.begin(9600);

  // switch to default com channel
  switchChannel(&transmitter0_serial, &transmitter0_set_pin, COM_CHANNEL);
}

void loop() {
  // receiving packages
  if (transmitter0_serial.available()) {
    if(package_index == 0) {
      package[MAX_PACKAGE_LENGTH] = {0};
      started_receiving_package = millis();
    }
    
    package[package_index] = transmitter0_serial.read();
    package_index++;
  }
  // handeling package information
  if(package_index >= MAX_PACKAGE_LENGTH) {
    // received full package
    if(testPackage(package, PERSONAL_ADDRESS)) {
      uint16_t source_address = package[6] << 5 | package[4];
      switch(package[2]) {
        case RR_CODE_LOGIN:
          // only accept if not logged in before or from same base
          if(BASE_ADDRESS == NULL || BASE_ADDRESS == source_address) {
            // set vars
            BASE_ADDRESS = source_address;
            ALERT_CHANNEL = package[8];
            // TODO: respond with RR_CODE_PYLON_STATUS
            Serial.print("Accepted RR_CODE_LOGIN from ");
            Serial.print(source_address);
            Serial.print("; Alert channel: ");
            Serial.println(ALERT_CHANNEL);
          }
          // response with denied code
          else {
            // TODO: respond with RR_CODE_DENIED
            Serial.print("Denied RR_CODE_LOGIN from ");
            Serial.print(source_address);
          }
          break;
        case RR_CODE_KEEP_ALIVE:
          Serial.print("Received package: RR_CODE_KEEP_ALIVE");
          // TODO
          break;
        case RR_CODE_SHUTDOWN_ACK:
          Serial.print("Received package: RR_CODE_SHUTDOWN_ACK");
          // TODO
          break;
        default:
          Serial.print("Received package with unknown RR_CODE: ");
          Serial.println(package[2]);
          break;
      }
    }

    // free old package
    package_index = 0;
    for(int i = 0; i < MAX_PACKAGE_LENGTH; i++) {
      package[i] = 0;
    }
  }
  else if(started_receiving_package + TIMEOUT < millis()) {

    // free old package
    package_index = 0;
    for(int i = 0; i < MAX_PACKAGE_LENGTH; i++) {
      package[i] = 0;
    }
  }
  
  if (Serial.available()) {
    transmitter0_serial.write(Serial.read());
  }
}
