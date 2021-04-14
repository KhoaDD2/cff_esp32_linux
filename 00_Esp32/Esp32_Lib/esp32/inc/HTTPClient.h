/* HTTPClient.h */
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

/** \file
HTTP Client header file
*/

#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H
#include <stdint.h>
#include <stdbool.h>

//#include "TCPSocketConnection.h"
//#include "TCPSocket.h"
//#include "WifiClient.h"
//#include "WiFi101.h"
#include "WifiClient.h"

#define HTTP_CLIENT_DEFAULT_TIMEOUT 40000

#include "IHTTPData.h"


///HTTP client results
typedef enum _HTTPResult
{
    HTTP_OK = 0,        ///<Success
    HTTP_PROCESSING,    ///<Processing
    HTTP_PARSE,         ///<url Parse error
    HTTP_DNS,           ///<Could not resolve name
    HTTP_PRTCL,         ///<Protocol error
    HTTP_NOTFOUND,      ///<HTTP 404 Error
    HTTP_REFUSED,       ///<HTTP 403 Error
    HTTP_ERROR,         ///<HTTP xxx error
    HTTP_TIMEOUT,       ///<Connection timeout
    HTTP_CONN,          ///<Connection error
    HTTP_CLOSED,        ///<Connection was closed by remote host
} HTTPResult;

typedef enum _HTTP_METH {
    HTTP_GET,
    HTTP_POST,
    HTTP_PUT,
    HTTP_DELETE,
    HTTP_HEAD
} HTTP_METH;

#ifndef HOSTNAME_MAX_SIZE
#define HOSTNAME_MAX_SIZE                                   64
#endif

#define Void_WifiClient         void



/**A simple HTTP Client
The HTTPClient is composed of:
- The actual client (HTTPClient)
- Classes that act as a data repository, each of which deriving from the HTTPData class (HTTPText for short text content, HTTPFile for file I/O, HTTPMap for key/value pairs, and HTTPStream for streaming purposes)
*/
typedef struct _HTTPClient
{
    void* dummy;
} HTTPClient;

