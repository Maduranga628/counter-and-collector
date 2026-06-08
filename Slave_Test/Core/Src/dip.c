/*
 * dip.c
 *
 *  Created on: Apr 10, 2026
 *      Author: Maduranga
 */

#include "dip.h"

/* DIP switches use PULLUP — open = 1, closed = 0
 * We invert so that closed switch = 1 (active)
 * DIP0 = LSB, DIP4 = MSB
 * ID range: 0-31 (5 bits)
 */
uint8_t DIP_ReadID(void)
{
    uint8_t id = 0;

    id |= (!HAL_GPIO_ReadPin(DIP0_GPIO_Port, DIP0_Pin)) << 0;
    id |= (!HAL_GPIO_ReadPin(DIP1_GPIO_Port, DIP1_Pin)) << 1;
    id |= (!HAL_GPIO_ReadPin(DIP2_GPIO_Port, DIP2_Pin)) << 2;
    id |= (!HAL_GPIO_ReadPin(DIP3_GPIO_Port, DIP3_Pin)) << 3;
    id |= (!HAL_GPIO_ReadPin(DIP4_GPIO_Port, DIP4_Pin)) << 4;

    return id;
}
