// PROTOCOL INFO
const uint8_t VERSION = 1;

// DEFAULT SETTINGS
const uint8_t COM_CHANNEL = 1; // default communication channel
const int TIMEOUT = 500;

// RESPONSE/REQUEST CODES
// 0 should not be used!
const uint8_t RR_CODE_LOGIN         = 1;
const uint8_t RR_CODE_KEEP_ALIVE    = 2;
const uint8_t RR_CODE_PYLON_STATUS  = 3;
const uint8_t RR_CODE_SHUTDOWN_ACK  = 4;
const uint8_t RR_CODE_ALERT         = 5;
const uint8_t RR_CODE_ACK           = 6;
const uint8_t RR_CODE_DENIED        = 7;

// PROTOCOL DATA LENGTH; index -> RR_CODE_X
const uint8_t HEADER_LENGTH = 64; // in bytes
const uint8_t MAX_PACKAGE_LENGTH = 16; // in bytes
const uint8_t DATA_MIN_LENGTH[] = {0, 2, 0, 1, 0, 0, 0}; // in bytes
const uint8_t DATA_MAX_LENGTH[] = {0, 2, 0, 8, 0, 0, 0}; // in bytes
