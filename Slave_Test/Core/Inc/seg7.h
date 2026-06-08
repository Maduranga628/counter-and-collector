/*
 * seg7.h
 * Header file for 4 digit 7 segment display
 *
 *  Created on: Apr 8, 2026
 *      Author: Maduranga
 */

#ifndef INC_SEG7_H_
#define INC_SEG7_H_

#include "main.h"

/* Digit select — active LOW (common anode) */
// make sure pin numbers align
#define D1_HIGH() HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, 0)
#define D1_LOW() HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, 1)
#define D2_HIGH() HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, 0)
#define D2_LOW() HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, 1)
#define D3_HIGH() HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, 0)
#define D3_LOW() HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, 1)
#define D4_HIGH() HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, 0)
#define D4_LOW() HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, 1)

#define SEG7_DIGIT_DELAY_MS   5
#define SEG7_MAX_VALUE        9999

void SEG7_DisplayNumber(uint16_t number);


#endif /* INC_SEG7_H_ */
