#ifndef CANBESERIAL_H_
#define CANBESERIAL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "stdbool.h"
#include "cobs_u8.h"

#define CBS_DEVICE_INFORMATION " christian-marty.ch/electricthings/CANbeSerial\x00\x00" // Leading space and trailing 0s will be overwritten

#define CBS_PROTOCOL_VERSION 1

#define TX_BUFFER_SIZE 500
#define RX_BUFFER_SIZE 500
#define MAX_FRAME_SIZE 80

typedef enum {
    cbs_baudRate_10k,
    cbs_baudRate_20k,
    cbs_baudRate_50k,
    cbs_baudRate_100k,
    cbs_baudRate_125k,
    cbs_baudRate_250k,
    cbs_baudRate_500k,
    cbs_baudRate_1M,
    cbs_baudRate_2M,
    cbs_baudRate_5M,
    cbs_baudRate_10M,
    cbs_baudRate_error = 0xFF
} cbs_baudRate_t;

typedef enum {
    cbs_error_noError,
    cbs_error_crc,
    cbs_error_notEnabled,
    cbs_error_txBufferFull,
    cbs_error_txFrameMalFormatted,
    cbs_error_txFrameDlcLengthMismatch,
    cbs_error_unknownPayloadId,
    cbs_error_unsupportedPayloadId,
    cbs_error_data_dlc,
    cbs_error_can_rx,
    cbs_error_can_tx,

    cbs_error_configurationStateCommand_size
} cbs_error_t;

typedef enum {
    cbs_data = 0x00,
    cbs_error = 0x01,
    cbs_transmitAcknowledgment = 0x02,

    cbs_protocolVersion = 0x08,
    cbs_protocolVersionRequest = 0x88,

    cbs_configurationState = 0x09,
    cbs_configurationStateRequest = 0x89,
    cbs_configurationStateCommand = 0xC9,

    cbs_deviceInformation = 0x0A,
    cbs_deviceInformationRequest = 0x8A,

    cbs_deviceStatus = 0x0B,
    cbs_deviceStatusRequest = 0x8B
} cbs_payloadId_t;


// *** Configuration Structure ***
// This structure can be used defines/contains the configuration of a CAN-Bus endpoint.
typedef struct {
    union {
        struct {
            uint8_t submissive: 1; // Read-only:  indicates that this connection is submissive and therefor can not change any settings in the configuration.
            uint8_t enabled: 1; // Enables the CAN Peripheral.
            uint8_t automaticRetransmission: 1;
            uint8_t silentMode: 1;
            uint8_t reserved0: 4;

            uint8_t reserved1: 8;
        } bits;
        uint8_t  byte[2];
        uint16_t word;
    } bus;
    cbs_baudRate_t baudrate;
    cbs_baudRate_t fdBaudrate;
} cbs_configuration_t ;

typedef enum {
    cbs_lec_noError,
    cbs_lec_stuffError,
    cbs_lec_formError,
    cbs_lec_bit1Error,
    cbs_lec_bit0Error,
    cbs_lec_crcError,
    cbs_lec_noChange
}cbs_lastErrorCode_t;

typedef struct {
    uint8_t transmitErrorCounter;
    uint8_t receiveErrorCounter;
    uint8_t errorLoggingCounter;
    union {
        struct {
            uint8_t lastErrorCode:3;
            uint8_t busOff: 1;
            uint8_t reserved: 4;
        } bits;
        uint8_t byte;
    } status;
}cbs_status_t;

typedef struct {
    cbs_configuration_t configuration;

    uint16_t txIndex;
    uint8_t txData[TX_BUFFER_SIZE];

    uint16_t rxIndex;
    uint8_t rxData[RX_BUFFER_SIZE];
    cobs_decodeStream_t cobsDecoder;
} cbs_t;

// *** Data Frame ***
typedef struct {
    uint8_t extended: 1;
    uint8_t remoteTransmissionRequest: 1;
    uint8_t flexibleDataRate: 1;
    uint8_t bitRateSwitching: 1;

    uint8_t dataLengthCode: 4;
} cbs_flags0_t;

typedef struct {
    uint32_t timestamp;
    uint32_t identifier; // in case of standard ID / only 16 bit are transmitted
    cbs_flags0_t flags0;
    uint8_t flags1;
    uint8_t data[64]; // data-size longer as in DLC specified length will be omitted.
} cbs_data_t ;


void cbs_init(cbs_t *cbs);
void cbs_onSerialDataReceived(cbs_t *cbs, uint8_t *data, uint8_t dataLength);

void cbs_sendData(cbs_t *cbs, cbs_data_t *data); // send can frame on serial port
void cbs_sendError(cbs_t *cbs, cbs_error_t error);
void cbs_sendAcknowledgment(cbs_t *cbs);

void cbs_handleDataFrame(cbs_t *cbs, cbs_data_t *data); // callback on data -> must be implemented in application code
bool cds_handleConfigurationChange(cbs_t *cbs); // callback on Configuration Change -> must be implemented in application code

int8_t cbs_dlcToLength(uint8_t dlc);

uint8_t cbs_encodeDataFlags0(cbs_flags0_t flags);
cbs_flags0_t cbs_decodeDataFlags0(uint8_t byte);

#ifdef __cplusplus
}
#endif
#endif //CANBESERIAL_H_