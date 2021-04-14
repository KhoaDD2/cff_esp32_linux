/* HTTPText.cpp */
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
#define __HTTPTEXT_IN_C__
#include <stdlib.h>
#include "HTTPTextIn.h"

#include <logfile.h>

#define OK 0


#define MIN(x,y) (((x)<(y))?(x):(y))

static char* m_str = NULL;
static size_t m_size = 0;
static size_t m_pos = 0;
static bool _is_malloc = false;

void HTTPTextIn__init(char* str)
{
    m_str = str;
    if (str == NULL)
    {
        m_size = 0;
    }
    else
    {
        m_size = strlen(str);
    }
}

static size_t HTTPTextIn__getDataLen() //For Content-Length header
{
    return m_size;
}

//IHTTPDataOut
static void HTTPTextIn__writeReset()
{
    m_pos = 0;
}

static int HTTPTextIn__write(const char* buf, size_t len)
{
    if (m_str)
    {
        size_t writeLen = MIN(len, m_size - m_pos);
        memcpy(m_str + m_pos, buf, writeLen);
        m_pos += writeLen;
        m_str[m_pos] = '\0';
    }
    return OK;
}

static void HTTPTextIn__setDataType(const char* type) //Internet media type from Content-Type header
{

}

static void HTTPTextIn__setIsChunked(bool chunked) //From Transfer-Encoding header
{

}

static void HTTPTextIn__setDataLen(size_t len) //From Content-Length header, or if the transfer is chunked, next chunk length
{
    if (!m_str)
    {
        _is_malloc = true;
        m_str = (char*)malloc(len + 1 + 64);
//        if(m_str==NULL)
//          while(1);
        m_size = len;
    }
}

static void HTTPTextIn__free()
{
    if (m_str && _is_malloc)
    {
        free(m_str);
    }
    m_str = NULL;
    _is_malloc = false;
    m_size = 0;
}

static void HTTPPTextIn_Nop(void)
{

}


char* HTTPTextIn_GetData(void)
{
    if (m_str)
    {
        m_str[m_size] = '\0';
    }
    return m_str;
}

IHTTPDataIn gHTTPDataIn =
{
    .getDataLen = HTTPTextIn__getDataLen,

  /** Reset stream to its beginning
   * Called by the HTTPClient on each new request
   */
    .writeReset = HTTPTextIn__writeReset,

  /** Write a piece of data transmitted by the server
   * @param buf Pointer to the buffer from which to copy the data
   * @param len Length of the buffer
   */
    .write = HTTPTextIn__write,

  /** Set MIME type
   * @param type Internet media type from Content-Type header
   */
    .setDataType = HTTPTextIn__setDataType,

  /** Determine whether the data is chunked
   *  Recovered from Transfer-Encoding header
   */
    .setIsChunked = HTTPTextIn__setIsChunked,

  /** If the data is not chunked, set its size
   * From Content-Length header
   */
    .setDataLen = HTTPTextIn__setDataLen,

    .close = HTTPPTextIn_Nop,
    .free = HTTPTextIn__free,

};


