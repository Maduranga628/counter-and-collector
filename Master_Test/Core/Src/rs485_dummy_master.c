/*
 * rs485_master.c
 *
 *  Created on: Apr 15, 2026
 *  Edited on:	Apr 21, 2026
 *      Author: Maduranga
 */

#include "rs485_dummy_master.h"
#include <string.h>
#include <stdio.h>


// Private Variables
// UART1 Declaration for PC communication for debugging purposes - Delete after making proper uart_pc.c and .h
extern UART_HandleTypeDef huart1;

// UART3 Declaration for Enabling RS485
extern UART_HandleTypeDef huart3;


// Internal States
// rx_buf Holds incoming bytes
static			uint8_t		rx_buf[RS485_RX_BUF_SIZE];

// Callback Flags
static volatile uint8_t  	frame_ready = 0;
static volatile uint16_t 	rx_size 	= 0;


// Function Descriptions
// Initialization
void RS485_Dummy_Master_Init(void)
{

	//nothing needed

}

// RxCallback
void RS485_Dummy_Master_RxCallback(uint16_t size)
{

	rx_size 	= size;
	frame_ready = 1;

}

// Collection Process - With Collect button press
void RS485_Dummy_Master_Collect(void)
{

	// Local Variables of the RAM Buffer
	uint8_t		collected_sessions[RS485_MAX_SESSIONS];
	uint16_t	collected_counters[RS485_MAX_SESSIONS];
	uint8_t		collected_count = 0;

	// Slave Polling
	for(int i = 0; i < RS485_NUM_SLAVES; i++)
	{
		// Reset Frame ready and rx size to 0
		frame_ready = 0;
		rx_size 	= 0;

		// Clear rx_buf to 0
		memset(rx_buf, 0, RS485_RX_BUF_SIZE);

		// Collected count to 0, Each slave starts fresh
		collected_count = 0;

		// Arm for receive Data from slaves and 10ms delay for settle RX
		HAL_UARTEx_ReceiveToIdle_IT(&huart3, rx_buf, RS485_DATA_FRAME_LEN);
		HAL_Delay(10);

		// Tx frame buffer for Request data command
		uint8_t		tx_frame[RS485_REQUEST_LEN];

		// Fill the TX frame with Start byte, slave ID and CMD_Request
		tx_frame[0] = RS485_START_BYTE;
		tx_frame[1] = i;	// Slave ID, i increase to RS485_NUM_SLAVES declared in the header
		tx_frame[2] = RS485_CMD_REQUEST;

		// Transmit request
		HAL_UART_Transmit(&huart3, tx_frame, RS485_REQUEST_LEN, 200);

		// For the timeout calculation
		uint32_t start = HAL_GetTick();

		// While loop for receive data until DATA_DONE or timeout
		while(1)
		{

			// Timeout happens, break the loop
			if(HAL_GetTick() - start > RS485_TIMEOUT)
			{

				break;

			}

			if(frame_ready == 0)
			{

				continue; // Not Ready yet

			}

			{

				// if frame ready = 1
				frame_ready = 0;

				// Size check
				// if not equal Re-arm RX
				if(rx_size != RS485_DATA_FRAME_LEN)
				{

					//Re-arm and continue
					HAL_UARTEx_ReceiveToIdle_IT(&huart3, rx_buf, RS485_DATA_FRAME_LEN);
					continue;

				}

				// Validate ist two bytes, if not matching re-arm RX
				if(rx_buf[0] != RS485_START_BYTE || rx_buf[1] != i)
				{

					//Re-arm and continue
					HAL_UARTEx_ReceiveToIdle_IT(&huart3, rx_buf, RS485_DATA_FRAME_LEN);
					continue;

				}

				{

					// first byte matches, parse the frame
					collected_sessions[collected_count] = rx_buf[2];	// parse the session value
					collected_counters[collected_count] = (uint16_t)rx_buf[3] | ((uint16_t)rx_buf[4] << 8);
					collected_count++;

					// Check the final frame to determine data end
					if(rx_buf[5] == RS485_DATA_DONE)
					{

						break;

					}
					// Check the final frame to determine more data coming
					if(rx_buf[5] == RS485_MORE_DATA)
					{

						// Re-arm RX
						HAL_UARTEx_ReceiveToIdle_IT(&huart3, rx_buf, RS485_DATA_FRAME_LEN);
						continue;

					}

				}

			}

		}

		// Send Done Frame only if we got data
		if(collected_count > 0)
		{
			// Fill the TX frame with Start byte, slave ID and CMD_done
			tx_frame[0] = RS485_START_BYTE;
			tx_frame[1] = i;	// Slave ID, i increase to RS485_NUM_SLAVES declared in the header
			tx_frame[2] = RS485_CMD_DONE;

			// Transmit done page
			HAL_UART_Transmit(&huart3, tx_frame, RS485_DONE_LEN, 200);

		}


		// Added a delay for debugging
		HAL_Delay(1000);
		// Debug Prints to PC terminal
		// Variable for the sending
		char debug_str[64];

		// Loop for each session need to be send, used j as the variable becuase i was used before
		for(int j = 0; j < collected_count; j++)
		{

			sprintf(debug_str, "slave:%d, session:%d, counter:%d\r\n",
					i,
					collected_sessions[j],
					collected_counters[j]);

			// Send using UART1
			HAL_UART_Transmit(&huart1, (uint8_t*)debug_str, strlen(debug_str), 200);

		}


	}

}