#ifdef __cplusplus
extern "C" {
#endif

///Instantiate the HTTP client
void HTTPClient__init(void);


/// Get the text form of the error number
///
/// Gets a pointer to a text message that described the result code.
///
/// @param[in] res is the HTTPResult code to translate to text.
/// @returns a pointer to a text message.
///
const char * HTTPClient__GetErrorMessage(HTTPResult res);

/**
Provides a basic authentification feature (Base64 encoded username and password)
Pass two NULL pointers to switch back to no authentication
@param[in] user username to use for authentication, must remain valid durlng the whole HTTP session
@param[in] user password to use for authentication, must remain valid durlng the whole HTTP session
*/
void HTTPClient__basicAuth(const char* user, const char* password); //Basic Authentification

/**
Set custom headers for request.

Pass NULL, 0 to turn off custom headers.

@code
    const char * hdrs[] =
        {
        "Connection", "keep-alive",
        "Accept", "text/html",
        "User-Agent", "Mozilla/5.0 (Windows NT 6.1; WOW64)",
        "Accept-Encoding", "gzip,deflate,sdch",
        "Accept-Language", "en-US,en;q=0.8",
        };

    http.basicAuth("username", "password");
    http.customHeaders(hdrs, 5);
@endcode

@param[in] headers an array (size multiple of two) key-value pairs, must remain valid during the whole HTTP session
@param[in] pairs number of key-value pairs
*/
void HTTPClient__customHeaders(const char** headers, size_t pairs);

//High Level setup functions
/** Execute a GET request on the URL
Blocks until completion
@param[in] url : url on which to execute the request
@param[in,out] pDataIn : pointer to an IHTTPDataIn instance that will collect the data returned by the request, can be NULL
@param[in] timeout waiting timeout in ms (osWaitForever for blocking function, not recommended)
@return 0 on success, HTTP error (<0) on failure
*/
HTTPResult HTTPClient__get(const char* url, IHTTPDataIn* pDataIn, int timeout); //Blocking

/** Execute a GET request on the URL
Blocks until completion
This is a helper to directly get a piece of text from a HTTP result
@param[in] url : url on which to execute the request
@param[out] result : pointer to a char array in which the result will be stored
@param[in] maxResultLen : length of the char array (including space for the NULL-terminating char)
@param[in] timeout waiting timeout in ms (osWaitForever for blocking function, not recommended)
@return 0 on success, HTTP error (<0) on failure
*/
HTTPResult HTTPClient__get_raw(const char* url, char** pp_result, size_t* p_maxResultLen, int timeout); //Blocking

/** Execute a POST request on the URL
Blocks until completion
@param[in] url : url on which to execute the request
@param[out] dataOut : a IHTTPDataOut instance that contains the data that will be posted
@param[in,out] pDataIn : pointer to an IHTTPDataIn instance that will collect the data returned by the request, can be NULL
@param[in] timeout waiting timeout in ms (osWaitForever for blocking function, not recommended)
@return 0 on success, HTTP error (<0) on failure
*/
HTTPResult HTTPClient__post(const char* url, const IHTTPDataOut* p_dataOut, IHTTPDataIn* pDataIn, int timeout); //Blocking

HTTPResult HTTPClient__post_raw(const char* path, char* content, size_t content_size, char** pp_recv_content, size_t* p_recv_len);
HTTPResult HTTPClient__post_text(const char* path, char* content, size_t content_size, char** pp_recv_content, size_t* p_recv_len);

/** Execute a PUT request on the URL
Blocks until completion
@param[in] url : url on which to execute the request
@param[in] dataOut : a IHTTPDataOut instance that contains the data that will be put
@param[in,out] pDataIn : pointer to an IHTTPDataIn instance that will collect the data returned by the request, can be NULL
@param[in] timeout waiting timeout in ms (osWaitForever for blocking function, not recommended)
@return 0 on success, HTTP error (<0) on failure
*/
HTTPResult HTTPClient__put(const char* url, const IHTTPDataOut* pdataOut, IHTTPDataIn* pDataIn, int timeout); //Blocking
HTTPResult HTTPClient__put_raw(const char* url, char* content, size_t content_size, char** p_recv_content, size_t* p_recv_len);

/** Execute a DELETE request on the URL
Blocks until completion
@param[in] url : url on which to execute the request
@param[in,out] pDataIn : pointer to an IHTTPDataIn instance that will collect the data returned by the request, can be NULL
@param[in] timeout waiting timeout in ms (osWaitForever for blocking function, not recommended)
@return 0 on success, HTTP error (<0) on failure
*/
HTTPResult HTTPClient__del(const char* url, IHTTPDataIn* pDataIn, int timeout); //Blocking

/** Get last request's HTTP response code
@return The HTTP response code of the last request
*/
int HTTPClient__getHTTPResponseCode(void);

/** Set the maximum number of automated redirections
@param[in] i is the number of redirections. Values < 1 are
    set to 1.
*/
void HTTPClient__setMaxRedirections(int i);

/** get the redirect location url
@returns const char pointer to the url.
*/
const char * HTTPClient__getLocation();

HTTPResult HTTPClient__connect(const char* url, HTTP_METH method, const IHTTPDataOut* pDataOut, IHTTPDataIn* pDataIn, int timeout); //Execute request
HTTPResult HTTPClient__recv(char* buf, size_t minLen, size_t maxLen, size_t* pReadLen); //0 on success, err code on failure
HTTPResult HTTPClient__send(const char* buf, size_t len); //0 on success, err code on failure
HTTPResult HTTPClient__parseURL(const char* url, char* scheme, size_t maxSchemeLen, char* host, size_t maxHostLen, uint16_t* port, char* path, size_t maxPathLen); //Parse URL
void HTTPClient__createauth (const char *user, const char *pwd, char *buf, int len);
int HTTPClient__base64enc(const char *input, unsigned int length, char *output, int len);

#ifdef __cplusplus
}
#endif


//Including data containers here for more convenience
//#include "data/HTTPText.h"
//#include "data/HTTPMap.h"
//#include "data/HTTPFile.h"
//#include "data/HTTPJson.h"
//#include "data/HTTPRaw.h"

#include "IHTTPData.h"
#include "data/HTTPTextIn.h"
#include "data/HTTPTextOut.h"
#include "data/HTTPRawOut.h"
#include "data/HTTPHexDumpIn.h"
#endif
