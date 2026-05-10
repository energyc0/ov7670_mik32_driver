#include "ov7670.h"

#include "gpio.h"
#include "mik32_hal.h"
#include "mik32_hal_gpio.h"
#include "mik32_hal_irq.h"

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

void pinModeOutput(int16_t pin)
{
    if (pin < sizeof(gpios) / sizeof(gpios[0])) {
        gpios[pin]->DIRECTION_OUT = (1 << pin_bits[pin]);
    }
}

void digitalWrite(int16_t pin, int16_t hi)
{
    if (pin < sizeof(gpios) / sizeof(gpios[0])) {
        if (hi) 
            gpios[pin]->SET = (1 << pin_bits[pin]);
        else
            gpios[pin]->CLEAR = (1 << pin_bits[pin]);
    }
}