#ifndef __UART_DEF_LINUX_HPP
#define __UART_DEF_LINUX_HPP

#include <libserial/SerialPort.h>

#include <errorcode.h>

#include <uart_dev.hpp>

namespace KhoaDD2Cff
{
    namespace Esp32Platform
    {
        using namespace LibSerial;

        constexpr const char* const SERIAL_PORT_1 = "/dev/ttyUSB0" ;
        constexpr const char* const SERIAL_PORT_2 = "/dev/ttyUSB1" ;
        
        class UartDevLinux : public UartDev
        {
            private:
                UartDevLinux();
                /**
                 * @brief Convert Esp32Platform's baudrate variable to LibSerial's baudrate
                 * @return baudrate in LibSerial's enum
                 */
                LibSerial::BaudRate convert_baudrate(Esp32Platform::Baudrate);

            public:
                ~UartDevLinux();
                /**
                 * @brief Init uart module, open usb CDC port.
                 * @return 0 for success, other is error code.
                 */
                ErrorCode_t init();
                /**
                 * @brief Close usb CDC port.
                 * @return 0 for success, other is error code
                 */
                ErrorCode_t close();
                /**
                 * @brief Change baudrate.
                 * @param baudRate he baud rate to be set for the serial port.
                 * @return 0 for success, other is error code
                 */
                ErrorCode_t change_baudrate(Esp32Platform::Baudrate);
                /**
                 * @brief Send 1 byte via uart
                 * @param d data to send
                 * @return 0 for success, other is error code
                 */
                ErrorCode_t send_byte(char d);
                 /**
                 * @brief Send bytes via uart
                 * @param d data to send in string
                 * @return 0 for success, other is error code
                 */
                ErrorCode_t send_bytes(std::string d);
                /**
                 * @brief Read 1 byte
                 * @param d ref to char variable to get data
                 * @return 0 for success, other is error code
                 */
                ErrorCode_t get_byte(char &d);
                /**
                 * @brief Read bytes
                 * @param d ref to string variable to get data
                 * @return 0 for success, other is error code
                 */
                ErrorCode_t get_bytes(std::string &d, int l);

            public:
                /**
                 * @brief Singleton
                 * @return point to ins
                 */
                static UartDevLinux * get_ins();

            private:
                static UartDevLinux * p_ins;
                Baudrate baudrate;
                SerialPort serial_port;
        }; // class UartDevLinux
    } // namespace Esp32Platform
} // namespace KhoaDD2Cff

#endif // __UART_DEF_LINUX_HPP