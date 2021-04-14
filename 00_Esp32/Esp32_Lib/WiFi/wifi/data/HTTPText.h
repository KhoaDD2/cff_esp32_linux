/* HTTPText.h */
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


#ifndef HTTPTEXT_H_
#define HTTPTEXT_H_

#include "../IHTTPData.h"
#include "HTTPTextIn.h"
#include "HTTPTextOut.h"

#if 0
/** A data endpoint to store text
*/
class HTTPText : public IHTTPDataIn, public IHTTPDataOut
{
public:
    /** Create an HTTPText instance for output
     * @param[in] str String to be transmitted
     */
    HTTPText(char* str = NULL);

    /** Create an HTTPText instance for input
     * @param[in] str Buffer to store the incoming string
     * @param[in] size Size of the buffer
     */
    HTTPText(char* str, size_t size);

    virtual size_t getDataLen(); //For Content-Length header

protected:
    friend class HTTPClient;

    //IHTTPDataIn
    virtual void readReset();

    virtual int read(char* buf, size_t len, size_t* pReadLen);

    virtual int getDataType(char* type, size_t maxTypeLen); //Internet media type for Content-Type header

    virtual bool getIsChunked(); //For Transfer-Encoding header

    //IHTTPDataOut
    virtual void writeReset();

    virtual int write(const char* buf, size_t len);

    virtual void setDataType(const char* type); //Internet media type from Content-Type header

    virtual void setIsChunked(bool chunked); //From Transfer-Encoding header

    virtual void setDataLen(size_t len); //From Content-Length header, or if the transfer is chunked, next chunk length

    virtual operator char*()
    {
        if (m_str)
            {
                m_str[m_size] = '\0';
            }
        return m_str;
    };
private:
    char* m_str;
    size_t m_size;

    size_t m_pos;
};
#endif
#endif /* HTTPTEXT_H_ */
