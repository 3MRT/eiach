#include <protocol_functions.h>
#include <protocol_settings.h>

////////// DEBUGGING FUNCTIONS //////////
void printPackage(byte package[]) {
  Serial.println();
  Serial.println("-------- Package --------");
  for(int i = 0; i < 16; i++) {
    switch(i) {
      case 0:
        Serial.print("Version:  ");
        break;
      case 1:
        Serial.print("Checksum: ");
        break;
      case 2:
        Serial.print("RR Code:  ");
        break;
      case 3:
        Serial.print("Length:   ");
        break;
      case 4:
        Serial.print("Source:   ");
        break;
      case 6:
        Serial.print("Dest.:    ");
        break;
      case 8:
        Serial.print("Data:     ");
        break;
      default:
        Serial.print("          ");
        break;
    }
    for(byte mask = 0x80; mask; mask >>= 1){
      if(mask  & package[i])
         Serial.print('1');
      else
         Serial.print('0');
    }
    Serial.print(" | ");
    Serial.print(package[i]);
    Serial.println();
  }
  Serial.println();
}

////////// SENDING FUNCTIONS //////////

// create header byte array for package
void createPackageHeader(byte package[], uint8_t rr_code, uint16_t personal_address, uint16_t destination_address) {
  package[0] = VERSION;
  package[2] = rr_code;
  package[3] = MAX_PACKAGE_LENGTH; // gets overwritten in sendPackage
  // source address
  package[4] = (personal_address >> 8) & 0xff;
  package[5] = personal_address & 0xff;
  // destination address
  package[6] = (destination_address >> 8) & 0xff;
  package[7] = destination_address & 0xff;
}
// send a package over software serial
void sendPackage(SoftwareSerial *transmitter, byte package[]) {
  // get package length
  uint8_t package_length = 0;
  for(int i = MAX_PACKAGE_LENGTH - 1; i >= 0 && package_length == 0; i--) {
    if(package_length == 0 && int(package[i]) != 0) {
      package_length = i + 1;
    }
  }
  // set package length
  package[3] = package_length;
  
  // calculate checksum
  package[1] = getChecksum(package);

  for(int i = 0; i < package_length; i++) {
    (*transmitter).write(package[i]);
  }
}
// calculate checksum of a package
uint8_t getChecksum(byte package[]) {
  // set original checksum to 0
  package[1] = 0;

  // checksum is the sum of the decimal values
  uint8_t checksum = 0;
  for(int i = 0; i < package[3] && i < MAX_PACKAGE_LENGTH; i++) {
    checksum += int(package[i]);
  }
  return checksum;
}

////////// TEST FUNCTIONS //////////

bool testPackage(byte package[], uint16_t personal_address) {
  // test same version
  if(package[0] != VERSION) {
    Serial.println("Received package with invalid version");
    return false;
  }
  // test checksum
  uint8_t checksum = package[1];
  if(checksum != getChecksum(package)) {
    Serial.println("Received package with invalid checksum");
    return false;
  }
  // check recipient
  uint16_t recipient_address = package[6] << 8 | package[7];
  if(recipient_address != personal_address) {
    return false;
  }
  // check valid rr_code
  if(package[2] < 1 && package[2] > 7) {
    Serial.println("Received package with invalid rr_code");
    return false;
  }
  // check data length validity
  if(package[3] < DATA_MIN_LENGTH[package[2]] + 8 
    && package[3] > DATA_MAX_LENGTH[package[2]] + 8) {
    Serial.println("Received package with invalid length");
    return false;
  }
  return true;
}

////////// AT FUNCTIONS //////////

// switch transmitter to new channel
// TODO: maeby merge with executeAtCommand for performance reasons
void switchChannel(SoftwareSerial *transmitter, int *set_pin, uint8_t channel) {
  char channel_string[8];
  snprintf(channel_string, 8, "AT+C%03d", channel);
  executeAtCommand(transmitter, set_pin, channel_string);
}

// execute a at command on a transmitter; average execution time: 109ms
void executeAtCommand(SoftwareSerial *transmitter, int *set_pin, String command) {
  digitalWrite(*set_pin, LOW);
  
  delay(50); // REQUIRED: possibly because of physical delay
  Serial.print("Executing AT Command: ");
  Serial.print(command);
  
  (*transmitter).println(command); // execute command
  delay(50); // REQUIRED: possibly because of physical delay

  // wait until a response is available
  while(!(*transmitter).available()) {}
  Serial.print(" -> ");
  while((*transmitter).available()) {
    Serial.write((*transmitter).read());
  }
  
  digitalWrite(*set_pin, HIGH);
}

// get addess saved in eeprom
uint16_t getPersonalAddress() {
  // get address
  int timeout = 0;
  int input = EEPROM.read(0);
  int oldInput = input;
  int reset = input > 0 && Serial;

  if(input == 0 || reset) {

    if(reset) {
      // if an address already exists, wait only 5s
      timeout = millis() + 5000;
    }

    // wait for connection
    while(!Serial) {}
    Serial.print("(Waiting) Please input the UNIQUE personal address: ");

    while(input == oldInput && (timeout == 0 || timeout > millis())) {
      int parsed = Serial.parseInt();
      if(parsed > 0) {
        input = parsed;
      }
    }
    // write address to eeprom
    EEPROM.write(0, input);
  }
  else {
    Serial.print("Personal address: ");
  }

  Serial.println(input);
  return input;
}
