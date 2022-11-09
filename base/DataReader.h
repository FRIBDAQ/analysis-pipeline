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

/** @file:  DataReader.h
 *  @brief: Read data from a file and present it to the caller.
 */
#ifndef DATAREADER_H
#define DATAREADER_H

#include <cstddef>

namespace frib {
    namespace analysis {
        /**
         * @class CDataReader
         *    This is a buffered data reader that is reading a set of items
         *    that are ring item like.  For the purposes of this class,
         *    we only need to know that each item starts with an std::unit32_t
         *    which represents its self inclusive size.
         *
         *    Formally we have an internal buffer into which we gulp pretty
         *    big chunks of the data source.  A client then asks us for
         *    a chunk of data giving us a maximum size.  We return
         *    a pointer to the block of data and an actual size, as well
         *    as a number of items.   Prior to the next request for data,
         *    the caller must indicate that it is done with the current block.
         *    Thus this is inherently not thread-safe though it can be used
         *    to feed data to e.g. a Dealer in an MPI environment.
         *
         *    Typical use case:
         *
         *  \verbatim
         *     CDataReader reader("somefile");
         *     auto got = reader.getBlock(maxbytes);
         *     size_t nbytes = got.s_nbytes;    // 0 if no more data.
         *     size_t nItems = got.s_nItems;    // 0 if no more data.
         *     const void* p = got.s_pData;     // data, nullptr if no more data.
         *     ...                              // do something with the data.
         *     reader.done();                   // now can get more data.
         * \endverbatim
         *
         * @note this works best (in terms of minmal data movement), if the
         *       size of the reader's buffer is closely matched to the maxsize's
         *       that are passed to getBlock().
         */
        class CDataReader {
        private:
            std::size_t m_nBytes;                   // bytes in the buffer.
            void*  m_pBuffer;                  // buffer itself.
            std::size_t m_nBufferSize;              // Size of the buffer.
            bool   m_eof;
            
            int    m_nFd;                      // Data source.
            
            // State of the last 'read':
            
            bool   m_fReleased;               // A new read is allowed.
            std::size_t m_nUserBytes;              // Bytes we gave to the user.
            std::size_t m_nUserItems;              // items we gave to the user.
            
        public:           // Exported data types:
            
            // What a read returns:
            
            typedef struct _Result {
                std::size_t s_nbytes;
                std::size_t s_nItems;
                const void* s_pData;
            } Result, *pResult;
        public:
            CDataReader(const char* pFilename, std::size_t bufferSize);
            CDataReader(int fd,  std::size_t bufferSize);
            virtual ~CDataReader();
            
        private:
            CDataReader(const CDataReader& rhs);
            CDataReader& operator=(const CDataReader& rhs);
            int operator==(const CDataReader& rhs);
            int operator!=(const CDataReader& rhs);
        public:
            Result getBlock(std::size_t maxbytes);
            void done();
        private:
            void allocateBuffer();
            void fillBuffer();
            void probeData(std::size_t maxBytes);
        };
    }
}


#endif
