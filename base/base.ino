// INCLUDE LIBRARIES AND FILES
#include <SoftwareSerial.h>
#include <protocol_settings.h>
#include <protocol_functions.h>

// INITIALIZE VARS
// setup transceiver modules
SoftwareSerial transmitter0_serial(11, 10);
int transmitter0_set_pin = 12;
SoftwareSerial transmitter1_serial(51, 50);
int transmitter1_set_pin = 52;

// PROTOCOL INIT
uint16_t PERSONAL_ADDRESS = 0; // must be unique
uint8_t ALERT_CHANNEL = 0;

uint16_t saved_pylon_addresses[] = {1, 2}; // addresses of pylons that will be parsed
bool logged_in_pylons[(sizeof(saved_pylon_addresses)/sizeof(saved_pylon_addresses[0]))] 
  = {false}; // saved state of all pylons addresss = saved_pylon_addresses[index]
uint8_t connected_devices = 0;
  
/// parsing variables (cr => current request)
int16_t cr_address_index = -1; // save where the parsing currently is
uint8_t cr_rr_code = 0; // request code of current request
long cr_timeout = 0; // timeout of current request
long cr_start_time = 0; // for performance tests
// receiving a package
byte received_package[MAX_PACKAGE_LENGTH] = {0};
uint8_t received_package_index = 0;
int failed_ping_requests = 0;
// when a pylon sets of an alarm, it jams the frequency with
// bytes of 11111111 equal to int 255.
int following_255 = 0;

// true: searching for unconnected devices
// false: always testing connections to pylons
bool pairing = true; 
enum ALARM_TYPES {off, silent, note, critical};
int alarm = 0;

// OTHER PINS
int alarm_pin = 9;
int rgb_led_r = 7;
int rgb_led_g = 6;
int rgb_led_b = 5;


void setup() {
  pinMode(rgb_led_r, OUTPUT);
  pinMode(rgb_led_g, OUTPUT);
  pinMode(rgb_led_b, OUTPUT);
  pinMode(alarm_pin, OUTPUT);
  tone(alarm_pin, 440);
  delay(100);
  noTone(alarm_pin);
  // configure set pins of transmitters
  pinMode(transmitter0_set_pin, OUTPUT);
  digitalWrite(transmitter0_set_pin, HIGH);
  pinMode(transmitter1_set_pin, OUTPUT);
  digitalWrite(transmitter1_set_pin, HIGH);

  // begin serial communication
  Serial.begin(9600);
  transmitter0_serial.begin(9600);
  // transmitter1_serial.begin(9600);
  Serial.println("////////// EIACH; BASE //////////");

  // get personal address
  PERSONAL_ADDRESS = getPersonalAddress();

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
}

