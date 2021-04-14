#ifndef _UART_DRV_H
#define _UART_DRV_H
#include <stdint.h>

uint32_t uart_get_char(uint8_t * bChar);
void uart_hw_init(void);
void uart_put_bytes(const uint8_t * p_src, uint32_t iLen);
void uart_drv_buffer_clear(void);
uint8_t uart_peek_char(uint8_t *);
/*
* @author Dat Le
* @date 2019-08-26
* @brief Reinit UART HW baudrate
* @param new baud rate
* @retval none
*/
void uart_hw_reinit_baud(uint32_t baud);


#endif /*_UART_DRV_H*/
