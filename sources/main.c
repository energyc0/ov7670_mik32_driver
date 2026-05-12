#include "arch/mik32.h"
#include "mik32_hal_gpio.h"
#include "mik32_hal_pcc.h"
#include "mik32_hal_usart.h"
#include "mik32_hal_i2c.h"
#include "mik32_hal_ssd1306.h"
#include "ov7670.h"
#include "arch/sccb.h"
#include "arch/uart.h"

/* 
 * Pins for ov7670 on MIK32 ELBEAR UNO:
 * D0-D7 are D4-D11
 * SIOC is SCL
 * SIOD is SDA
 * HREF is A4
 * VSYNC is A5
 * XCLK is D13
 * PCLK is D3
 * RESET is 3.3V
 * PWDN is GND
 */

static Pin scl = {
    .gpio = GPIO_1,
    .pin_num = 13
};
static Pin sda = {
    .gpio = GPIO_1,
    .pin_num = 12
};

static Pin d0 = {
    .gpio = GPIO_0,
    .pin_num = 8
};
static Pin d1 = {
    .gpio = GPIO_0,
    .pin_num = 1
};
static Pin d2 = {
    .gpio = GPIO_0,
    .pin_num = 2
};
static Pin d3 = {
    .gpio = GPIO_1,
    .pin_num = 8
};
static Pin d4 = {
    .gpio = GPIO_1,
    .pin_num = 9
};
static Pin d5 = {
    .gpio = GPIO_0,
    .pin_num = 3
};
static Pin d6 = {
    .gpio = GPIO_1,
    .pin_num = 3
};
static Pin d7 = {
    .gpio = GPIO_1,
    .pin_num = 1
};
static Pin xclk = {
    .gpio = XCLK_PIN_GPIO,
    .pin_num = XCLK_PIN_NUM
};
static Pin pclk = {
    .gpio = PCLK_PIN_GPIO,
    .pin_num = PCLK_PIN_NUM
};
static Pin vsync = {
    .gpio = GPIO_1,
    .pin_num = 5
};
static Pin href = {
    .gpio = GPIO_1,
    .pin_num = 7
};

static void SystemClock_Config();
static void GPIO_Init();
static void Pins_Init(OV7670_pins* pins);
static void Arch_Init(OV7670_arch* arch);

USART_HandleTypeDef husart0;

int main()
{
    SystemClock_Config();
    GPIO_Init();
    USART_Init();
    SCCB_Init();

    OV7670_pins pins;
    Pins_Init(&pins);
    OV7670_arch arch;
    Arch_Init(&arch);

    OV7670_host host;
    host.pins = &pins;
    host.arch = &arch;

    if (OV7670_begin(&host, OV7670_COLOR_YUV, OV7670_SIZE_DIV16, 30.0) != OV7670_STATUS_OK) {
        HAL_USART_Print(&husart0, "Failed to initialize camera!", USART_TIMEOUT_DEFAULT);
        while (1);
    }

    /* Debug prints */
    uint8_t pid = MIK32_OV7670_read_register(OV7670_REG_PID); // Should be 0x76
    uint8_t ver = MIK32_OV7670_read_register(OV7670_REG_VER); // Should be 0x73
    USART_Print("PID: ");
    USART_PrintInt(pid);
    USART_Print(" and VER: ");
    USART_PrintInt(ver);
    USART_Print("\r\n");
    
    while (1) {
        GPIO_2->OUTPUT ^= GPIO_PIN_7;
        
        HAL_DelayMs(1000);
    }
}

static void SystemClock_Config()
{
    PCC_InitTypeDef PCC_OscInit = { 0 };

    PCC_OscInit.OscillatorEnable = PCC_OSCILLATORTYPE_ALL;
    PCC_OscInit.FreqMon.OscillatorSystem = PCC_OSCILLATORTYPE_OSC32M;
    PCC_OscInit.FreqMon.ForceOscSys = PCC_FORCE_OSC_SYS_UNFIXED;
    PCC_OscInit.FreqMon.Force32KClk = PCC_FREQ_MONITOR_SOURCE_OSC32K;
    PCC_OscInit.AHBDivider = 0;
    PCC_OscInit.APBMDivider = 0;
    PCC_OscInit.APBPDivider = 0;
    PCC_OscInit.HSI32MCalibrationValue = 128;
    PCC_OscInit.LSI32KCalibrationValue = 8;
    PCC_OscInit.RTCClockSelection = PCC_RTC_CLOCK_SOURCE_AUTO;
    PCC_OscInit.RTCClockCPUSelection = PCC_CPU_RTC_CLOCK_SOURCE_OSC32K;
    HAL_PCC_Config(&PCC_OscInit);
}

static void GPIO_Init()
{
    GPIO_InitTypeDef GPIO_InitStruct = { 0 };

    __HAL_PCC_GPIO_0_CLK_ENABLE();
    __HAL_PCC_GPIO_1_CLK_ENABLE();
    __HAL_PCC_GPIO_2_CLK_ENABLE();
    __HAL_PCC_GPIO_IRQ_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_7;
    GPIO_InitStruct.Mode = HAL_GPIO_MODE_GPIO_OUTPUT;
    GPIO_InitStruct.Pull = HAL_GPIO_PULL_NONE;
    HAL_GPIO_Init(GPIO_2, &GPIO_InitStruct);
}

static void Pins_Init(OV7670_pins* pins)
{
    pins->sda = &sda;
    pins->scl = &scl;
    pins->enable = NULL; // Not set 
    pins->reset = NULL; // Not set
    pins->vsync = &vsync;
    pins->hsync = &href;
    pins->xclk = &xclk;
    pins->pclk = &pclk;
    pins->data[0] = &d0;
    pins->data[1] = &d1;
    pins->data[2] = &d2;
    pins->data[3] = &d3;
    pins->data[4] = &d4;
    pins->data[5] = &d5;
    pins->data[6] = &d6;
    pins->data[7] = &d7;
}

static void Arch_Init(OV7670_arch* arch)
{
}
