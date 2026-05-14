#pragma once

#include <stdint.h>

/* PCC_GPIO_1_CLK must be enabled */
void I2C_BitBang_Init();

/* Write I2C byte and return ACK bit. */
__attribute__((section(".ram_text"))) uint8_t I2C_BitBang_WriteByte(uint8_t byte, uint8_t is_start, uint8_t is_stop);

/* Read I2C byte. */
__attribute__((section(".ram_text"))) uint8_t I2C_BitBang_ReadByte(uint8_t ack_bit, uint8_t is_stop);