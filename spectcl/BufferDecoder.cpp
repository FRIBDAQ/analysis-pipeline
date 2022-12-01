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

/** @file:  BufferDecoder.cpp
 *  @brief: Implement the stubbed CBufferDecoder class
 */

#include "BufferDecoder.h"
#include "AnalysisRingItems.h"

namespace frib {
    namespace analysis {
        /**
         * constructor
         */
        CBufferDecoder::CBufferDecoder() : m_pItem(nullptr) {}
        /**
         * destructor
         */
        CBufferDecoder::~CBufferDecoder() {}
        
        /**
         * getBody 
         *   The body will point to the ring item as it does for RingBufferDecoder.
         *   that body will have to be set by the worker framework by calling
         *   setBody.
         *  @return Address_t - pointer to the ring item.
         */
        const Address_t
        CBufferDecoder::getBody() {
            return m_pItem;
        }
        /**
         * getBodySize
         *    Treating m_pItem as a ring item, return its size.
         */
        UInt_t
        CBufferDecoder::getBodySize() {
            pRingItemHeader pH = reinterpret_cast<pRingItemHeader>(m_pItem);
            return pH->s_size;
        }
        /**
         * getRun
         *     We don't know the run so always return 0.
         *  @return UInt_t (0).
         */
        UInt_t
        CBufferDecoder::getRun() {
            return 0;
        }
        /**
         *getEntityCount
         *  @return UInt_t - by definition a ring item has one entity
         */
        UInt_t
        CBufferDecoder::getEntityCount() {
            return 1;
        }
        /**
         * getSequenceNo
         *   @return UInt_t - we really don't have a sequence number so return 0
         */
        UInt_t
        CBufferDecoder::getSequenceNo()
        {
            return 0;
        }
        /**
         * getLamCount
         *    This is deprecated and has been returning 0 for some time.
         * @return UInt_t 0
         */
        UInt_t
        CBufferDecoder::getLamCount() {
            return 0;
        }
        /**
         * getPaternCount
         *    Similarly, this has been returning 0 for some time
         * @return UInt_t 0
         */
        UInt_t
        CBufferDecoder::getPatternCount()  {
            return 0;
        }
        /**
         * getBufferType
         *   Since we're only going to call the uperator() this will
         *   always be 1 for physics event.
         *@returnUint_t 1
         */
        UInt_t
        CBufferDecoder::getBufferType() {
            return 1;
        }
        /**
         * getByteOrder
         *    Little endian won the world.  We just return the machine's byt order
         *    signatures.
         *  @param[out] signature16 - 16 bit sig.,
         *  @param[out] signature32 - 32 bit sig.
         */
        void
        CBufferDecoder::getByteOrder(
            Short_t& Signature16, Int_t& Signature32
        ) {
            Signature16 = 0x0102;
            Signature32 = 0x01020304;
        }
        /**
         * getTitle
         *    We don't have a title but we can return an empty string.
         * @return std::string ""
         */
        std::string
        CBufferDecoder::getTitle() {
            return "";
        }
        /**
         * Ring items are not block mode.
         */
        bool
        CBufferDecoder::blockMode() {
            return false;
        }
        /**
         * setBody
         *    Used by the framework to set the value of m_pItem
         */
        void
        CBufferDecoder::setBody(Address_t p) {
            m_pItem= p;
        }
    }
}