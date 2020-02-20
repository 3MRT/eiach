// INCLUDE LIBRARIES AND FILES
#include <SoftwareSerial.h>
#include "protocol_settings.h"
#include "protocol_functions.h"

// INITIALIZE VARS
// setup transceiver modules
SoftwareSerial transmitter0_serial(11, 10);
int transmitter0_set_pin = 12;
SoftwareSerial transmitter1_serial(51, 50);
int transmitter1_set_pin = 52;

// PROTOCOL INIT
const uint16_t PERSONAL_ADDRESS = 0; // must be unique
uint8_t ALERT_CHANNEL = 0;

uint16_t saved_pylon_addresses[] = {1, 2}; // addresses of pylons that will be parsed
bool logged_in_pylons[(sizeof(SAVED_PYLON_ADDRESSES)/sizeof(SAVED_PYLON_ADDRESSES[0]))] 
  = {false}; // saved state of all pylons addresss = saved_pylon_addresses[index]
  
/// parsing variables (cr => current request)
uint16_t cr_address_index = 0; // save where the parsing currently is
uint8_t cr_rr_code = NULL; // request code of current request
long cr_timeout = NULL; // timeout of current request

// true: searching for unconnected devices
// false: always testing connections to pylons
bool paring = true; 


void setup() {
  // configure set pins of transmitters
  pinMode(transmitter0_set_pin, OUTPUT);
  digitalWrite(transmitter0_set_pin, HIGH);
  pinMode(transmitter1_set_pin, OUTPUT);
  digitalWrite(transmitter1_set_pin, HIGH);

  // begin serial communication
  Serial.begin(9600);
  transmitter0_serial.begin(9600);
  // transmitter1_serial.begin(9600);

  // use physical randomness
  randomSeed(analogRead(0));
  // channel should be at least 10 units from com_channel
  // maximum channel is 126
  ALERT_CHANNEL = random(11, 127);
  Serial.print("Random alert channel: ");
  Serial.println(ALERT_CHANNEL);

  // switch to default com channel
  switchChannel(&transmitter0_serial, &transmitter0_set_pin, COM_CHANNEL);
  // TODO: uncomment when transmitter1 is connected
  // switchChannel(&transmitter1_serial, &transmitter1_set_pin, ALERT_CHANNEL);

  // DEBUGGING
  byte package[MAX_PACKAGE_LENGTH] = {0};
  package[0] = VERSION;
  package[1] = 0; // TODO: create checksum
  package[2] = RR_CODE_PYLON_STATUS;
  package[3] = 0; // TODO: get length
  // source address
  package[4] = (PERSONAL_ADDRESS >> 8) & 0xff;
  package[5] = PERSONAL_ADDRESS & 0xff;

  printPackage(package);
  Serial.println();
}

void loop() {
  if (transmitter0_serial.available()) {
    Serial.write(transmitter0_serial.read());
  }
  if (Serial.available()) {
    transmitter0_serial.write(Serial.read());
  }
}
