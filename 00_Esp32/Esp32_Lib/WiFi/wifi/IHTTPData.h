/* IHTTPData.h */
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

#ifndef IHTTPDATA_H
#define IHTTPDATA_H

//#include "mbed_toolchain.h"
#include <string.h>
#include <stdbool.h>


///This is a simple interface for HTTP data storage (impl examples are Key/Value Pairs, File, etc...)
typedef struct _IHTTPDataOut
{
  /** Reset stream to its beginning
   * Called by the HTTPClient on each new request
   */
  void (*readReset)(void);

  /** Read a piece of data to be transmitted
   * @param[out] buf Pointer to the buffer on which to copy the data
   * @param[in] len Length of the buffer
   * @param[out] pReadLen Pointer to the variable on which the actual copied data length will be stored
   */
  int (*read)(char* buf, size_t len, size_t* pReadLen);

  /** Get MIME type
   * @param[out] type Internet media type from Content-Type header
   * @param[in] maxTypeLen is the size of the type buffer to write to
   */
  int (*getDataType)(char* type, size_t maxTypeLen); //Internet media type for Content-Type header

  /** Determine whether the HTTP client should chunk the data
   *  Used for Transfer-Encoding header
   */
  bool (*getIsChunked)(void);

  /** If the data is not chunked, get its size
   *  Used for Content-Length header
   */
  size_t (*getDataLen)(void);

  void (*free)(void);
  void* _data;

} IHTTPDataOut;

///This is a simple interface for HTTP data storage (impl examples are Key/Value Pairs, File, etc...)
typedef struct _IHTTPDataIn
{
  size_t (*getDataLen)(void);

  /** Reset stream to its beginning
   * Called by the HTTPClient on each new request
   */
  void (*writeReset)(void);

  /** Write a piece of data transmitted by the server
   * @param buf Pointer to the buffer from which to copy the data
   * @param len Length of the buffer
   */
  int (*write)(const char* buf, size_t len);

  /** Set MIME type
   * @param type Internet media type from Content-Type header
   */
  void (*setDataType)(const char* type);

  /** Determine whether the data is chunked
   *  Recovered from Transfer-Encoding header
   */
  void (*setIsChunked)(bool chunked);

  /** If the data is not chunked, set its size
   * From Content-Length header
   */
  void (*setDataLen)(size_t len);

  void (*close)(void);
  void (*free)(void);
  void* _data;
} IHTTPDataIn;

#endif
