#include <string.h>

#include <logfile.h>

#include <uart_dev.hpp>
#include <uart_dev_linux.hpp>

#include <uart_dev_wrapper.h>

using namespace KhoaDD2Cff::Esp32Platform;

static UartDev * p_uart_dev = NULL;

/**
 * @brief Init usb CDC port.
 * @return 0 for success, other is error code
 */
ErrorCode_t __uart_dev__init()
{
    ErrorCode_t r=RETURN_SUCCESS;
    p_uart_dev = UartDevLinux::get_ins();

    if(p_uart_dev==NULL)
    {
        r=RETURN_BAD_REF;
    }

    if(r==RETURN_SUCCESS)
    {
        r=p_uart_dev->init();
    }

    return r;
};

/**
 * @brief Close usb CDC port.
 * @return 0 for success, other is error code
 */
ErrorCode_t __uart_dev__close()
{
    ErrorCode_t r=RETURN_SUCCESS;

    if(p_uart_dev==NULL)
    {
        r=RETURN_BAD_REF;
    }

    if(r==RETURN_SUCCESS)
    {
        r=p_uart_dev->close();
    }
    return r;
};

/**
 * @brief Send 1 byte via uart
 * @param d data to send
 * @return 0 for success, other is error code
 */
ErrorCode_t __uart_dev__send_byte(char d)
{
    ErrorCode_t r=RETURN_SUCCESS;

    if(p_uart_dev==NULL)
    {
        r=RETURN_BAD_REF;
    }

    r=p_uart_dev->send_byte(d);

    return r;
};

/**
 * @brief Send bytes via uart
 * @param d data to send in string
 * @return 0 for success, other is error code
 */
ErrorCode_t __uart_dev__send_bytes(const char *d, int l)
{
    ErrorCode_t r=RETURN_SUCCESS;
    
    if(p_uart_dev==NULL)
    {
        r=RETURN_BAD_REF;
    }
    
    if(d==NULL)
    {
        r=RETURN_NULL_PTR;
    }

    if(r==RETURN_SUCCESS)
    {
        r=p_uart_dev->send_bytes(std::string(d,l));
    }

    return r;
};

/**
 * @brief Read 1 byte
 * @param d pointer to char variable to send data
 * @param l length of data to send
 * @return 0 for success, other is error code
 */
ErrorCode_t __uart_dev__get_byte(char *d)
{
    ErrorCode_t r=RETURN_SUCCESS;

    if(p_uart_dev==NULL)
    {
        r=RETURN_BAD_REF;
    }

    if(d==NULL)
    {
        r=RETURN_NULL_PTR;
    }

    if(r==RETURN_SUCCESS)
    {
        r=p_uart_dev->get_byte(*d);
    }
    
    return r;
};

/**
 * @brief Read 1 byte
 * @param d pointer to char variable to send data
 * @param l length of data to send
 * @return 0 for success, other is error code
 */
ErrorCode_t __uart_dev__peek_byte(char *d)
{
    ErrorCode_t r=RETURN_SUCCESS;

    if(p_uart_dev==NULL)
    {
        r=RETURN_BAD_REF;
    }

    return r;
};

/**
 * @brief Read bytes
 * @param d ref to string variable to get data
 * @param max_l max input's length
 * @param l read length expected
 * @return 0 for success, other is error code
 */
ErrorCode_t __uart_dev__get_bytes(char *d, int max_l, int l)
{
    ErrorCode_t r=RETURN_SUCCESS;
    std::string tmp_string;
    
    if(p_uart_dev==NULL)
    {
        r=RETURN_BAD_REF;
    
    }

    if(r==RETURN_SUCCESS )
    {
        r=p_uart_dev->get_bytes(tmp_string,l);
    }

    if(r==RETURN_SUCCESS && tmp_string.size()>max_l)
    {
        LOGE("Buffer in is too small");
        r=RETURN_ERROR;
    }
    return r;
};