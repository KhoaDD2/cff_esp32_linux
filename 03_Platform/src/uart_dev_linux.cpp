#include <libserial/SerialPort.h>

#include <logfile.h>
#include <uart_dev_linux.hpp>

using namespace KhoaDD2Cff::Esp32Platform;
using namespace LibSerial;

UartDevLinux * UartDevLinux::p_ins = nullptr;

UartDevLinux::UartDevLinux(): baudrate(UART_DEV_BAUDRATE_115200)
{

};

UartDevLinux::~UartDevLinux()
{

};

ErrorCode_t UartDevLinux::init()
{
    ErrorCode_t r=RETURN_SUCCESS;
    try
    {
        this->serial_port.Open(SERIAL_PORT_1) ;
    }
    catch (const OpenFailed& e)
    {
        LOGE("%s", e.what());
        r=RETURN_ERROR; 
    }

    if(r==RETURN_SUCCESS)
    {
        // Set the baud rate of the serial port.
        LOGD("Set baudrate at 921600");
        this->serial_port.SetBaudRate(BaudRate::BAUD_921600) ;

        // Set the number of data bits.
        this->serial_port.SetCharacterSize(CharacterSize::CHAR_SIZE_8) ;

        // Turn off hardware flow control.
        this->serial_port.SetFlowControl(FlowControl::FLOW_CONTROL_NONE) ;

        // Disable parity.
        this->serial_port.SetParity(Parity::PARITY_NONE) ;
        
        // Set the number of stop bits.
        this->serial_port.SetStopBits(StopBits::STOP_BITS_1) ;
    }

    return r;
};

ErrorCode_t UartDevLinux::close()
{
    ErrorCode_t r=RETURN_SUCCESS;
    this->serial_port.Close();
    return r;
};

ErrorCode_t UartDevLinux::change_baudrate(Baudrate baud)
{
    ErrorCode_t r=RETURN_SUCCESS;
    this->baudrate = baud;
    this->serial_port.SetBaudRate(convert_baudrate(baud)) ;
    return r;
};

ErrorCode_t UartDevLinux::send_byte(char d)
{
    ErrorCode_t r=RETURN_SUCCESS;
    this->serial_port.WriteByte(d);
    return r;
};

ErrorCode_t UartDevLinux::send_bytes(std::string d)
{
    ErrorCode_t r=RETURN_SUCCESS;
    this->serial_port.Write(d);
    return r;
};

ErrorCode_t UartDevLinux::get_byte(char &d)
{
    ErrorCode_t r=RETURN_SUCCESS;
    try
    {
        this->serial_port.ReadByte(d,1000);
    } 
    catch (ReadTimeout e)
    {
        r=RETURN_ERROR;
    }
    return r;
};

ErrorCode_t UartDevLinux::get_bytes(std::string &d, int l)
{
    ErrorCode_t r=RETURN_SUCCESS;
    try
    {
        this->serial_port.Read(d,l,1000);
    } 
    catch (ReadTimeout e)
    {
        r=RETURN_ERROR;
    }
    return r;
};

UartDevLinux * UartDevLinux::get_ins()
{
    if( UartDevLinux::p_ins == nullptr )
    {
        UartDevLinux::p_ins = new UartDevLinux();
    }
    return UartDevLinux::p_ins;
}

LibSerial::BaudRate UartDevLinux::convert_baudrate(Esp32Platform::Baudrate b)
{
    LibSerial::BaudRate r=BaudRate::BAUD_115200;
    switch (b)
    {
    case UART_DEV_BAUDRATE_9600:
        r=BaudRate::BAUD_9600;
        break;
    case UART_DEV_BAUDRATE_19200:
        r=BaudRate::BAUD_19200;
        break;
    case UART_DEV_BAUDRATE_38400:
        r=BaudRate::BAUD_38400;
        break;
    case UART_DEV_BAUDRATE_57600:
        r=BaudRate::BAUD_57600;
        break;
    case UART_DEV_BAUDRATE_115200:
        r=BaudRate::BAUD_115200;
        break;
    case UART_DEV_BAUDRATE_921600:
        r=BaudRate::BAUD_921600;
        break;
    default:
        LOGE("Not support");
        break;
    }
};