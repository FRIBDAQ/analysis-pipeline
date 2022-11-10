/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Giordano Cerriza
	     FRIB
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** @file:  DataReader.cpp
 *  @brief: Implement the CDataReader class.
 */
#include "DataReader.h"
#include <unistd.h>
#include <stdexcept>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <string>
#include <sstream>
#include <cstdlib>
#include <new>
namespace frib {
    namespace analysis {
        /**
         * constructor
         *    @param pFilename  - the name of a file that's oppened readonly.
         *    @parm bufferSize  - number of bytes of buffer.
         * The file is opened, the book keeping variables are initialized and
         * then the buffer is filled via fillBufffer.
         */
        CDataReader::CDataReader(const char* pFilename, size_t bufferSize) :
            m_nBytes(0), m_pBuffer(nullptr), m_nBufferSize(bufferSize),
            m_eof(false),m_nFd(-1), m_fReleased(true)
        {
            // Open the file, on success, set m_nFd, allocate and fill the buffer
            // on failure throw std::runtime_error:
            
            m_nFd = open(pFilename, O_RDONLY);
            if (m_nFd < 0) {
                std::string failureReason = strerror(errno);
                std::stringstream errorStream;
                errorStream << "Failed to open: " << pFilename << " for read: "
                    << failureReason;
                std::string errormsg = errorStream.str();
                throw std::runtime_error(errormsg);
            }
            // The file is open:
            
            allocateBuffer();
            fillBuffer();
        }
        /**
         * constructor
         *    Already have a file descriptor open:
         *  @param fd -  a file descriptor open on the data source.
         *  @note fd will be closed on destruction.
         */
        CDataReader::CDataReader(int fd, std::size_t bufferSize) :
            m_nBytes(0), m_pBuffer(nullptr), m_nBufferSize(bufferSize),
            m_eof(false),m_nFd(fd), m_fReleased(true)
        {
            allocateBuffer();
            fillBuffer();
        }
        /**
         * destructor
         */
        CDataReader::~CDataReader() {
            close(m_nFd);
            std::free(m_pBuffer);
        }
        /**
         * getBlock
         *   - Ensure this is legal (m_fReleased is true).
         *   -probeData to figure out how much data to actually give.
         *   - Build up the Result value from the state data
         *     probeData created and return.
         */
        CDataReader::Result
        CDataReader::getBlock(std::size_t maxbytes)
        {
            if (!m_fReleased) {
                throw std::logic_error("Attemped read without releasing prior data");
            }
            probeData(maxbytes);
            
            Result res = {
                .s_nbytes = m_nUserBytes,
                .s_nItems = m_nUserItems,
                .s_pData  = m_nUserBytes > 0 ?  m_pBuffer : nullptr
            };
            m_fReleased = true;
            return res;
        
        }
        /**
         * done
         *    Indicate the client is done with the last chunk of data:
         *    -  Ensure this is a valid call (m_fReleased is false).
         *    - Slide any data in the buffer to the front.
         *    - Fill any remaining space
         */
        void
        CDataReader::done()
        {
            if (m_fReleased) {
                throw std::logic_error("Releasing but already released");
            }
            std::uint8_t* pfront = static_cast<std::uint8_t*>(m_pBuffer);
            memmove(pfront, pfront + m_nUserBytes, m_nUserBytes);
            m_nBytes -= m_nUserBytes;
            m_fReleased = true;
            fillBuffer();                  // Read ahead more.
        }
        //////////////////////////////////////////////////////////////////////////
        // Private utilities:
        
        /**
         *  allocateBuffer:
         *     Attempt t allocated a buffer of m_nBufferSize bytes.
         *     if that fails throw std::bad_alloc.
         */
        void
        CDataReader::allocateBuffer() {
            m_pBuffer = malloc(m_nBufferSize);
            if(!m_pBuffer) {
                throw std::bad_alloc();
            }
        }
        /**
         * fillBuffer:
         *    - Figure out how much data space is still free in the
         *      m_pBuffer (m_nBufferSize - m_nBytes).
         *    - Read that into m_pBuffer + m_nBBytes.
         *    - If read gives a zero - m_eof => true... no more reads.
         */
        void
        CDataReader::fillBuffer() {
            if (!m_eof) {
                size_t nFree = m_nBufferSize - m_nBytes;
                std::uint8_t* p = reinterpret_cast<std::uint8_t*>(m_pBuffer);
                auto n = read(m_nFd, p+m_nBytes, nFree);
                
                if (n < 0) {
                    throw std::runtime_error("Read failed in CDtaReader");
                }
                if (n == 0) {
                    m_eof = true;
                    return;
                }
                m_nBytes += n;
            }
        }
        /**
         * probeData
         *   Set up m_nUserBytes, m_nUserItems so that from m_pBuffer
         *   + m_nUserBytes, there's at least one item, and no more
         *   than maxbytes or m_nBytes.
         *
         *   @param maxBytes - maximum number of byes the caller can deal with./
         */
        void
        CDataReader::probeData(std::size_t maxBytes) {
            m_nUserBytes = 0;
            m_nUserItems = 0;
            
            // probe the smaller of maxBytes and m_nBytes;
            
            if (maxBytes > m_nBytes) maxBytes = m_nBytes;
            if (maxBytes == 0 && m_eof) {
                return;
            }
            union {
                std::uint8_t* pBytes;
                std::uint32_t* pLong;
            } p;
            p.pBytes = reinterpret_cast<std::uint8_t*>(m_pBuffer);
            
            while (m_nUserBytes <= maxBytes) {
                std::uint32_t size = *(p.pLong);
                if (size > m_nBytes) {
                    throw std::logic_error("Internal buffer overflowed by a single ring item");
                }
                m_nUserBytes += size;
                m_nUserItems++;
                p.pBytes += size;
            }
        }
    }       
}