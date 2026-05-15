#include "i2c_bitbang.h"
#include "mik32_hal_gpio.h"
#include "uart.h"
#include <stdint.h>

#define I2C_DELAY() HAL_DelayUs(5)
#define I2C_FREQUENCY (100000) //100 KHz for ov7670

#define SDA_PIN_BIT (1 << 12)
#define SDA_GPIO GPIO_1
#define SCL_PIN_BIT (1 << 13)
#define SCL_GPIO GPIO_1

#define SDA_OUTPUT()    do { SDA_GPIO->DIRECTION_OUT = SDA_PIN_BIT; } while(0)
#define SDA_INPUT()     do { SDA_GPIO->DIRECTION_IN = SDA_PIN_BIT; } while(0)
#define SDA_HIGH()      do { SDA_GPIO->SET = SDA_PIN_BIT; } while(0)
#define SDA_LOW()       do { SDA_GPIO->CLEAR = SDA_PIN_BIT; } while(0)
#define SDA_READ()      (SDA_GPIO->STATE & SDA_PIN_BIT)

#define SCL_OUTPUT()    do { SCL_GPIO->DIRECTION_OUT = SCL_PIN_BIT; } while(0)
#define SCL_HIGH()      do { SCL_GPIO->SET = SCL_PIN_BIT; } while(0)
#define SCL_LOW()       do { SCL_GPIO->CLEAR = SCL_PIN_BIT; } while(0)
#define SCL_READ()      (SCL_GPIO->STATE & SCL_PIN_BIT)

__attribute__((section(".ram_text"))) static inline void I2C_BitBang_WriteBit(uint8_t bit)
{
    if (bit) SDA_HIGH();
    else SDA_LOW();

    I2C_DELAY();
    SCL_HIGH();
    I2C_DELAY();
    SCL_LOW();
}

__attribute__((section(".ram_text"))) static inline uint8_t I2C_BitBang_ReadBit()
{
    uint8_t bit;

    SDA_INPUT();
    I2C_DELAY();
    SCL_HIGH();
    I2C_DELAY();

    bit = SDA_READ() > 0;
    SCL_LOW();
    SDA_OUTPUT();
    return bit;
}

__attribute__((section(".ram_text"))) static inline void I2C_BitBang_Start()
{
    SDA_OUTPUT();
    SDA_HIGH();
    SCL_HIGH();
    I2C_DELAY();
    
    SDA_LOW();
    I2C_DELAY();
    
    SCL_LOW();
    I2C_DELAY();
}

__attribute__((section(".ram_text"))) static inline void I2C_BitBang_Stop()
{
    SDA_LOW();
    I2C_DELAY();
    
    SCL_HIGH();
    I2C_DELAY();
    
    SDA_HIGH();
    I2C_DELAY();
}

__attribute__((section(".ram_text"))) uint8_t I2C_BitBang_WriteByte(uint8_t byte, uint8_t is_start, uint8_t is_stop)
{
    uint8_t ack, i;
    if (is_start) I2C_BitBang_Start();

    for (i = 0; i < 8; i++) {
        I2C_BitBang_WriteBit(byte & 0x80);
        byte <<= 1;
    }
    
    ack = I2C_BitBang_ReadBit();

    if (is_stop) I2C_BitBang_Stop();

    return ack;
}

__attribute__((section(".ram_text"))) uint8_t I2C_BitBang_ReadByte(uint8_t ack_bit, uint8_t is_stop)
{
    uint8_t byte = 0;
    
    for (uint8_t i = 0; i < 8; i++) {
        byte <<= 1;
        byte |= I2C_BitBang_ReadBit();
    }

    I2C_BitBang_WriteBit(ack_bit);
    if (is_stop) I2C_BitBang_Stop();

    return byte;
}

void I2C_BitBang_Init()
{
    SDA_OUTPUT();
    SDA_HIGH();
    SCL_OUTPUT();
    SCL_HIGH();
    I2C_DELAY();
    USART_Print("Bit-Bang I2C Bit-Bang init OK.\r\n");
}

