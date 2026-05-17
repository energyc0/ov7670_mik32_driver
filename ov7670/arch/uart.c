#include "uart.h"
#include "mik32_hal_timer16.h"
#include "mik32_hal_usart.h"

static USART_HandleTypeDef husart0;

static int32_t printIntStr(char* buf, int64_t num)
{
    int32_t size = 0;
    int32_t start = 0;
    if (num < 0) {
        buf[size++] = '-';
        start = 1;
    } else if (num == 0) {
        buf[size++] = '0';
    }

    while (num != 0) {
        buf[size++] = '0' + (num > 0 ? (num % 10) : -(num % 10));
        num /= 10;
    }

    buf[size] = '\0';
    for (int32_t end = size - 1; start < end; start++, end--) {
        char temp = buf[start];
        buf[start] = buf[end];
        buf[end] = temp;
    }

    return size;
}

void USART_Init()
{
    husart0.Instance = UART_0;
    husart0.transmitting = Enable;
    husart0.receiving = Enable;
    husart0.frame = Frame_8bit;
    husart0.parity_bit = Disable;
    husart0.parity_bit_inversion = Disable;
    husart0.bit_direction = LSB_First;
    husart0.data_inversion = Disable;
    husart0.tx_inversion = Disable;
    husart0.rx_inversion = Disable;
    husart0.swap = Disable;
    husart0.lbm = Disable;
    husart0.stop_bit = StopBit_1;
    husart0.mode = Asynchronous_Mode;
    husart0.xck_mode = XCK_Mode3;
    husart0.last_byte_clock = Disable;
    husart0.overwrite = Disable;
    husart0.rts_mode = AlwaysEnable_mode;
    husart0.dma_tx_request = Disable;
    husart0.dma_rx_request = Disable;
    husart0.channel_mode = Duplex_Mode;
    husart0.tx_break_mode = Disable;
    husart0.Interrupt.ctsie = Disable;
    husart0.Interrupt.eie = Disable;
    husart0.Interrupt.idleie = Disable;
    husart0.Interrupt.lbdie = Disable;
    husart0.Interrupt.peie = Disable;
    husart0.Interrupt.rxneie = Disable;
    husart0.Interrupt.tcie = Disable;
    husart0.Interrupt.txeie = Disable;
    husart0.Modem.rts = Disable; // out
    husart0.Modem.cts = Disable; // in
    husart0.Modem.dtr = Disable; // out
    husart0.Modem.dcd = Disable; // in
    husart0.Modem.dsr = Disable; // in
    husart0.Modem.ri = Disable; // in
    husart0.Modem.ddis = Disable; // out
    husart0.baudrate = SERIAL_BAUDRATE;
    HAL_USART_Init(&husart0);
}

void USART_Print(char* str)
{
    HAL_USART_Print(&husart0, str, USART_TIMEOUT_DEFAULT);
}

void USART_PrintInt(int64_t val)
{
    char buf[16];
    printIntStr(buf, val);
    USART_Print(buf);
}

void USART_WriteData(char* buf, uint32_t count)
{
    HAL_USART_Write(&husart0, buf, count, USART_TIMEOUT_DEFAULT);
}
