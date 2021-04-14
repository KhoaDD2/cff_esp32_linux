#include <errorcode.h>
#include <uart_dev.hpp>

using namespace KhoaDD2Cff::Esp32Platform;

UartDev::UartDev(): baudrate(UART_DEV_BAUDRATE_115200)
{

};

UartDev::~UartDev()
{

};

ErrorCode_t UartDev::init(){
    ErrorCode_t r=RETURN_SUCCESS;
    return r;
};
ErrorCode_t UartDev::close(){
    ErrorCode_t r=RETURN_SUCCESS;
    return r;
};
ErrorCode_t UartDev::change_baudrate(Esp32Platform::Baudrate b){
    ErrorCode_t r=RETURN_SUCCESS;
    return r;
};
ErrorCode_t UartDev::send_byte(char d){
    ErrorCode_t r=RETURN_SUCCESS;
    return r;
};
ErrorCode_t UartDev::send_bytes(std::string d){
    ErrorCode_t r=RETURN_SUCCESS;
    return r;
};
ErrorCode_t UartDev::get_byte(char &d){
    ErrorCode_t r=RETURN_SUCCESS;
    return r;
};
ErrorCode_t UartDev::get_bytes(std::string &d, int l){
    ErrorCode_t r=RETURN_SUCCESS;
    return r;
};
