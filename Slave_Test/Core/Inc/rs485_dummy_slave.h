/*
 * rs485_dummy_slave.h
 *
 *  Created on: Apr 18, 2026
 *  Edited on:	Apr 20, 2026
 *      Author: Maduranga
 */

#ifndef INC_RS485_DUMMY_SLAVE_H_
#define INC_RS485_DUMMY_SLAVE_H_

#include "main.h"
#include "EEPROM_log.h"

// Constants
// To and From Slave and Master
#define RS485_START_BYTE		0xFF

// Receiving Commands from master
#define RS485_CMD_REQUEST		0xCA
#define RS485_CMD_DONE			0xCD

// DATA indicators from the Slave
#define RS485_MORE_DATA			0xDA
#define RS485_DATA_DONE			0xDD

// Frame Lengths

/*	REQUEST_DATA	: [START][slave_id][CMD_REQ]                    = 3 bytes
 *	DATA_FRAME		: [START][slave_id][session][c0][c1][MORE/DONE] = 6 bytes
 *	DONE			: [START][slave_id][CMD_DONE]                   = 3 bytes */

// From Master
#define RS485_REQUEST_LEN		3
#define RS485_DONE_LEN			3

// To Master
#define RS485_DATA_FRAME_LEN	6

// MAX Rx BUF Size
#define RS485_RX_BUF_SIZE		8

// No. of Slaves
#define	RS485_NUM_SLAVES		32	// For now. Can be Increased to 31. RS485 Bus can hold 32 devices

// Timeout
#define RS485_TIMEOUT			1000


// Function Declarations

// Initialization - Takes slave id, returns nothing
void RS485_Dummy_Slave_Init(uint8_t id);

// Rx Callback - takes uint8_t size, returns nothing
void RS485_Dummy_Slave_RxCallback(uint16_t size);

// Process - (takes uint8_t session_count, uint16_t *session_nos, uint16_t *counters) <- not doing this
// Passing arrays from main.c is complicated. Do EEPROM reads inside the Process
void RS485_Dummy_Slave_Process(void);


#endif /* INC_RS485_DUMMY_SLAVE_H_ */
