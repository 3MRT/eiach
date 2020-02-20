// PROTOCOL INFO
const uint8_t VERSION = 1;

// DEFAULT SETTINGS
const uint8_t COM_CHANNEL = 1; // default communication channel

// RESPONSE/REQUEST CODES
const uint8_t RR_CODE_ACK           = 0;
const uint8_t RR_CODE_LOGIN         = 1;
const uint8_t RR_CODE_KEEP_ALIVE    = 2;
const uint8_t RR_CODE_PYLON_STATUS  = 3;
const uint8_t RR_CODE_SHUTDOWN_ACK  = 4;
const uint8_t RR_CODE_ALERT         = 5;

// PROTOCOL DATA LENGTH in bits; index -> RR_CODE_X
const uint8_t HEADER_LENGTH = 64; // in bytes
const uint8_t MAX_PACKAGE_LENGTH = 16; // in bytes
const uint8_t DATA_MIN_LENGTH[] = {0, 16, 0, 8, 0, 0};
const uint8_t DATA_MAX_LENGTH[] = {0, 16, 0, 64, 0, 0};
