#pragma once

#include "gpio.h"
#include "mik32_hal_i2c.h"
#include <stdint.h>

typedef struct {
    GPIO_TypeDef* gpio;
    uint8_t pin_num;
} Pin;

typedef Pin* OV7670_pin;

void pinModeOutput(OV7670_pin pin);
void digitalWrite(OV7670_pin pin, uint8_t hi);

#define MIK32_SYSTEM_FREQ_HZ (32000000UL)

#define OV7670_delay_ms(x) HAL_DelayMs(x)
#define OV7670_pin_output(pin) pinModeOutput(pin);
#define OV7670_pin_write(pin, hi) digitalWrite(pin, hi ? 1 : 0)
#define OV7670_disable_interrupts() HAL_IRQ_DisableInterrupts()
#define OV7670_enable_interrupts() HAL_IRQ_EnableInterrupts()

#define XCLK_PIN_NUM (3)
#define XCLK_PIN_GPIO (GPIO_1)
#define XCLK_TIMER (TIMER32_2)
#define XCLK_TIMER_CHANNEL (3)
#define XCLK_FREQUENCY (16000000UL)
#define XCLK_TIMER_TOP ((MIK32_SYSTEM_FREQ_HZ / XCLK_FREQUENCY)-1)

#define PCLK_PIN_NUM (1)
#define PCLK_PIN_GPIO (GPIO_1)

#define OV7670_XCLK_HZ (XCLK_FREQUENCY)
#define OV7670_PCLK_HZ (OV7670_XCLK_HZ) // 30 FPS
#define PCLK_TIMER_TOP (MIK32_SYSTEM_FREQ_HZ / OV7670_PCLK_HZ)
#define OV7670_FPS (30)

#define CAMERA_WIDTH (40)
#define CAMERA_HEIGHT (30)

typedef struct {
    I2C_HandleTypeDef* hi2c;
} OV7670_arch;

extern void OV7670_capture(uint32_t* dest, uint16_t width, uint16_t height,
                           volatile uint32_t* vsync_reg, uint32_t vsync_bit,
                           volatile uint32_t* hsync_reg, uint32_t hsync_bit);

int MIK32_OV7670_read_register(uint8_t reg);
void MIK32_OV7670_write_register(uint8_t reg, uint8_t value);