#pragma once

#include "mik32_hal_i2c.h"
#include <stdint.h>

/* PCC_GPIO_1_CLK must be enabled */
void SCCB_Init();

/* Write byte to OV7670 and return 0 on success. */
int SCCB_WriteReg(uint8_t reg_addr, uint8_t reg_data);

/* Read byte from OV7670 and return 0 on success and write to reg_data. */
int SCCB_ReadReg(uint8_t reg_addr, uint8_t *reg_data);