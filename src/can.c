//
// can: initializes and provides methods to interact with the CAN peripheral
//
#include "stm32g4xx_hal.h"
#include "usbd_cdc_if.h"
#include "can.h"
#include "led.h"
#include "error.h"
#include "system.h"


// Private variables
static FDCAN_HandleTypeDef can_handle;
//static FDCAN_FilterTypeDef filter;
static uint32_t prescaler;
static uint32_t data_prescaler;
enum can_bus_state bus_state;
static uint8_t can_autoretransmit = ENABLE;
static can_txbuf_t txqueue = {0};


// Initialize CAN peripheral settings, but don't actually start the peripheral
void can_init(void)
{
    // Initialize GPIO for CAN transceiver 
    GPIO_InitTypeDef GPIO_InitStruct;
    __HAL_RCC_FDCAN_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();


    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, 0); // CAN Standby - turn standby off (hw pull hi)


    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, 1); // CAN IO power


    //PB8     ------> CAN_RX
    //PB9     ------> CAN_TX
    GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF9_FDCAN1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);


    // Initialize default CAN filter configuration
    /*filter.FilterIdHigh = 0;
    filter.FilterIdLow = 0;
    filter.FilterMaskIdHigh = 0;
    filter.FilterMaskIdLow = 0;
    filter.FilterFIFOAssignment = CAN_RX_FIFO0;
    filter.FilterBank = 0;
    filter.FilterMode = CAN_FILTERMODE_IDMASK;
    filter.FilterScale = CAN_FILTERSCALE_32BIT;
    filter.FilterActivation = ENABLE;*/


    // default to 125 kbit/s
    can_set_bitrate(cbs_baudrate_500k);
    can_set_data_bitrate(cbs_baudrate_500k);
    can_handle.Instance = FDCAN1;
    bus_state = OFF_BUS;
}


// Start the CAN peripheral
void can_enable(void)
{
    if (bus_state == OFF_BUS)
    {
        can_handle.Init.ClockDivider = FDCAN_CLOCK_DIV1; // 144Mhz
//        can_handle.Init.FrameFormat = FDCAN_FRAME_CLASSIC;
        can_handle.Init.FrameFormat = FDCAN_FRAME_FD_BRS;


    	//can_handle.Init.Prescaler = prescaler;
    	can_handle.Init.Mode = FDCAN_MODE_NORMAL;
    	can_handle.Init.AutoRetransmission = can_autoretransmit;
        can_handle.Init.TransmitPause = DISABLE; // emz
        can_handle.Init.ProtocolException = DISABLE; // emz

        can_handle.Init.NominalPrescaler = prescaler; // 170mhz base clock
    	can_handle.Init.NominalSyncJumpWidth = 1;
    	can_handle.Init.NominalTimeSeg1 = 14;
    	can_handle.Init.NominalTimeSeg2 = 2;
        
        // FD only
        can_handle.Init.DataPrescaler = data_prescaler; // 2 for 5Mbit rate with 170 base clock
        can_handle.Init.DataSyncJumpWidth = 1;
        can_handle.Init.DataTimeSeg1 = 14;
        can_handle.Init.DataTimeSeg2 = 2;
        
        // For other bitrates with prescaler changes:
        // 2Mbits is same 14/2 with a prescalar of 5
        // Probably would prefer to change the time quanta instead

    	//can_handle.Init.TimeTriggeredMode = DISABLE;
    	//can_handle.Init.AutoBusOff = ENABLE;
    	//can_handle.Init.AutoWakeUp = DISABLE;
    	//can_handle.Init.ReceiveFifoLocked = DISABLE;
    	//can_handle.Init.TransmitFifoPriority = DISABLE;
        
        can_handle.Init.StdFiltersNbr = 0;
        can_handle.Init.ExtFiltersNbr = 0;
        can_handle.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;

        
        HAL_FDCAN_Init(&can_handle);

        //HAL_CAN_ConfigFilter(&can_handle, &filter);

        HAL_FDCAN_Start(&can_handle);
        bus_state = ON_BUS;
        led_blue_on();
    }
}


// Disable the CAN peripheral and go off-bus
void can_disable(void)
{
    if (bus_state == ON_BUS)
    {
        HAL_FDCAN_Stop(&can_handle);

        bus_state = OFF_BUS;
        led_blue_off();
    }
}


void can_set_data_bitrate(cbs_baudrate_t baudrate)
{
    switch (baudrate){
        case cbs_baudrate_10k: data_prescaler = 1000; break;
        case cbs_baudrate_20k: data_prescaler = 500; break;
        case cbs_baudrate_50k: data_prescaler = 200; break;
        case cbs_baudrate_100k: data_prescaler = 100; break;
        case cbs_baudrate_125k: data_prescaler = 80; break;
        case cbs_baudrate_250k: data_prescaler = 40; break;
        case cbs_baudrate_500k: data_prescaler = 20; break;
        case cbs_baudrate_1M: data_prescaler = 10; break;
        case cbs_baudrate_2M: data_prescaler = 5; break;
        case cbs_baudrate_5M: data_prescaler = 2; break;
        case cbs_baudrate_10M: data_prescaler = 1; break;
    }
}

