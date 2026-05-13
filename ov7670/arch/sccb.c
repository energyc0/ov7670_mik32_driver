#include "sccb.h"
#include "i2c.h"
#include "mik32_hal_def.h"
#include "mik32_hal_i2c.h"
#include "power_manager.h"
#include "uart.h"
#include <stdint.h>

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

#define SCCB_DELAY_HALF() HAL_DelayUs(5)
#define SCCB_DELAY() HAL_DelayUs(10)
#define SCCB_FREQUENCY (100000) //100 KHz for ov7670

#define OV7670_SLAVE_ADDR (0x21)
#define OV7670_WRITE_ADDR (0x42)
#define OV7670_READ_ADDR (0x43)

#define SDA_PIN_BIT (1 << 12)
#define SDA_GPIO GPIO_1
#define SCL_PIN_BIT (1 << 13)
#define SCL_GPIO GPIO_1

#define SDA_OUTPUT()    do { SDA_GPIO->DIRECTION_OUT = SDA_PIN_BIT; } while(0)
#define SDA_INPUT()     do { SDA_GPIO->DIRECTION_IN = SDA_PIN_BIT; } while(0)
#define SDA_HIGH()      do { SDA_GPIO->SET = SDA_PIN_BIT; } while(0)
#define SDA_LOW()         do { SDA_GPIO->CLEAR = SDA_PIN_BIT; } while(0)

#define SCL_OUTPUT()    do { SCL_GPIO->DIRECTION_OUT = SCL_PIN_BIT; } while(0)
#define SCL_HIGH()      do { SCL_GPIO->SET = SCL_PIN_BIT; } while(0)
#define SCL_LOW()         do { SCL_GPIO->CLEAR = SCL_PIN_BIT; } while(0)

I2C_HandleTypeDef hi2c;

static HAL_StatusTypeDef SCCB_WaitTXIS(I2C_HandleTypeDef* handler, uint32_t timeout);


static inline void SCCB_Start()
{
    SDA_OUTPUT();
    SDA_HIGH();
    SCCB_DELAY_HALF();
    
    SCL_HIGH();
    SCCB_DELAY_HALF();
    
    SDA_LOW();
    SCCB_DELAY_HALF();
    
    SCL_LOW();
    SCCB_DELAY_HALF();

}

static inline void SCCB_Stop()
{
    SDA_LOW();
    SCCB_DELAY_HALF();
    
    SCL_HIGH();
    SCCB_DELAY_HALF();
    
    SDA_HIGH();   // SDA: 0 -> 1 при SCL = 1
    SCCB_DELAY();
}

void SCCB_Init()
{/*
    hi2c.Instance = I2C_1;
    hi2c.Init.Mode = HAL_I2C_MODE_MASTER;

    hi2c.Init.DigitalFilter = I2C_DIGITALFILTER_OFF;
    hi2c.Init.AnalogFilter = I2C_ANALOGFILTER_DISABLE;
    hi2c.Init.AutoEnd = I2C_AUTOEND_ENABLE;
    hi2c.Init.frequency = SCCB_FREQUENCY;

    hi2c.Clock.PRESC = 5;
    hi2c.Clock.SCLDEL = 15;
    hi2c.Clock.SDADEL = 15;
    hi2c.Clock.SCLH = 15;
    hi2c.Clock.SCLL = 15;

    if (HAL_I2C_Init(&hi2c) == HAL_OK) {
        USART_Print("SCCB init OK\r\n");
    }
    */

    SDA_OUTPUT();
    SDA_HIGH();
    SCL_OUTPUT();
    SCL_HIGH();

    USART_Print("SCCB init OK\r\n");
}

int SCCB_WriteByte(uint8_t reg_addr, uint8_t reg_data)
{
    uint8_t bytes_to_transmit[3] = {OV7670_WRITE_ADDR, reg_addr, reg_data};



    /* Send 3 bytes */
    hi2c.Instance->CR2 &= ~I2C_CR2_NBYTES_M;
    hi2c.Instance->CR2 |= I2C_CR2_NBYTES(ARRAY_SIZE(bytes_to_transmit));

    /* RELOAD is enabled, AUTOEND is enabled, so sending STOP automatically */
    hi2c.Instance->CR2 &= ~I2C_CR2_RELOAD_M;
    hi2c.Instance->CR2 &= ~(1 << I2C_CR2_AUTOEND_S);

    /* Set ov7670 address */
    HAL_I2C_Master_SlaveAddress(&hi2c, OV7670_SLAVE_ADDR);
    /* We want to write */
    hi2c.Instance->CR2 &= ~I2C_CR2_RD_WRN_M;
    /* Start */
    hi2c.Instance->CR2 |= I2C_CR2_START_M;

    for (uint32_t i = 0; i < ARRAY_SIZE(bytes_to_transmit); i++)
    {
        /* Ignore NACK, because of DONT CARE bit */
        /* Set NACK flag back */
        while (!(hi2c.Instance->ISR & I2C_ISR_TXE_M));
        /* Transmit byte */
        hi2c.Instance->TXDR = bytes_to_transmit[i];
    }

    /* All the data is transmitted */
    hi2c.Instance->ISR |= (~I2C_ISR_NACKF_M);
    if (HAL_I2C_WaitBusy(&hi2c, I2C_TIMEOUT_DEFAULT) != HAL_OK) {
        return -2;
    }
    
    return 0;
    
}


