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

/** @file:  BufferDecoder.h
 *  @brief: Stub Buffer Decoder.
 */
#ifndef ANALYSIS_BUFFERDECODER_H
#define ANALYSIS_BUFFERDECODER_H
#include "SpecTclTypes.h"
#include <string>

namespace frib {
    namespace analysis {
        /**
         * @class CBufferDecoder
         *     In SpecTcl, the buffer decoder class offers specific services
         *     to event processors that are not available in the parallel
         *     analysis framework since not all workers will see the state
         *     transition items that are used to gather the information needed
         *     to provide those services.  What we do is offer the interfaces,
         *     so the user code can compile and either return dummy stuff or
         *     throw exceptions if there's nothing reasonable to return.
         */
        
        class CBufferDecoder {
        private:
            Address_t m_pItem;
        public:
            
            CBufferDecoder();
            virtual ~CBufferDecoder();
            const Address_t getBuffer();
        
            virtual const Address_t getBody();
            virtual UInt_t getBodySize() ;
            virtual UInt_t getRun();
            virtual UInt_t getEntityCount();
            virtual UInt_t getSequenceNo();
            virtual UInt_t getLamCount();
            virtual UInt_t getPatternCount();
            virtual UInt_t getBufferType();
            virtual void getByteOrder(Short_t& Signature16,
                                      Int_t& Signature32);
            virtual std::string getTitle();
            virtual bool blockMode();     // True if data source must deliver fixed sized blocks.
            // Used by the framework
            
            void setBody(Address_t p);
        };
    }
}


#endif