// Set the bitrate of the CAN peripheral
void can_set_bitrate(cbs_baudrate_t baudrate)
{
    switch (baudrate){
        case cbs_baudrate_10k: prescaler = 1000; break;
        case cbs_baudrate_20k: prescaler = 500; break;
        case cbs_baudrate_50k: prescaler = 200; break;
        case cbs_baudrate_100k: prescaler = 100; break;
        case cbs_baudrate_125k: prescaler = 80; break;
        case cbs_baudrate_250k: prescaler = 40; break;
        case cbs_baudrate_500k: prescaler = 20; break;
        case cbs_baudrate_1M: prescaler = 10; break;
        case cbs_baudrate_2M: prescaler = 5; break;
        case cbs_baudrate_5M: prescaler = 2; break;
        case cbs_baudrate_10M: prescaler = 1; break;
    }
}


// Set CAN peripheral to silent mode
void can_set_silent(uint8_t silent)
{
    if (bus_state == ON_BUS)
    {
        // cannot set silent mode while on bus
        return;
    }
    if (silent)
    {
    	can_handle.Init.Mode = FDCAN_MODE_BUS_MONITORING; // !!!?!?!
    } else {
    	can_handle.Init.Mode = FDCAN_MODE_NORMAL;
    }
}


// Set CAN peripheral to silent mode
void can_set_autoretransmit(uint8_t autoretransmit)
{
    if (bus_state == ON_BUS)
    {
        // Cannot set autoretransmission while on bus
        return;
    }
    if (autoretransmit)
    {
    	can_autoretransmit = ENABLE;
    } else {
    	can_autoretransmit = DISABLE;
    }
}


// Convert a FDCAN_data_length_code to number of bytes in a message
int8_t hal_dlc_code_to_bytes(uint32_t hal_dlc_code)
{
    switch(hal_dlc_code)
    {
        case FDCAN_DLC_BYTES_0:
            return 0;
        case FDCAN_DLC_BYTES_1:
            return 1;
        case FDCAN_DLC_BYTES_2:
            return 2;
        case FDCAN_DLC_BYTES_3:
            return 3;
        case FDCAN_DLC_BYTES_4:
            return 4;
        case FDCAN_DLC_BYTES_5:
            return 5;
        case FDCAN_DLC_BYTES_6:
            return 6;
        case FDCAN_DLC_BYTES_7:
            return 7;
        case FDCAN_DLC_BYTES_8:
            return 8;
        case FDCAN_DLC_BYTES_12:
            return 12;
        case FDCAN_DLC_BYTES_16:
            return 16;
        case FDCAN_DLC_BYTES_20:
            return 20;
        case FDCAN_DLC_BYTES_24:
            return 24;
        case FDCAN_DLC_BYTES_32:
            return 32;
        case FDCAN_DLC_BYTES_48:
            return 48;
        case FDCAN_DLC_BYTES_64:
            return 64;
        default:
            return -1;
    }
}

// Send a message on the CAN bus. Called from USB ISR.
uint32_t can_tx(FDCAN_TxHeaderTypeDef *tx_msg_header, uint8_t* tx_msg_data)
{
	// If when we increment the head we're going to hit the tail
	// (if we're filling the last spot in the queue)
	if( ((txqueue.head + 1) % TXQUEUE_LEN) == txqueue.tail)
	{
		error_assert(ERR_FULLBUF_CANTX);
		return HAL_ERROR;
	}

	// Convert length to bytes
	uint32_t len = hal_dlc_code_to_bytes(tx_msg_header->DataLength);

	// Don't overrun buffer element max length
	if(len > TXQUEUE_DATALEN)
		return HAL_ERROR;

	// Save the header to the circular buffer
	txqueue.header[txqueue.head] = *tx_msg_header;

	// Copy the data to the circular buffer
	for(uint8_t i=0; i<len; i++)
	{
		txqueue.data[txqueue.head][i] = tx_msg_data[i];
	}

	// Increment the head pointer
	txqueue.head = (txqueue.head + 1) % TXQUEUE_LEN;

	return HAL_OK;
}


// Process data from CAN tx/rx circular buffers
void can_process(void)
{
	while((txqueue.tail != txqueue.head) && (HAL_FDCAN_GetTxFifoFreeLevel(&can_handle) > 0))
	{
		uint32_t status;

		// Transmit can frame
		status = HAL_FDCAN_AddMessageToTxFifoQ(&can_handle, &txqueue.header[txqueue.tail], txqueue.data[txqueue.tail]);
		txqueue.tail = (txqueue.tail + 1) % TXQUEUE_LEN;

		// This drops the packet if it fails (no retry). Failure is unlikely
		// since we check if there is a TX mailbox free.
		if(status != HAL_OK)
		{
			error_assert(ERR_CAN_TXFAIL);
		}

		led_green_on();
	}
}


// Receive message from the CAN bus (blocking)
uint32_t can_rx(FDCAN_RxHeaderTypeDef *rx_msg_header, uint8_t* rx_msg_data)
{
    uint32_t status;
    status = HAL_FDCAN_GetRxMessage(&can_handle, FDCAN_RX_FIFO0, rx_msg_header, rx_msg_data);


    return status;
}


// Check if a CAN message has been received and is waiting in the FIFO
uint8_t is_can_msg_pending(uint8_t fifo)
{
    if (bus_state == OFF_BUS) return 0;
    return(HAL_FDCAN_GetRxFifoFillLevel(&can_handle, FDCAN_RX_FIFO0) > 0);
}


// Return reference to CAN handle
FDCAN_HandleTypeDef* can_gethandle(void)
{
	return &can_handle;
}