int SCCB_ReadByte(uint8_t reg_addr, uint8_t *reg_data)
{
/* Phase 1: Write register address */
    uint8_t bytes_to_transmit[2] = {OV7670_WRITE_ADDR, reg_addr};
    
    hi2c.Instance->CR2 &= ~I2C_CR2_NBYTES_M;
    hi2c.Instance->CR2 |= I2C_CR2_NBYTES(ARRAY_SIZE(bytes_to_transmit));
    
    hi2c.Instance->CR2 &= ~I2C_CR2_RELOAD_M;
    hi2c.Instance->CR2 |= (1 << I2C_CR2_AUTOEND_S);
    
    /* Set ov7670 address */
    HAL_I2C_Master_SlaveAddress(&hi2c, OV7670_SLAVE_ADDR);
    hi2c.Instance->CR2 &= ~I2C_CR2_RD_WRN_M;  /* Write mode */
    hi2c.Instance->CR2 |= I2C_CR2_START_M;
    
    for (uint32_t i = 0; i < ARRAY_SIZE(bytes_to_transmit); i++)
    {
        while (!(hi2c.Instance->ISR & I2C_ISR_TXIS_M));
        
        hi2c.Instance->TXDR = bytes_to_transmit[i];
    }
    
    /* Wait for STOP */
    if (HAL_I2C_WaitBusy(&hi2c, I2C_TIMEOUT_DEFAULT) != HAL_OK)
        return -2;
    
    /* Phase 2: Read actual data */
    hi2c.Instance->CR2 &= ~I2C_CR2_NBYTES_M;
    hi2c.Instance->CR2 |= I2C_CR2_NBYTES(1);  /* Read 1 byte */
    
    hi2c.Instance->CR2 &= ~I2C_CR2_RELOAD_M;
    hi2c.Instance->CR2 |= (1 << I2C_CR2_AUTOEND_S);
    
    /* Set ov7670 address */
    HAL_I2C_Master_SlaveAddress(&hi2c, OV7670_SLAVE_ADDR);
    hi2c.Instance->CR2 |= I2C_CR2_RD_WRN_M;   /* Read mode */
    hi2c.Instance->CR2 |= I2C_CR2_START_M;
    
    /* Wait for RXNE flag */
    if (HAL_I2C_Master_WaitRXNE(&hi2c, I2C_TIMEOUT_DEFAULT) != HAL_OK)
        return -3;
    
    /* Read the data */
    *reg_data = (uint8_t)hi2c.Instance->RXDR;
    
    /* Wait for STOP */
    if (HAL_I2C_WaitBusy(&hi2c, I2C_TIMEOUT_DEFAULT) != HAL_OK)
        return -4;
    
    return 0;  /* Success */
}

static HAL_StatusTypeDef SCCB_WaitTXIS(I2C_HandleTypeDef* handler, uint32_t timeout)
{
    uint32_t status_isr = 0;
    
    /* Wait for NACK */
    while (timeout--)
    {
        status_isr = handler->Instance->ISR;
        
        if (status_isr & I2C_ISR_NACKF_M)
            handler->Instance->ISR |= I2C_ISR_NACKF_M;

        if (status_isr & (I2C_ISR_BERR_M | I2C_ISR_ARLO_M))
        {
            if (status_isr & I2C_ISR_BERR_M)
            {
                handler->ErrorCode = I2C_ERROR_BERR;
                return HAL_ERROR;
            }

            if (status_isr & I2C_ISR_ARLO_M)
            {
                handler->ErrorCode = I2C_ERROR_ARLO;
                return HAL_ERROR;
            }
        }
        if (status_isr & I2C_ISR_TXIS_M)
            return HAL_OK;
    }

    handler->ErrorCode = I2C_ERROR_TIMEOUT;
    return HAL_TIMEOUT;
}
