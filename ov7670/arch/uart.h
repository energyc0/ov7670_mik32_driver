#pragma once

#include <stdint.h>

void USART_Init();
void USART_Print(char* str);
void USART_PrintInt(int64_t val);
void USART_WriteData(char* buf, uint32_t count);
