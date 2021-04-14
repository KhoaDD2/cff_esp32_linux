#include <stdlib.h>

#include <errorcode.h>
#include <logfile.h>

#include <uart_dev_linux.hpp>

using namespace KhoaDD2Cff::Esp32Platform;

int main()
{
    int r=EXIT_SUCCESS;
    ErrorCode_t ret_uart=RETURN_SUCCESS;
    UartDevLinux *p_uart_linux=NULL;
    char byte_read;
    
    p_uart_linux = UartDevLinux::get_ins();

    if(p_uart_linux==NULL)
    {
        ret_uart= RETURN_ERROR;
    }

    if(ret_uart==RETURN_SUCCESS)
    {
        ret_uart = p_uart_linux->init();
    }

    if(ret_uart==RETURN_SUCCESS)
    {
        p_uart_linux->close();
    }

    // Final step
    if(ret_uart!=RETURN_SUCCESS)
    {
        r=EXIT_FAILURE;
    }
    return r;
}