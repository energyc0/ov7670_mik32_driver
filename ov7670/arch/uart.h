#pragma once

#include <stdint.h>

void USART_Init();
void USART_Print(char* str);
void USART_PrintInt(int64_t val);
void USART_WriteData(uint8_t* buf, uint32_t count);
void USART_PrintHex(uint8_t byte);
