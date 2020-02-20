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
byte received_package[MAX_PACKAGE_LENGTH] = {0};
uint8_t received_package_index = 0;
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
    if(received_package_index == 0) {
      received_package[MAX_PACKAGE_LENGTH] = {0};
      started_receiving_package = millis();
    }
    
    received_package[received_package_index] = transmitter0_serial.read();
    received_package_index++;
  }
  // handeling package information
  if(received_package_index >= MAX_PACKAGE_LENGTH) {
    // received full package
    if(testPackage(received_package, PERSONAL_ADDRESS)) {
      uint16_t source_address = received_package[6] << 5 | received_package[4];
      switch(received_package[2]) {
        case RR_CODE_LOGIN:
          // only accept if not logged in before or from same base
          if(BASE_ADDRESS == NULL || BASE_ADDRESS == source_address) {
            // set vars
            BASE_ADDRESS = source_address;
            ALERT_CHANNEL = received_package[8];
            // respond with RR_CODE_PYLON_STATUS
            byte package[MAX_PACKAGE_LENGTH] = {0};
            createPackageHeader(
              package,
              RR_CODE_ACK,
              PERSONAL_ADDRESS,
              BASE_ADDRESS
            );
            sendPackage(&transmitter0_serial, package);
            
            Serial.print("Accepted RR_CODE_LOGIN from ");
            Serial.print(source_address);
            Serial.print("; Alert channel: ");
            Serial.println(ALERT_CHANNEL);
          }
          // response with denied code
          else {
            // respond with RR_CODE_DENIED
            byte package[MAX_PACKAGE_LENGTH] = {0};
            createPackageHeader(
              package,
              RR_CODE_DENIED,
              PERSONAL_ADDRESS,
              BASE_ADDRESS
            );
            sendPackage(&transmitter0_serial, package);
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
          Serial.println(received_package[2]);
          break;
      }
    }

    // free old package
    received_package_index = 0;
    for(int i = 0; i < MAX_PACKAGE_LENGTH; i++) {
      received_package[i] = 0;
    }
  }
  else if(started_receiving_package + TIMEOUT < millis()) {

    // free old package
    received_package_index = 0;
    for(int i = 0; i < MAX_PACKAGE_LENGTH; i++) {
      received_package[i] = 0;
    }
  }
  
  if (Serial.available()) {
    transmitter0_serial.write(Serial.read());
  }
}
