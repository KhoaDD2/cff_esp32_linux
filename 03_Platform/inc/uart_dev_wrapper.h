#ifndef __UART_DEF_H__
#define __UART_DEF_H__

#ifdef __cplusplus
extern "C" {
#endif 

#include <errorcode.h>

/**
 * @brief Init usb CDC port.
 * @return 0 for success, other is error code
 */
ErrorCode_t __uart_dev__init();
/**
 * @brief Close usb CDC port.
 * @return 0 for success, other is error code
 */
ErrorCode_t __uart_dev__close();
/**
 * @brief Send 1 byte via uart
 * @param d data to send
 * @return 0 for success, other is error code
 */
ErrorCode_t __uart_dev__send_byte(char d);
    /**
 * @brief Send bytes via uart
 * @param d data to send in string
 * @return 0 for success, other is error code
 */
ErrorCode_t __uart_dev__send_bytes(const char *d, int l);
/**
 * @brief Read 1 byte
 * @param d pointer to char variable to send data
 * @param l length of data to send
 * @return 0 for success, other is error code
 */
ErrorCode_t __uart_dev__get_byte(char *d);
/**
 * @brief Read bytes
 * @param d ref to string variable to get data
 * @param max_l max input's length
 * @param l read length expected
 * @return 0 for success, other is error code
 */
ErrorCode_t __uart_dev__get_bytes(char *d, int max_l, int l);

/**
 * @brief Peek 1 byte
 * @param d pointer to char variable to peek data
 * @return 0 for success, other is error code
 */
ErrorCode_t __uart_dev__peek_byte(char *d);

#ifdef __cplusplus
}
#endif 

#endif // __UART_DEF_H__