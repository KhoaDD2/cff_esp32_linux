/* HTTPRaw.cpp */
/* Copyright (C) 2012 mbed.org, MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#define __HTTPRAW_OUT_C__
#include <stdlib.h>
#include <stdint.h>

#include "HTTPRawOut.h"
#define OK 0

#define MIN(x,y) (((x)<(y))?(x):(y))
static char* m_str = NULL;
static size_t m_size = 0;
static size_t m_pos = 0;
static bool _is_malloc = false;

void HTTPRawOut__init(char* str, size_t size)
{
    m_pos = 0;
    m_str = str;
    if (str == NULL)
    {
        m_size = 0;
    }
    else
    {
        m_size = size;
    }
}

//IHTTPDataIn
static void HTTPRawOut__readReset()
{
    m_pos = 0;
}

static int HTTPRawOut__read(char* buf, size_t len, size_t* pReadLen)
{
    if (m_str)
    {
        *pReadLen = MIN(len, m_size - m_pos);
        memcpy(buf, m_str + m_pos, *pReadLen);
        m_pos += *pReadLen;
    }
    return OK;
}

static bool HTTPRawOut__getIsChunked() //For Transfer-Encoding header
{
    return false;
}

static size_t HTTPRawOut__getDataLen() //For Content-Length header
{
    return m_size;
}


static void HTTPRawOut__free(void)
{
    if (m_str && _is_malloc)
    {
        free(m_str);
    }
    m_str = NULL;
    _is_malloc = false;
    m_size = 0;
}

char* HTTPRawOut__GetData(void)
{
    if (m_str)
    {
        m_str[m_size] = '\0';
    }
    return m_str;
}

static int HTTPRawOut_getDataType(char* type, size_t maxTypeLen) //Internet media type for Content-Type header
{
    strncpy(type, "application/octet-stream", maxTypeLen-1);
    type[maxTypeLen-1] = '\0';
    return OK;
}




IHTTPDataOut gHTTPRawOut =
{
  /** Reset stream to its beginning
   * Called by the HTTPClient on each new request
   */
  .readReset = HTTPRawOut__readReset,

  /** Read a piece of data to be transmitted
   * @param[out] buf Pointer to the buffer on which to copy the data
   * @param[in] len Length of the buffer
   * @param[out] pReadLen Pointer to the variable on which the actual copied data length will be stored
   */
  .read = HTTPRawOut__read,

  /** Get MIME type
   * @param[out] type Internet media type from Content-Type header
   * @param[in] maxTypeLen is the size of the type buffer to write to
   */
  .getDataType = HTTPRawOut_getDataType,

  /** Determine whether the HTTP client should chunk the data
   *  Used for Transfer-Encoding header
   */
  .getIsChunked = HTTPRawOut__getIsChunked,

  /** If the data is not chunked, get its size
   *  Used for Content-Length header
   */
  .getDataLen = HTTPRawOut__getDataLen,

  .free = HTTPRawOut__free,

};