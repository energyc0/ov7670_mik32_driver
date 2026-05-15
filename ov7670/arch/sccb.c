#include "sccb.h"
#include "ov7670.h"
#include "i2c_bitbang.h"

#define OV7670_SLAVE_ADDR (0x21)
#define OV7670_WRITE_ADDR (0x42)
#define OV7670_READ_ADDR (0x43)

void SCCB_Init()
{
    I2C_BitBang_Init();
}

int SCCB_WriteReg(uint8_t reg_addr, uint8_t reg_data)
{
    I2C_BitBang_WriteByte(OV7670_WRITE_ADDR, 1, 0);
    I2C_BitBang_WriteByte(reg_addr, 0, 0);
    I2C_BitBang_WriteByte(reg_data, 0, 1);

    return 0;
}


int SCCB_ReadReg(uint8_t reg_addr, uint8_t *reg_data)
{
    I2C_BitBang_WriteByte(OV7670_WRITE_ADDR, 1, 0);
    I2C_BitBang_WriteByte(reg_addr, 0, 1);

    I2C_BitBang_WriteByte(OV7670_READ_ADDR, 1, 0);
    *reg_data = I2C_BitBang_ReadByte(0, 1);

    return 0;
}