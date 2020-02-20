////////// SENDING FUNCTIONS //////////

void createPackageHeader(byte package[], uint8_t rr_code, uint16_t personal_address, uint16_t destination_address);
void sendPackage(SoftwareSerial *transmitter, byte package[]);

// create header byte array for package
void createPackageHeader(byte package[], uint8_t rr_code, uint16_t personal_address, uint16_t destination_address) {
  package[0] = VERSION;
  package[1] = 0; // TODO: create checksum
  package[2] = RR_CODE_LOGIN;
  package[3] = MAX_PACKAGE_LENGTH; // TODO: get length
  // source address
  package[4] = (personal_address >> 8) & 0xff;
  package[5] = personal_address & 0xff;
  // destination address
  package[6] = (destination_address >> 8) & 0xff;
  package[7] = destination_address & 0xff;
}
// send a package over software serial
void sendPackage(SoftwareSerial *transmitter, byte package[]) {
  for(int i = 0; i < MAX_PACKAGE_LENGTH; i++) {
    (*transmitter).write(package[i]);
  }
}

////////// TEST FUNCTIONS //////////

bool testPackage(uint16_t personal_address);

bool testPackage(byte package[], uint16_t personal_address) {
  // test same version
  if(package[0] != VERSION) {
    return false;
  }
  // TODO: test checksum
  // TODO: check recipient
  uint16_t recipient_address = package[6] << 8 | package[7];
  if(recipient_address != personal_address) {
    return false;
  }
  // TODO: check data length validity
  return true;
}

////////// DEBUGGING FUNCTIONS //////////

void printPackage(byte package[]);

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

////////// AT FUNCTIONS //////////

void switchChannel(SoftwareSerial *transmitter, int *set_pin, uint8_t channel);
void executeAtCommand(SoftwareSerial *transmitter, int *set_pin, String command);

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

