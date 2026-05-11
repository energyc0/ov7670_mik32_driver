#pragma once

#include "gpio.h"
#include "mik32_hal_i2c.h"
#include <stdint.h>

typedef struct {
    GPIO_TypeDef* gpio;
    uint32_t pin_bit;
} OV7670_pin;

void pinModeOutput(OV7670_pin* pin);
void digitalWrite(OV7670_pin* pin, uint8_t hi);

#define OV7670_delay_ms(x) HAL_DelayMs(x)
#define OV7670_pin_output(pin) pinModeOutput(pin);
#define OV7670_pin_write(pin, hi) digitalWrite(pin, hi ? 1 : 0)
#define OV7670_disable_interrupts() HAL_IRQ_DisableInterrupts()
#define OV7670_enable_interrupts() HAL_IRQ_EnableInterrupts()

#define OV7670_XCLK_HZ 24000000

typedef struct {
    I2C_HandleTypeDef* hi2c;
} OV7670_arch;

extern void OV7670_capture(uint32_t* dest, uint16_t width, uint16_t height,
                           volatile uint32_t* vsync_reg, uint32_t vsync_bit,
                           volatile uint32_t* hsync_reg, uint32_t hsync_bit);
