#include <EEPROM.h>

void setup() {
  pinMode(8, INPUT);
  Serial.begin(9600);
}

void loop() {
  if(Serial.available()) {
    EEPROM.write(0, 1);
    delay(100);
    Serial.println(EEPROM.read(0));
  }
}
