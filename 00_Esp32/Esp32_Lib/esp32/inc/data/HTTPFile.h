#ifndef HTTPFILE_H
#define HTTPFILE_H
#include <mbed.h>
#include "../IHTTPData.h"


/// A file handling mechanism - downloads files to a locally accessible file system
class HTTPFile : public IHTTPDataIn {
    
    public:
        /// Instantiate HTTPFile with a specified file on a locally accessible file system.
        ///
        /// @code
        /// HTTPFile latest("/local/status.txt");
        /// HTTPErrorCode = http.get("http://server.dom/path/serverstatus.txt", &latest);
        /// if (HTTPErrorCode == HTTP_OK) {
        ///    ... // file successfully downloaded
        /// }
        /// @endcode
        ///
        /// @param filename is the fully qualified filename to create.
        ///
        HTTPFile(char* filename);
        
        /** Closes the file, should be called once the http connection is closed.
         */
        virtual void close();
        
    protected:     
       
        friend class HTTPClient;
    
        /** Reset stream to its beginning 
        * Called by the HTTPClient on each new request
        */
        virtual void writeReset();
        
        /** Write a piece of data transmitted by the server
        * @param[in] buf Pointer to the buffer from which to copy the data
        * @param[in] len Length of the buffer
        * @returns number of bytes written.
        */
        virtual int write(const char* buf, size_t len);
        
        /** Set MIME type
        * @param[in] type Internet media type from Content-Type header
        */
        virtual void setDataType(const char* type);
        
        /** Determine whether the data is chunked
        *  Recovered from Transfer-Encoding header
        * @param[in] chunked indicates the transfer is chunked.
        */
        virtual void setIsChunked(bool chunked);
        
        /** If the data is not chunked, set its size
        * From Content-Length header
        * @param[in] len defines the size of the non-chunked transfer.
        */
        virtual void setDataLen(size_t len);
    private:
        FILE *file;
        size_t m_len;
        bool m_chunked;
};
#endif  // HTTPFILE_H