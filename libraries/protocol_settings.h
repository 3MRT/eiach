/*
 * WARNING: no value of the protocol must be smaller than:
 *  uint8_t: 255
 *  uint16_t: 65535
 * to ensure that the base can correctly identifiy
 * an alarm.
 */

// PROTOCOL INFO
const uint8_t VERSION = 1;

// DEFAULT SETTINGS
const uint8_t COM_CHANNEL = 1; // default communication channel
const int TIMEOUT = 120;
const int PING_REDUNDANCY = 4; // if a ping failes, try again 4x before starting alert

// RESPONSE/REQUEST CODES
// 0 should not be used!
const uint8_t RR_CODE_LOGIN         = 1;
const uint8_t RR_CODE_KEEP_ALIVE    = 2;
const uint8_t RR_CODE_PYLON_STATUS  = 3;
const uint8_t RR_CODE_SHUTDOWN_ACK  = 4;
const uint8_t RR_CODE_ALERT         = 5;
const uint8_t RR_CODE_ACK           = 6;
const uint8_t RR_CODE_DENIED        = 7;
// when adding a code, update testPackage! and DATA_MIN/MAX_LENGTH

// PROTOCOL DATA LENGTH; index -> RR_CODE_X
const uint8_t HEADER_LENGTH = 64; // in bytes
const uint8_t MAX_PACKAGE_LENGTH = 16; // in bytes
const uint8_t DATA_MIN_LENGTH[] = {0, 0, 2, 0, 1, 0, 0, 0}; // in bytes
const uint8_t DATA_MAX_LENGTH[] = {0, 0, 2, 0, 8, 0, 0, 0}; // in bytes
