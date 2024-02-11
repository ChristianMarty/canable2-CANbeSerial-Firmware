//
// CANable2 firmware
//

#include "stm32g4xx.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"
#include "can.h"
#include "system.h"
#include "led.h"
#include "CANbeSerial.h"

cbs_t cbs;

void serial_data(const uint8_t *data, uint8_t size)
{
    cbs_onSerialDataReceived(&cbs, data, size);
}

void cds_handleConfigurationChange(cbs_t *cbs)
{
    can_disable();
    can_set_silent(cbs->configuration.bus.bits.silentMode);
    can_set_autoretransmit(cbs->configuration.bus.bits.automaticRetransmission);
    can_set_bitrate(cbs->configuration.baudrate);
    can_set_data_bitrate(cbs->configuration.fdBaudrate);
    can_enable();
}

void cbs_handleDataFrame(cbs_t *cbs, cbs_data_t *data)
{
    // Set default header. All values overridden below as needed.
    FDCAN_TxHeaderTypeDef frame_header =
    {
        .TxFrameType = FDCAN_DATA_FRAME,
        .FDFormat = FDCAN_CLASSIC_CAN, // default to classic frame
        .IdType = FDCAN_STANDARD_ID, // default to standard ID
        .BitRateSwitch = FDCAN_BRS_OFF, // no bitrate switch
        .ErrorStateIndicator = FDCAN_ESI_ACTIVE, // error active
        .TxEventFifoControl = FDCAN_NO_TX_EVENTS, // don't record tx events
        .MessageMarker = 0, // ?
    };

    if(data->flags.bits.rtr) frame_header.TxFrameType = FDCAN_DATA_FRAME;
    else frame_header.TxFrameType = FDCAN_DATA_FRAME;

    if(data->flags.bits.fd) frame_header.FDFormat = FDCAN_CLASSIC_CAN;
    else frame_header.FDFormat = FDCAN_FD_CAN;

    if(data->flags.bits.extended) frame_header.IdType = FDCAN_STANDARD_ID;
    else frame_header.IdType = FDCAN_EXTENDED_ID;

    if(cbs->configuration.baudrate == cbs->configuration.fdBaudrate)frame_header.BitRateSwitch = FDCAN_BRS_OFF;
    else frame_header.BitRateSwitch = FDCAN_BRS_ON;

    frame_header.DataLength = data->flags.bits.dlc;

    can_tx(&frame_header, &data->data[0]);
}

int main(void)
{
    // Initialize peripherals
    system_init();
    can_init();
    led_init();
    usb_init();

    cbs_init(&cbs);

    // Power-on blink sequence
   // led_blue_blink(2);

    while(1)
    {
        led_process();
        can_process();
        cdc_process();

        cdc_transmit(cbs.txData, cbs.txIndex);
        cbs.txIndex = 0;

        // Message has been received, pull it from the buffer
        if(is_can_msg_pending(FDCAN_RX_FIFO0))
        {
            cbs_data_t data;
            FDCAN_RxHeaderTypeDef rx_msg_header;
			if (can_rx(&rx_msg_header, &data.data[0]) == HAL_OK) {
                data.identifier = rx_msg_header.Identifier;
                data.flags.bits.dlc = rx_msg_header.DataLength;
                data.flags.bits.fd = rx_msg_header.FDFormat;
                data.flags.bits.extended = rx_msg_header.IdType;
                cbs_sendData(&cbs, &data);
            }else{
                cbs_sendError(&cbs,cbs_error_can_rx);
            }
        }
    }
}

