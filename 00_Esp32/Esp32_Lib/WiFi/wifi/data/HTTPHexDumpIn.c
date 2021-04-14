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
#include <stdint.h>
#include "HTTPHexDumpIn.h"


#define OK 0


#define MIN(x,y) (((x)<(y))?(x):(y))

#define DEBUG   "HTTPDump"
#include <stdio.h>
#if defined(DEBUG)
#define DUMP_PRINT  printf
#define DBG(x, ...)  DUMP_PRINT("[DBG %s %3d] "x"\r\n", DEBUG, __LINE__, ##__VA_ARGS__);
#define WARN(x, ...) DUMP_PRINT("[WRN %s %3d] "x"\r\n", DEBUG, __LINE__, ##__VA_ARGS__);
#define ERR(x, ...)  DUMP_PRINT("[ERR %s %3d] "x"\r\n", DEBUG, __LINE__, ##__VA_ARGS__);
#define INFO(x, ...) DUMP_PRINT("[INF %s %3d] "x"\r\n", DEBUG, __LINE__, ##__VA_ARGS__);
#else
#define DBG(x, ...)
#define WARN(x, ...)
#define ERR(x, ...)
#define INFO(x, ...)
#define DUMP_PRINT(x, ...)
#endif

static size_t m_size = 0;
static size_t m_pos = 0;

void HTTPHexDumpIn__init(void)
{
    m_size = 0;
}

static size_t HTTPHexDumpIn__getDataLen() //For Content-Length header
{
    return m_size;
}

//IHTTPDataOut
static void HTTPHexDumpIn__writeReset()
{
    m_pos = 0;
}

static int HTTPHexDumpIn__write(const char* buf, size_t len)
{
    for (uint32_t i = 0; i < len; i ++)
    {
        DUMP_PRINT("%.02X", *buf);
        m_pos++;
        buf++;
        if (m_pos % 32 == 0)
        {
            DUMP_PRINT("\r\n");
        }
    }
    return OK;
}

static void HTTPHexDumpIn__setDataType(const char* type) //Internet media type from Content-Type header
{

}

static void HTTPHexDumpIn__setIsChunked(bool chunked) //From Transfer-Encoding header
{

}

static void HTTPHexDumpIn__setDataLen(size_t len) //From Content-Length header, or if the transfer is chunked, next chunk length
{
    DUMP_PRINT("DataLen=%d\r\n", len);
}



static void HTTPPTextIn_Nop(void)
{

}

static void HTTPPTextIn_Close(void)
{
    DUMP_PRINT("Done");
}


char* HTTPHexDumpIn_GetData(void)
{
    return NULL;
}

IHTTPDataIn gHTTPHexDumpIn =
{
    .getDataLen = HTTPHexDumpIn__getDataLen,

  /** Reset stream to its beginning
   * Called by the HTTPClient on each new request
   */
    .writeReset = HTTPHexDumpIn__writeReset,

  /** Write a piece of data transmitted by the server
   * @param buf Pointer to the buffer from which to copy the data
   * @param len Length of the buffer
   */
    .write = HTTPHexDumpIn__write,

  /** Set MIME type
   * @param type Internet media type from Content-Type header
   */
    .setDataType = HTTPHexDumpIn__setDataType,

  /** Determine whether the data is chunked
   *  Recovered from Transfer-Encoding header
   */
    .setIsChunked = HTTPHexDumpIn__setIsChunked,

  /** If the data is not chunked, set its size
   * From Content-Length header
   */
    .setDataLen = HTTPHexDumpIn__setDataLen,

    .close = HTTPPTextIn_Close,
    .free = HTTPPTextIn_Nop,

};


