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

static inline uint8_t ov7670_read_pixel()
{
    return 0;
}

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
    // Camera reset
    MIK32_OV7670_write_register(OV7670_REG_COM7, OV7670_COM7_RESET);

    return OV7670_STATUS_OK;
}

static void timer_init()
{
    Timer16_HandleTypeDef htimer16_1;
    htimer16_1.Instance = TIMER16_1;

    /* Настройка тактирования */
    htimer16_1.Clock.Source = TIMER16_SOURCE_INTERNAL_OSC32M;
    htimer16_1.CountMode = TIMER16_COUNTMODE_INTERNAL; /* При тактировании от Input1 не имеет значения */
    htimer16_1.Clock.Prescaler = TIMER16_PRESCALER_1;
    htimer16_1.ActiveEdge = TIMER16_ACTIVEEDGE_RISING; /* Выбирается при тактировании от Input1 */

    /* Настройка режима обновления регистра ARR и CMP */
    htimer16_1.Preload = TIMER16_PRELOAD_AFTERWRITE;

    /* Настройка триггера */
    htimer16_1.Trigger.Source = TIMER16_TRIGGER_TIM1_GPIO1_9;
    htimer16_1.Trigger.ActiveEdge = TIMER16_TRIGGER_ACTIVEEDGE_SOFTWARE; /* При использовании триггера значение должно быть отлично от software */
    htimer16_1.Trigger.TimeOut = TIMER16_TIMEOUT_DISABLE;                /* Разрешить повторное срабатывание триггера */

    /* Настройки фильтра */
    htimer16_1.Filter.ExternalClock = TIMER16_FILTER_NONE;
    htimer16_1.Filter.Trigger = TIMER16_FILTER_NONE;

    /* Настройка режима энкодера */
    htimer16_1.EncoderMode = TIMER16_ENCODER_DISABLE;

    htimer16_1.Waveform.Enable = TIMER16_WAVEFORM_GENERATION_ENABLE;
    htimer16_1.Waveform.Polarity = TIMER16_WAVEFORM_POLARITY_NONINVERTED;

    HAL_Timer16_Init(&htimer16_1);
    HAL_Timer16_StartPWM(&htimer16_1, 1, 1);
    /* 
     * Connect XCLK pin to TIMER2_0_CH0 for clock signals generation
     * XCLK is PORT_1_0 pin.
     */
    PAD_CONFIG->PORT_1_CFG |= 2 << (XCLK_PIN_NUM * 2);
    PAD_CONFIG->PORT_1_DS |= (XCLK_PIN_NUM * 2);
    /* Configure timer */
    PM->CLK_APB_P_SET = PM_CLOCK_APB_P_TIMER32_2_M;
    XCLK_TIMER->ENABLE = 0;
    XCLK_TIMER->TOP = 1;
    XCLK_TIMER->PRESCALER = 0;
    XCLK_TIMER->CONTROL =
        TIMER32_CONTROL_MODE_UP_M | TIMER32_CONTROL_CLOCK_PRESCALER_M;
    XCLK_TIMER->INT_MASK = 0;
    XCLK_TIMER->INT_CLEAR = 0xFFFFFFFF;

    /* Duty = 50% */
    XCLK_TIMER->CHANNELS[XCLK_TIMER_CHANNEL].OCR = 1;
    XCLK_TIMER->CHANNELS[XCLK_TIMER_CHANNEL].CNTRL =
        TIMER32_CH_CNTRL_MODE_PWM_M | TIMER32_CH_CNTRL_ENABLE_M;

    XCLK_TIMER->ENABLE = 1;

    /* 
     * Connect PCLK pin to TIMER1_0_CH0 for clock signals generation
     * PCLK is PORT_0_0 pin.
     */
    PAD_CONFIG->PORT_0_CFG |= 2 << (PCLK_PIN_NUM * 2);
    /* Configure timer */
    PM->CLK_APB_P_SET = PM_CLOCK_APB_P_TIMER32_1_M;
    PCLK_TIMER->ENABLE = 0;
    PCLK_TIMER->TOP = PCLK_TIMER_TOP;
    PCLK_TIMER->PRESCALER = 0;
    PCLK_TIMER->CONTROL =
        TIMER32_CONTROL_MODE_UP_M | TIMER32_CONTROL_CLOCK_PRESCALER_M;
    PCLK_TIMER->INT_MASK = 0;
    PCLK_TIMER->INT_CLEAR = 0xFFFFFFFF;
    
    /* Duty = 50% */
    PCLK_TIMER->CHANNELS[PCLK_TIMER_CHANNEL].ICR = (PCLK_TIMER->TOP+1)/2;
    /* Capture mode */
    PCLK_TIMER->CHANNELS[PCLK_TIMER_CHANNEL].CNTRL =
        (TIMER32_CH_CNTRL_MODE_CAPTURE_M 
            | TIMER32_CH_CNTRL_ENABLE_M 
            | TIMER32_CH_CNTRL_CAPTURE_POS_M) 
            & (~TIMER32_CH_CNTRL_DIR_M);
    PCLK_TIMER->ENABLE = 1;
}

static void configure_pins(OV7670_host* host)
{
    /* Save data pins and configure them to input */
    for (int32_t i = 0; i < 8; i++) {
        data_gpios[i] = host->pins->data[i]->gpio;
        data_pin_bits[i] = (1 << host->pins->data[i]->pin_num);
        data_gpios[i]->DIRECTION_IN = data_pin_bits[i];
    }
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

void OV7670_capture(uint32_t* dest, uint16_t width, uint16_t height,
                           volatile uint32_t* vsync_reg, uint32_t vsync_bit,
                           volatile uint32_t* hsync_reg, uint32_t hsync_bit)
{
    uint8_t* mik_dest = (uint8_t*)dest;
    while (*vsync_reg & vsync_bit)
        ; // Wait for VSYNC low (frame end)
    OV7670_disable_interrupts();
    while (!(*vsync_reg & vsync_bit))
        ; // Wait for VSYNC high (frame start)

    for (uint16_t y = 0; y < height; y++) { // For each row...
    while (*hsync_reg & hsync_bit)
        ; //  Wait for HSYNC low (row end)
    while (!(*hsync_reg & hsync_bit))
        ;                               //  Wait for HSYNC high (row start)
    for (int x = 0; x < width; x++) { //   For each column pair...
        /* Wait for clock */
        while (!(PCLK_PIN_GPIO->STATE & (1<< PCLK_PIN_NUM)))
        ;
        /* Read data */
        *mik_dest = ov7670_read_pixel();
        while (PCLK_PIN_GPIO->STATE & (1<< PCLK_PIN_NUM));
    }
    }
}