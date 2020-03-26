// INCLUDE LIBRARIES AND FILES
#include <SoftwareSerial.h>
#include <protocol_settings.h>
#include <protocol_functions.h>

// INITIALIZE VARS
// setup transceiver modules
SoftwareSerial transmitter0_serial(11, 10);
int transmitter0_set_pin = 12;

// PROTOCOL INIT
uint16_t PERSONAL_ADDRESS = 0; // must be unique
uint16_t BASE_ADDRESS = NULL;
uint8_t ALERT_CHANNEL = 0;

// receiving a package
byte received_package[MAX_PACKAGE_LENGTH] = {0};
uint8_t received_package_index = 0;
long started_receiving_package = 0;

// pylon variables
bool alarm = false;
bool low_battery = false;
bool functional = true;
bool request_shutdown = false;

// OTHER PINS
int rgb_led_r = 7;
int rgb_led_g = 6;
int rgb_led_b = 5;


void setup() {
  pinMode(rgb_led_r, OUTPUT);
  pinMode(rgb_led_g, OUTPUT);
  pinMode(rgb_led_b, OUTPUT);
  // configure set pins of transmitters
  pinMode(transmitter0_set_pin, OUTPUT);
  digitalWrite(transmitter0_set_pin, HIGH);

  // begin serial communication
  Serial.begin(9600);
  Serial.println("////////// EIACH; PYLON //////////");
  transmitter0_serial.begin(9600);

  // get personal address
  PERSONAL_ADDRESS = getPersonalAddress();

  // switch to default com channel
  switchChannel(&transmitter0_serial, &transmitter0_set_pin, COM_CHANNEL);
}

void loop() {

  // led status
  if(alarm) {
    digitalWrite(rgb_led_r, HIGH);
    digitalWrite(rgb_led_g, LOW);
    digitalWrite(rgb_led_b, LOW);
  }
  else if(ALERT_CHANNEL == 0) {
    digitalWrite(rgb_led_r, LOW);
    digitalWrite(rgb_led_g, LOW);
    digitalWrite(rgb_led_b, HIGH);
  }
  else {
    digitalWrite(rgb_led_r, LOW);
    digitalWrite(rgb_led_g, HIGH);
    digitalWrite(rgb_led_b, LOW);
  }
  
  // DEBUGGING
  if(Serial.available()) {
    Serial.read();
    alarm = true;
  }
  else {
    alarm = false;
  }
  
  if (alarm) {
    transmitter0_serial.write(255);
  }
  else {
    // receiving packages
    if (transmitter0_serial.available()) {
      if(received_package_index == 0) {
        received_package[MAX_PACKAGE_LENGTH] = {0};
        started_receiving_package = millis();
      }
      
      received_package[received_package_index] = transmitter0_serial.read();
      received_package_index++;
    }
    // handeling package information; receive until length from received_package[3] is reached
    // saves about 8ms instead of waiting for all 16 bytes
    if(received_package_index > 3 && 
        (received_package_index >= received_package[3] || received_package_index >= MAX_PACKAGE_LENGTH)) {
      // received full package
      if(testPackage(received_package, PERSONAL_ADDRESS)) {
        uint16_t source_address = received_package[4] << 8 | received_package[5];
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
              // printPackage(package);
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
            // response with RR_CODE_PYLON_STATUS
            if(BASE_ADDRESS == source_address) {
              byte package[MAX_PACKAGE_LENGTH] = {0};
              createPackageHeader(
                package,
                RR_CODE_PYLON_STATUS,
                PERSONAL_ADDRESS,
                BASE_ADDRESS
              );
  
              // set data
              if(alarm) { package[8] |= 1UL << 7; }
              if(low_battery) { package[8] |= 1UL << 6; }
              if(functional) { package[8] |= 1UL << 5; }
              if(request_shutdown) { package[8] |= 1UL << 4; }
              
              Serial.println(" -> Responded with RR_CODE_PYLON_STATUS");
              sendPackage(&transmitter0_serial, package);
            }
            else {
              Serial.println(" -> ERROR: Not from own base");
            }
            break;
          case RR_CODE_SHUTDOWN_ACK:
            Serial.println("Received package: RR_CODE_SHUTDOWN_ACK");
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
  }
}
