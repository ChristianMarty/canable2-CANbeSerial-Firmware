#ifndef CANBESERIAL_H_
#define CANBESERIAL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "cobs_u8.h"

#define CBS_DEVICE_INFORMATION " christian-marty.ch/electricthings/CANbeSerial\x00\x00"

#define CBS_PROTOCOL_VERSION 1

#define TX_BUFFER_SIZE 500
#define RX_BUFFER_SIZE 500
#define MAX_FRAME_SIZE 70

typedef enum {
    cbs_baudrate_10k,
    cbs_baudrate_20k,
    cbs_baudrate_50k,
    cbs_baudrate_100k,
    cbs_baudrate_125k,
    cbs_baudrate_250k,
    cbs_baudrate_500k,
    cbs_baudrate_1M,
    cbs_baudrate_2M,
    cbs_baudrate_5M,
    cbs_baudrate_10M,
    cbs_baudrate_error = 0xFF
} cbs_baudrate_t;

typedef struct {
    cbs_baudrate_t baudrate;
    cbs_baudrate_t fdBaudrate;
    union {
        struct {
            uint8_t enabled: 1;
            uint8_t automaticRetransmission: 1;
            uint8_t silentMode: 1;
        } bits;
        uint8_t  byte;
    } bus;
} cbs_configuration_t ;

typedef struct {
    uint32_t identifier;
    union {
        struct {
            uint8_t extended: 1;
            uint8_t fd: 1;
            uint8_t rtr: 1;
            uint8_t reserved: 1;
            uint8_t dlc: 4;
        } bits;
        uint8_t byte;
    } flags;
    uint8_t data[64]; // data over in the DLC specified length will be omitted.
} cbs_data_t ;

typedef struct {
    cbs_configuration_t configuration;

    uint16_t txIndex;
    uint8_t txData[TX_BUFFER_SIZE];

    uint16_t rxIndex;
    uint8_t rxData[RX_BUFFER_SIZE];
    cobs_decodeStream_t cobsDecoder;
} cbs_t;

typedef enum {
    cbs_error_crc,
    cbs_error_txBufferFull,
    cbs_error_txFrameMalformatted,
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

    cbs_protocolVersion = 0x02,
    cbs_protocolVersionRequest = 0x82,

    cbs_configurationState = 0x03,
    cbs_configurationStateRequest = 0x83,
    cbs_configurationStateCommand = 0xC3,

    cbs_deviceInformation = 0x04,
    cbs_deviceInformationRequest = 0x84
} cbs_payloadId_t;


void cbs_init(cbs_t *cbs);
void cbs_onSerialDataReceived(cbs_t *cbs, uint8_t *data, uint8_t dataLength);

void cbs_sendData(cbs_t *cbs, cbs_data_t *data); // send can frame on serial port
void cbs_sendError(cbs_t *cbs, cbs_error_t error);

void cbs_handleDataFrame(cbs_t *cbs, cbs_data_t *data); // callback on data -> must be implemented in application code
void cds_handleConfigurationChange(cbs_t *cbs); // callback on Configuration Change -> must be implemented in application code

int8_t cbs_dlcToLength(uint8_t dlc);

#ifdef __cplusplus
}
#endif
#endif //CANBESERIAL_H_