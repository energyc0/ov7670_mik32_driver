#pragma once

#include <stdint.h>

typedef int8_t OV7670_pin;

void pinModeOutput(int16_t pin);
void digitalWrite(int16_t pin, int16_t hi);

#define OV7670_delay_ms(x) HAL_DelayMs(x)
#define OV7670_pin_output(pin) pinModeOutput(pin);
#define OV7670_pin_write(pin, hi) digitalWrite(pin, hi ? 1 : 0)
#define OV7670_disable_interrupts() HAL_IRQ_DisableInterrupts()
#define OV7670_enable_interrupts() HAL_IRQ_EnableInterrupts()

