#ifndef __UART_DEF_HPP
#define __UART_DEF_HPP

#include <stdint.h>
#include <string>

#include <errorcode.h>

namespace KhoaDD2Cff
{
    namespace Esp32Platform
    {
        enum Baudrate : int 
        {
            UART_DEV_BAUDRATE_9600,
            UART_DEV_BAUDRATE_19200,
            UART_DEV_BAUDRATE_38400,
            UART_DEV_BAUDRATE_57600,
            UART_DEV_BAUDRATE_115200,
            UART_DEV_BAUDRATE_921600,
        };
        class UartDev
        {
            private:
                Baudrate baudrate;
            public:
                UartDev();
                virtual ~UartDev();
            public:
                /**
                 * @brief Init uart module, open usb CDC port.
                 * @return 0 for success, other is error code.
                 */
                virtual ErrorCode_t init();
                /**
                 * @brief Close usb CDC port.
                 * @return 0 for success, other is error code
                 */
                virtual ErrorCode_t close();
                /**
                 * @brief Change baudrate.
                 * @param baudRate he baud rate to be set for the serial port.
                 * @return 0 for success, other is error code
                 */
                virtual ErrorCode_t change_baudrate(Esp32Platform::Baudrate);
                /**
                 * @brief Send 1 byte via uart
                 * @param d data to send
                 * @return 0 for success, other is error code
                 */
                virtual ErrorCode_t send_byte(char d);
                 /**
                 * @brief Send bytes via uart
                 * @param d data to send in string
                 * @return 0 for success, other is error code
                 */
                virtual ErrorCode_t send_bytes(std::string d);
                /**
                 * @brief Read 1 byte
                 * @param d ref to char variable to get data
                 * @return 0 for success, other is error code
                 */
                virtual ErrorCode_t get_byte(char &d);
                /**
                 * @brief Read bytes
                 * @param d ref to string variable to get data
                 * @return 0 for success, other is error code
                 */
                virtual ErrorCode_t get_bytes(std::string &d, int l);
        }; // class UartDev
    } // namespace Esp32Platform
} // namespace KhoaDD2Cff

#endif // __UART_DEF_LINUX_HPP