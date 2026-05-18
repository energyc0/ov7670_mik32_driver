#include "arch/mik32.h"
#include "arch/uart.h"
#include "mik32_hal_timer16.h"
#include "ov7670.h"

#include "gpio.h"
#include "mik32_hal_i2c.h"
#include "mik32_hal.h"
#include "mik32_hal_gpio.h"
#include "mik32_hal_irq.h"
#include "mik32_memory_map.h"
#include "timer32.h"
#include "timer16.h"
#include "sccb.h"
#include <stdint.h>

static GPIO_TypeDef* data_gpios[8];
static int32_t data_pin_bits[8];

static void timer_init();
static void configure_pins(OV7670_host* host);
static void pin_to_input(OV7670_pin pin);

__attribute__((section(".ram_text"), always_inline))  static inline uint8_t ov7670_read_byte()
{
    /* Wait for clock */
    while (!(PCLK_PIN_GPIO->STATE & PCLK_PIN_BIT));

    /* Read data, OV7670 is MSB */
    //register uint8_t byte = (gpio_state & 0x3F) | (((gpio_state & 600) >> 3) & 0xC0);

    uint8_t byte = 0;

    register uint32_t gpio_state = GPIO_0->STATE;
    for (int32_t i = 7; i >= 0; i--) {
        byte <<= 1;
        if ((gpio_state & data_pin_bits[i]))
            byte |= 1;
    }


    while (PCLK_PIN_GPIO->STATE & PCLK_PIN_BIT);
    return byte;
}

__attribute__((section(".ram_text"), always_inline))  static inline uint16_t ov7670_read_pixel()
{
    /* OV7670 is MSB, while MIK32 is LSB */
    return ((uint16_t)ov7670_read_byte() << 8) | (uint16_t)ov7670_read_byte();
}

void pinModeOutput(OV7670_pin pin)
{
    pin->gpio->DIRECTION_OUT = (1 << pin->pin_num);
}

void digitalWrite(OV7670_pin pin, uint8_t hi)
{
    if (hi) 
        pin->gpio->SET = (1 << pin->pin_num);
    else
        pin->gpio->CLEAR = (1 << pin->pin_num);
}

OV7670_status OV7670_arch_begin(OV7670_host* host) {
    configure_pins(host);
    timer_init();
    HAL_DelayMs(300);
    return OV7670_STATUS_OK;
}

static void timer_init()
{
    /* 
     * Connect XCLK pin to TIMER2_0_CH0 for clock signals generation
     * XCLK is PORT_1_0 pin.
     */
    PAD_CONFIG->PORT_1_CFG &= ~(0b11 << (XCLK_PIN_NUM * 2));
    PAD_CONFIG->PORT_1_CFG |= 0b10 << (XCLK_PIN_NUM * 2);
    /* Configure timer */
    PM->CLK_APB_P_SET = PM_CLOCK_APB_P_TIMER32_2_M;
    XCLK_TIMER->ENABLE = 0;
    XCLK_TIMER->TOP = XCLK_TIMER_TOP;
    XCLK_TIMER->PRESCALER = 0;
    XCLK_TIMER->CONTROL =
        TIMER32_CONTROL_MODE_UP_M | TIMER32_CONTROL_CLOCK_PRESCALER_M;
    XCLK_TIMER->INT_MASK = 0;
    XCLK_TIMER->INT_CLEAR = 0xFFFFFFFF;

    /* Duty = 50% */
    XCLK_TIMER->CHANNELS[XCLK_TIMER_CHANNEL].OCR = XCLK_TIMER_TOP/2 + 1;
    XCLK_TIMER->CHANNELS[XCLK_TIMER_CHANNEL].CNTRL =
        TIMER32_CH_CNTRL_MODE_PWM_M | TIMER32_CH_CNTRL_ENABLE_M;

    XCLK_TIMER->ENABLE = 1;
}

static void configure_pins(OV7670_host* host)
{
    /* Save data pins and configure them to input */
    for (int32_t i = 0; i < 8; i++) {
        data_gpios[i] = host->pins->data[i]->gpio;
        data_pin_bits[i] = (1 << host->pins->data[i]->pin_num);
        pin_to_input(host->pins->data[i]);
    }

    pin_to_input(host->pins->hsync);
    pin_to_input(host->pins->vsync);
    pin_to_input(host->pins->pclk);
}

static void pin_to_input(OV7670_pin pin)
{
    uint32_t shift = (pin->pin_num  * 2);
    if (pin->gpio == GPIO_0) {
        PAD_CONFIG->PORT_0_CFG &= ~(0b11 << shift);
        PAD_CONFIG->PORT_0_PUPD &= ~(0b11 << shift);
        //PAD_CONFIG->PORT_0_PUPD |= (0b10 << shift);
    } else  {
        PAD_CONFIG->PORT_1_CFG &= ~(0b11 << shift);
        PAD_CONFIG->PORT_1_PUPD &= ~(0b11 << shift);
        //PAD_CONFIG->PORT_1_PUPD |= (0b10 << shift);
    }
    pin->gpio->DIRECTION_IN = (1 << pin->pin_num);
}

void OV7670_print(char* str)
{
    USART_Print(str);
}

int OV7670_read_register(void* platform, uint8_t reg)
{
    return MIK32_OV7670_read_register(reg);
}

void OV7670_write_register(void* platform, uint8_t reg, uint8_t value)
{
    MIK32_OV7670_write_register(reg, value);
}

void MIK32_OV7670_write_register(uint8_t reg, uint8_t value)
{
    int ret = SCCB_WriteReg(reg, value);
    if (ret) {
        USART_Print("SCCB_WriteByte() returned ");
        USART_PrintInt(ret);
        USART_Print("\n\r");
    }
}

int MIK32_OV7670_read_register(uint8_t reg)
{
    uint8_t val = -1;
    int ret = SCCB_ReadReg(reg, &val);
    if (ret) {
        USART_Print("SCCB_ReadByte() returned ");
        USART_PrintInt(ret);
        USART_Print("\n\r");
    }
    return (int)val;
}

__attribute__((section(".ram_text"))) void OV7670_capture(uint32_t* dest, uint16_t width, uint16_t height,
                           volatile uint32_t* vsync_reg, uint32_t vsync_bit,
                           volatile uint32_t* hsync_reg, uint32_t hsync_bit)
{
    uint16_t* mik_dest = (uint16_t*)dest;
    OV7670_disable_interrupts();

    while (!(*vsync_reg & vsync_bit))
        ; // Wait for VSYNC high (frame start)
    while (*vsync_reg & vsync_bit)
        ; // Wait for VSYNC low (frame end)
    for (uint16_t y = 0; y < height; y++) { // For each row...
        while (*hsync_reg & hsync_bit)
            ; //  Wait for HSYNC low (row end)
        while (!(*hsync_reg & hsync_bit))
            ;                               //  Wait for HSYNC high (row start)
        for (uint16_t x = 0; x < width; x++) { //   For each column pair...
            (*mik_dest++) = ov7670_read_pixel();
        }
    }
    OV7670_enable_interrupts();
}