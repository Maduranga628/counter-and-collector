/*
 * rs485_dummy_slave.c
 *
 *  Created on: Apr 18, 2026
 *  Edited on:	Apr 21, 2026
 *      Author: Maduranga
 */

#include "rs485_dummy_slave.h"
#include <string.h>
#include "EEPROM_log.h"
#include "dip.h"


// Private Variables
// UART3 Declaration
extern UART_HandleTypeDef huart3;

// Internal States
// rx_buf Holds incoming bytes
static 			uint8_t		rx_buf[RS485_RX_BUF_SIZE];

// Callback flags
static volatile uint8_t		frame_ready = 0;
static volatile uint16_t	rx_size 	= 0;

//Slave ID - 0 for now
static 			uint8_t 	slave_id;


// Function Descriptions
// Initialization
void RS485_Dummy_Slave_Init(uint8_t id)
{

	// Pass the Slave ID to this file
	slave_id = DIP_ReadID();
	// Arm receiving to idle interrupt with 'RS485_REQUEST_LEN' length(3 bytes)
	HAL_UARTEx_ReceiveToIdle_IT(&huart3, rx_buf, RS485_REQUEST_LEN);

}

// RxCallback
void RS485_Dummy_Slave_RxCallback(uint16_t size)
{

	// Check incoming size equal to 'RS485_REQUEST_LEN' length(3 bytes)
	//if(size == RS485_REQUEST_LEN)
	//	frame_ready = 1; // Set the flag

	//Debug
	if(size == RS485_REQUEST_LEN && rx_buf[1] == slave_id)
		frame_ready = 1;

	else
	// Wrong incoming message, ignore and Re-Arm receiving to idle interrupt
		HAL_UARTEx_ReceiveToIdle_IT(&huart3, rx_buf, RS485_REQUEST_LEN);

}

//Process
void RS485_Dummy_Slave_Process(void)
{

	//Check frame ready
	if(frame_ready == 0) 	// frame ready = 0, return immediately
		return;
	else					// if it is 1, make frame ready to 0 and move to next step
		frame_ready = 0;

	// Validate incoming frame
	// If NOT START_BYTE correct AND slave_id correct AND CMD_REQUEST correct return and Re-arm
	if(!(rx_buf[0] == RS485_START_BYTE	&&
		 rx_buf[1] == slave_id			&&
		 rx_buf[2] == RS485_CMD_REQUEST ))
	{

		//Re-Arm and Return
		HAL_UARTEx_ReceiveToIdle_IT(&huart3, rx_buf, RS485_REQUEST_LEN);
		return;

	}

	// Valid Message
	{
		//Process
		//Local variables
		uint8_t 		session_count = 0;
		SessionRecord_t	rec = {0};
		uint8_t			sn;		// Loop variable
		uint8_t			found; 	// Loop Out Variable

		uint8_t			tx_frame[RS485_DATA_FRAME_LEN]; // Outgoing frame buffer

		// Search for 0 to 255 of session numbers in the EEPROM
		for(sn = 0; sn<255; sn++)
		{

			found = EEPROM_LOG_ReadSession(sn, &rec);

			// If session was found increase the session_no count
			if(found == 1)
			{
				session_count++;
			}

			// If not break it
			if(found == 0)
			{
				break;
			}
		}

		// Fill the tx_frame and transmit it
		// 0 to session count for loop, calls EEPROM_LOG_Read to get data
		for(int i = 0; i < session_count; i++)
		{

			// Read EEPROM and take it to the RAM
			EEPROM_LOG_ReadSession(i, &rec);

			// Fill the TX Frame
			tx_frame[0] = RS485_START_BYTE;
			tx_frame[1] = slave_id;
			tx_frame[2] = rec.session_no;
			tx_frame[3] = rec.counter & 0xFF;
			tx_frame[4] = rec.counter >> 8;
			tx_frame[5] = (i == session_count -1) ? RS485_DATA_DONE : RS485_MORE_DATA; // if last session send data done.

			//Send each tx_frame with 20ms delay
			HAL_UART_Transmit(&huart3, tx_frame, RS485_DATA_FRAME_LEN, 200);
			HAL_Delay(20);

		}

		// Re-arm for Done message
		HAL_UARTEx_ReceiveToIdle_IT(&huart3, rx_buf, RS485_DONE_LEN);

		// Timeout wait loop
		uint32_t start = HAL_GetTick();

		while(frame_ready == 0)
		{

			if(HAL_GetTick() - start > RS485_TIMEOUT)
			{

				// Timed out - Re-arm for next request and return
				HAL_UARTEx_ReceiveToIdle_IT(&huart3, rx_buf, RS485_REQUEST_LEN);
				return;

			}

		}

		// Clear flag
		frame_ready = 0;

		// Validate the done message
		if(!(rx_buf[0] == RS485_START_BYTE 	&&
			 rx_buf[1] == slave_id			&&
			 rx_buf[2] == RS485_CMD_DONE	))
		{
			// Invalid Done Command Frame- Re-arm and return
			HAL_UARTEx_ReceiveToIdle_IT(&huart3, rx_buf, RS485_REQUEST_LEN);
			return;
		}

		//Mark session forwarded step
		//Since EEPROM_LOG_MarkCollected was removed this is not necessary. So just Re-arm and Return
		HAL_UARTEx_ReceiveToIdle_IT(&huart3, rx_buf, RS485_REQUEST_LEN);
		return;

	}


}

