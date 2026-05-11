#include "ov7670.h"

#include "gpio.h"
#include "mik32_hal_i2c.h"
#include "mik32_hal.h"
#include "mik32_hal_gpio.h"
#include "mik32_hal_irq.h"
#include "mik32_memory_map.h"
#include "timer32.h"
#include "sccb.h"
#include <stdint.h>

static void timer_init(OV7670_host* host);
static void i2c_init(OV7670_host* host);

static GPIO_TypeDef* gpios[20] = {
    GPIO_0, //0
    GPIO_0, //1
    GPIO_0, //2
    GPIO_0, //3
    GPIO_0, //4
    GPIO_0, //5
    GPIO_0, //6
    GPIO_1, //7
    GPIO_1, //8
    GPIO_0, //9
    GPIO_1, //10
    GPIO_1, //11
    GPIO_1, //12
    GPIO_1, //13
    GPIO_1, //14
    GPIO_1, //15
    GPIO_0, //16
    GPIO_0, //17
    GPIO_1, //18
    GPIO_1 //19
};

static int8_t pin_bits[20] = {
    5, //0
    6, //1
    10, //2
    0, //3
    8, //4
    1, //5
    2, //6
    8, //7
    9, //8
    3, //9
    3, //10
    1, //11
    0, //12
    2, //13
    5, //14
    7, //15
    4, //16
    7, //17
    12, //18
    13 //19
};

void pinModeOutput(OV7670_pin* pin)
{
    pin->gpio->DIRECTION_OUT = pin->pin_bit;
}

void digitalWrite(OV7670_pin* pin, uint8_t hi)
{
    if (hi) 
        pin->gpio->SET = pin->pin_bit;
    else
        pin->gpio->CLEAR = pin->pin_bit;
}

OV7670_status OV7670_arch_begin(OV7670_host* host) {
    i2c_init(host);
    timer_init(host);
    
    return OV7670_STATUS_OK;
}

static void timer_init(OV7670_host* host)
{
    /* Connect pins 1 and 2 to TIMER2_0_CH0 */
    PAD_CONFIG->PORT_1_CFG |= 2 << (PWM_PIN1_NUM * 2);
    PAD_CONFIG->PORT_1_CFG |= 2 << (PWM_PIN2_NUM * 2);
    /* Turn on TIMER32_2 */
    PM->CLK_APB_P_SET = PM_CLOCK_APB_P_TIMER32_2_M;
    TIMER32_2->ENABLE = 0;
    TIMER32_2->TOP = PWM_PERIOD_TICKS;
    TIMER32_2->PRESCALER = 0;
    TIMER32_2->CONTROL =
        TIMER32_CONTROL_MODE_UP_M | TIMER32_CONTROL_CLOCK_PRESCALER_M;
    TIMER32_2->INT_MASK = 0;
    TIMER32_2->INT_CLEAR = 0xFFFFFFFF;

    TIMER32_2->CHANNELS[PWM_PIN1_TIMER_CHANNEL].OCR = PWM_DUTY_CYCLE_TICKS(led1_duty_cycle_percent);
    TIMER32_2->CHANNELS[PWM_PIN1_TIMER_CHANNEL].CNTRL =
        TIMER32_CH_CNTRL_MODE_PWM_M | TIMER32_CH_CNTRL_ENABLE_M;

    TIMER32_2->CHANNELS[PWM_PIN2_TIMER_CHANNEL].OCR = PWM_DUTY_CYCLE_TICKS(led2_duty_cycle_percent);;
    TIMER32_2->CHANNELS[PWM_PIN2_TIMER_CHANNEL].CNTRL =
        TIMER32_CH_CNTRL_MODE_PWM_M | TIMER32_CH_CNTRL_ENABLE_M;

    TIMER32_2->ENABLE = 1;
}
    

static void i2c_init(OV7670_host* host)
{
    host->arch->hi2c->Instance = I2C_1;
    host->arch->hi2c->Init.Mode = HAL_I2C_MODE_MASTER;

    host->arch->hi2c->Init.DigitalFilter = I2C_DIGITALFILTER_OFF;
    host->arch->hi2c->Init.AnalogFilter = I2C_ANALOGFILTER_DISABLE;
    host->arch->hi2c->Init.AutoEnd = I2C_AUTOEND_ENABLE;

    /* Настройка частоты */
    host->arch->hi2c->Clock.PRESC = 5;
    host->arch->hi2c->Clock.SCLDEL = 15;
    host->arch->hi2c->Clock.SDADEL = 15;
    host->arch->hi2c->Clock.SCLH = 15;
    host->arch->hi2c->Clock.SCLL = 15;

    if (HAL_I2C_Init(host->arch->hi2c) == HAL_OK)
    {
        HAL_USART_Print(&husart0, "I2C init OK\r\n", USART_TIMEOUT_DEFAULT);
    }
}

extern void OV7670_print(char* str)
{
    USART_Print(str);
}

int OV7670_read_register(void* platform, uint8_t reg)
{
    int val = -1;
    int ret = SCCB_ReadByte(reg, &val);
    if (ret) {
        USART_Print("SCCB_ReadByte() returned ");
        USART_PrintInt(val);
        USART_Print("\n\r");
    }
    return val;
}

void OV7670_write_register(void* platform, uint8_t reg, uint8_t value)
{
    int ret = SCCB_WriteByte(reg, value);
    if (ret) {
        USART_Print("SCCB_WriteByte() returned ");
        USART_PrintInt(val);
        USART_Print("\n\r");
    }
}
