/*
 * seg7.c
 * Source file for 4 digit 7 segment display
 *
 *  Created on: Apr 8, 2026
 *      Author: Maduranga
 */

#include "seg7.h"

/* Segment encoding for 0–9, common anode (active LOW, so inverted) */
static const uint8_t segmentNumber[10] = {
		~0x3f, // 0
		~0x06, // 1
		~0x5b, // 2
		~0x4f, // 3
		~0x66, // 4
		~0x6d, // 5
		~0x7d, // 6
		~0x07, // 7
		~0x7f, // 8
		~0x67, // 9
};

/* Write one byte to segments a–g on GPIOA pins 0–6 */
static void SEG7_WriteSegments(uint8_t pattern)
{
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, (pattern>>0)&1);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, (pattern>>1)&1);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, (pattern>>2)&1);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, (pattern>>3)&1);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, (pattern>>4)&1);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, (pattern>>5)&1);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, (pattern>>6)&1);
}

/* Clamp value and split into 4 digits, then multiplex */
void SEG7_DisplayNumber(uint16_t number)
{
    if (number > SEG7_MAX_VALUE) number = SEG7_MAX_VALUE;

    uint8_t d1 = number / 1000;
    uint8_t d2 = (number / 100) % 10;
    uint8_t d3 = (number / 10)  % 10;
    uint8_t d4 = number % 10;

    /* Digit 1 (thousands) */
    SEG7_WriteSegments(segmentNumber[d1]);
    D1_LOW();
    HAL_Delay(SEG7_DIGIT_DELAY_MS);
    D1_HIGH();

    /* Digit 2 (hundreds) */
    SEG7_WriteSegments(segmentNumber[d2]);
    D2_LOW();
    HAL_Delay(SEG7_DIGIT_DELAY_MS);
    D2_HIGH();

    /* Digit 3 (tens) */
    SEG7_WriteSegments(segmentNumber[d3]);
    D3_LOW();
    HAL_Delay(SEG7_DIGIT_DELAY_MS);
    D3_HIGH();

    /* Digit 4 (units) */
    SEG7_WriteSegments(segmentNumber[d4]);
    D4_LOW();
    HAL_Delay(SEG7_DIGIT_DELAY_MS);
    D4_HIGH();
}


