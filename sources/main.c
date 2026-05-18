#include "arch/mik32.h"
#include "mik32_hal.h"
#include "mik32_hal_gpio.h"
#include "mik32_hal_pcc.h"
#include "mik32_hal_timer32.h"
#include "mik32_hal_usart.h"
#include "mik32_hal_i2c.h"
#include "mik32_memory_map.h"
#include "ov7670.h"
#include "arch/sccb.h"
#include "arch/i2c_bitbang.h"
#include "arch/uart.h"
#include <stdint.h>
#include "scr1_timer.h"

/* 
 * Pins for ov7670 on MIK32 ELBEAR UNO:
 * D0 is D3 - PORT_0_0
 * D1 is D5 - PORT_0_1
 * D2 is D6 - PORT_0_2
 * D3 is D9 - PORT_0_3
 * D4 is A2 - PORT_0_4
 * D5 is RX - PORT_0_5
 * D6 is A4 - PORT_0_9
 * D7 is D2 - PORT_0_10
 * SIOC is SCL - PORT_1_13
 * SIOD is SDA - PORT_1_12
 * HREF is D13 - PORT_1_2
 * VSYNC is D12 - PORT_1_0
 * XCLK is D10 - PORT_1_3
 * PCLK is D11 - PORT_1_1
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
    .pin_num = 0
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
    .gpio = GPIO_0,
    .pin_num = 3
};
static Pin d4 = {
    .gpio = GPIO_0,
    .pin_num = 4
};
static Pin d5 = {
    .gpio = GPIO_0,
    .pin_num = 5
};
static Pin d6 = {
    .gpio = GPIO_0,
    .pin_num = 9
};
static Pin d7 = {
    .gpio = GPIO_0,
    .pin_num = 10
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
    .pin_num = 0
};
static Pin href = {
    .gpio = GPIO_1,
    .pin_num = 2
};

static void SystemClock_Config();
static void GPIO_Init();
static void Pins_Init(OV7670_pins* pins);
static void Arch_Init(OV7670_arch* arch);
static int Try_Read_Registers(); // Try read registers of camera, return 1 if successful, 0 on error

static void debug_framebuffer(char* buf, uint32_t size)
{
    USART_Print("Framebuffer debug:\r\n");
    for (int i = 0; i < size; i++) {
        USART_PrintHex(buf[i]);
        USART_Print(" ");
    }
    USART_Print("\r\n");
}

static void Send_Framebuffer_Data(uint8_t* framebuffer, uint32_t size)
{
    USART_Print("RDY\r\n");
    USART_WriteData(framebuffer, size);
}

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
    host.platform = &host;

    if (OV7670_begin(&host, CAMERA_COLORSPACE, CAMERA_DIV_FACTOR, 1.0) != OV7670_STATUS_OK) {
        USART_Print("Failed to initialize camera!");
        while (1);
    }
    
    HAL_DelayMs(300);
    USART_Print("HELLO\r\n");
    while (!Try_Read_Registers()) {
        HAL_DelayMs(1000);
    }
    OV7670_test_pattern(&host, OV7670_TEST_PATTERN_COLOR_BAR);
    USART_Print("PAD_CONFIG->PORT_0_CFG = ");
    USART_PrintInt(PAD_CONFIG->PORT_0_CFG);
    USART_Print("\r\n");
    
    uint16_t framebuffer[CAMERA_WIDTH * CAMERA_HEIGHT];
    while (1) {
        OV7670_capture((uint32_t*)framebuffer,
        CAMERA_WIDTH, CAMERA_HEIGHT,
        &vsync.gpio->STATE, 1 << vsync.pin_num,
        &href.gpio->STATE, 1 << href.pin_num);
        GPIO_2->OUTPUT ^= GPIO_PIN_7;
        //Send_Framebuffer_Data((uint8_t*)framebuffer, sizeof(framebuffer));
        debug_framebuffer((char*)framebuffer, sizeof(framebuffer));
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
    __HAL_PCC_GPIO_0_CLK_ENABLE();
    __HAL_PCC_GPIO_1_CLK_ENABLE();
    __HAL_PCC_GPIO_2_CLK_ENABLE();
    __HAL_PCC_GPIO_IRQ_CLK_ENABLE();

    // LED pin
    GPIO_2->DIRECTION_OUT = (1 << 7);
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

static int Try_Read_Registers()
{
    /* Debug prints */
    uint8_t pid = MIK32_OV7670_read_register(OV7670_REG_PID); // Should be 0x76
    uint8_t ver = MIK32_OV7670_read_register(OV7670_REG_VER); // Should  be 0x73

    if (pid == 0x76 && ver == 0x73)
        return 1;

    USART_Print("Failed to read registers (expected 0x76 and 0x73)\r\nPID: ");
    USART_PrintInt(pid);
    USART_Print(", VER: ");
    USART_PrintInt(ver);
    USART_Print("\r\n");

    return 0;
}

static void Arch_Init(OV7670_arch* arch)
{
}
