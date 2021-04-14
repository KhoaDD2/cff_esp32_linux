#include <stdio.h>
#include <string.h>

#include <uart_drv.h>
#include <timer_drv.h>

#include <logfile.h>

#include <uart_dev_wrapper.h>

void uart_hw_init(void)
{
    LOGI("Init Uart hw");
    __uart_dev__init();
}

uint32_t uart_get_char(uint8_t * c)
{
    uint32_t r=-1;
    r=RETURN_SUCCESS==__uart_dev__get_byte((char *)c)?1:0;
    return r;
}

void uart_put_bytes(const uint8_t * p_src, uint32_t iLen)
{
    __uart_dev__send_bytes((const char *)p_src,(int)iLen);
}

void uart_drv_buffer_clear(void)
{
    
}

uint8_t uart_peek_char(uint8_t *c)
{
    uint8_t r=0;
    r=RETURN_SUCCESS==__uart_dev__peek_byte((char *)c)?1:0;
    return r;
}
