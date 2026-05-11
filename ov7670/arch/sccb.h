#pragma once

#include "mik32_hal_i2c.h"
#include <stdint.h>

void SCCB_Init();

/* Write byte to OV7670 and return 0 on success. */
int SCCB_WriteByte(uint8_t reg_addr, uint8_t reg_data);

/* Read byte from OV7670 and return 0 on success and write to reg_data. */
int SCCB_ReadByte(uint8_t reg_addr, uint8_t *reg_data);