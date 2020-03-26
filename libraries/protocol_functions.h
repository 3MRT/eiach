#ifndef PROTOCOL_FUNCTIONS
#define PROTOCOL_FUNCTIONS

#include <Arduino.h>
#include <EEPROM.h>
#include <SoftwareSerial.h>

extern uint16_t getPersonalAddress();
extern void printPackage(byte package[]);
extern void createPackageHeader(byte package[], uint8_t rr_code, uint16_t personal_address, uint16_t destination_address);
extern void sendPackage(SoftwareSerial *transmitter, byte package[]);
extern uint8_t getChecksum(byte package[]);
extern bool testPackage(byte package[], uint16_t personal_address);
extern void switchChannel(SoftwareSerial *transmitter, int *set_pin, uint8_t channel);
extern void executeAtCommand(SoftwareSerial *transmitter, int *set_pin, String command);

#endif