void loop() {
  long current_millis = millis();
  // only listen when a package was sent
  if(cr_rr_code > 0 && cr_timeout > 0) {
    while (transmitter0_serial.available() 
        && cr_timeout > current_millis 
        && received_package_index < MAX_PACKAGE_LENGTH) {
      // handle received data
      if(received_package_index == 0) {
        received_package[MAX_PACKAGE_LENGTH] = {0};
      }
      
      received_package[received_package_index] = transmitter0_serial.read();
      if(int(received_package[received_package_index]) == 255) {
        following_255++;
      }
      else {
        following_255 = 0;
      }
      received_package_index++;
    }
    if(received_package_index < MAX_PACKAGE_LENGTH && cr_timeout < current_millis) {
      // handle timeout
      Serial.print("Package timed out; Time: ");
      Serial.print(millis());
      Serial.print("ms");

      if(cr_rr_code == RR_CODE_KEEP_ALIVE) {
        // alert
        failed_ping_requests++;
        Serial.print("; Failed ping requests: ");
        Serial.print(failed_ping_requests);
        if(failed_ping_requests < PING_REDUNDANCY) {
          Serial.print("; Trying again!");
          cr_address_index--;
        }
        else {
          Serial.print("; PING_REDUNDANCY exceeded! => alarm = critical");
          alarm = critical;
        }
      }
      Serial.println();
      
      cr_rr_code = 0;
      cr_timeout = 0;
      cr_start_time = 0;
  
      // free old package
      received_package_index = 0;
      for(int i = 0; i < MAX_PACKAGE_LENGTH; i++) {
        received_package[i] = 0;
      }
    }
    else {
      // waiting
    }
    // handeling package information; receive until length from received_package[3] is reached
    // saves about 8ms instead of waiting for all 16 bytes
    if(received_package_index > 3 && 
        (received_package_index >= received_package[3] || received_package_index >= MAX_PACKAGE_LENGTH)) {
      // received full package
      if(testPackage(received_package, PERSONAL_ADDRESS)) {
        // printPackage(received_package);
        uint16_t source_address = received_package[6] << 5 | received_package[4];
        // requested code for package
        switch(cr_rr_code) {
          
          // handle responses of login requests
          case RR_CODE_LOGIN:
            switch(received_package[2]) {
              // pylon accepted login request
              case RR_CODE_ACK:
                Serial.print("Pylon accepted request! Time used: ");
                Serial.print(millis() - cr_start_time);
                Serial.println("ms");
                // mark as logged in
                logged_in_pylons[cr_address_index] = true;
                connected_devices++;
                break;
              // pylon denied login request
              // probably because it was connected to another base
              case RR_CODE_DENIED:
                Serial.println("Pylon denied request!");
                logged_in_pylons[cr_address_index] = false;
                break;
              default:
                Serial.print("Received package with unknown RR_CODE: ");
                Serial.println(received_package[2]);
                break;
            }
            break;

          case RR_CODE_KEEP_ALIVE:
            if(received_package[2] == RR_CODE_PYLON_STATUS) {
              Serial.print("Received response RR_CODE_PYLON_STATUS; Time used: ");
              Serial.print(millis() - cr_start_time);
              Serial.println("ms");
              // parse pylon status
              if((received_package[8] >> 7) & 1) { // alarm
                Serial.println("Pylon has started alarm; alarm = critical");
                alarm = critical;
              }
              if(!(received_package[8] >> 5) & 1) { // not functional
                Serial.println("Pylon's gyroscope is not functional; alarm = critical");
                alarm = critical;
              }
              if((received_package[8] >> 6) & 1) { // low_battery
                Serial.println("Pylon's battery is low; alarm = note");
                alarm = note;
              }
              if((received_package[8] >> 4) & 1) { // request shutdown
                Serial.println("Pylon requested shutdown; sending RR_CODE_SHUTDOWN_ACK");
                // log out 
                logged_in_pylons[cr_address_index] = false;
                connected_devices--;
                // response with RR_CODE_SHUTDOWN_ACK
                byte package[MAX_PACKAGE_LENGTH] = {0};
                createPackageHeader(
                  package,
                  RR_CODE_SHUTDOWN_ACK,
                  PERSONAL_ADDRESS,
                  saved_pylon_addresses[cr_address_index]
                );
                sendPackage(&transmitter0_serial, package);
              }
              // reset ping requests
              failed_ping_requests = 0;
            }
            else {
              Serial.print("Invalid response code: ");
              Serial.println(received_package[2]);
            }
            break;
          
          default:
            Serial.print("Sent package with unknown RR_CODE: ");
            Serial.println(cr_rr_code);
            break;
        }
      }

      cr_rr_code = 0;
      cr_timeout = 0;
      cr_start_time = 0;
      // free old package
      received_package_index = 0;
      for(int i = 0; i < MAX_PACKAGE_LENGTH; i++) {
        received_package[i] = 0;
      }
    }
  }
  else if(transmitter0_serial.available()) {
    if(int(transmitter0_serial.read()) == 255) {
      following_255++;
    }
    else {
      following_255 = 0;
    }
  }
  // when not waiting for a package continue parsing
  else {
    // proceed to next cr_address_index
    cr_address_index++;
    // begin at 0 if end is reached and end pairing
    if(cr_address_index >= sizeof(logged_in_pylons)) {
      cr_address_index = 0;
      pairing = false;
    }
    
    // checking for login of every pylon
    if(pairing && logged_in_pylons[cr_address_index] == false) {
      Serial.print("Sending pairing request to pylon-id ");
      Serial.println(saved_pylon_addresses[cr_address_index]);
      // pairing
      byte package[MAX_PACKAGE_LENGTH] = {0};
      createPackageHeader(
        package,
        RR_CODE_LOGIN,
        PERSONAL_ADDRESS,
        saved_pylon_addresses[cr_address_index]
      );
      // alert channel
      package[8] = ALERT_CHANNEL;
      delay(25); // TODO: investigate why delay is required
      // printPackage(package);

      sendPackage(&transmitter0_serial, package);

      // set cr_rr_code and cr_timeout to wait for response
      cr_rr_code = package[2];
      cr_start_time = millis();
      cr_timeout = cr_start_time + TIMEOUT;
    }
    // test availability of every pylon
    else if(!pairing && logged_in_pylons[cr_address_index] == true) {
      Serial.println();
      Serial.print("Sending RR_CODE_KEEP_ALIVE to pylon-id ");
      Serial.println(saved_pylon_addresses[cr_address_index]);
      // pining
      byte package[MAX_PACKAGE_LENGTH] = {0};
      createPackageHeader(
        package,
        RR_CODE_KEEP_ALIVE,
        PERSONAL_ADDRESS,
        saved_pylon_addresses[cr_address_index]
      );
      // printPackage(package);

      sendPackage(&transmitter0_serial, package);

      // set cr_rr_code and cr_timeout to wait for response
      cr_rr_code = package[2];
      cr_start_time = millis();
      cr_timeout = cr_start_time + TIMEOUT;
    }
  }

  // when following_255 is to large, an alarm is detected
  if(following_255 > 2) {
    Serial.println("Detected alarm from pylon!");
    alarm = critical;
  }
  
  // led status
  if(alarm == critical) {
    digitalWrite(rgb_led_r, HIGH);
    digitalWrite(rgb_led_g, LOW);
    digitalWrite(rgb_led_b, LOW);
  }
  else if(pairing || connected_devices <= 0) {
    digitalWrite(rgb_led_r, LOW);
    digitalWrite(rgb_led_g, LOW);
    digitalWrite(rgb_led_b, HIGH);
  }
  else {
    digitalWrite(rgb_led_r, LOW);
    digitalWrite(rgb_led_g, HIGH);
    digitalWrite(rgb_led_b, LOW);
  }
  
  // alarm management
  if(alarm == critical) {
    tone(alarm_pin, 440);
  }
  else {
    noTone(alarm_pin);
  }
}
