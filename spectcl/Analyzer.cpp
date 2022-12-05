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

/** @file:  Analyzer.cpp
 *  @brief: Implement CAnalyzer stub class.
 */

#include "Analyzer.h"
#include "BufferDecoder.h"
#include "TreeParameter.h"

namespace frib {
    namespace analysis {
        static CBufferDecoder dummyDecoder;
        
        /**
         * constructors - they really don't do anything.
         */
        CAnalyzer::CAnalyzer() {}
        CAnalyzer::CAnalyzer(
            UInt_t am_nParametersInEvent,
            UInt_t nThreshold) {}
        CAnalyzer::CAnalyzer(const CAnalyzer& aCAnalyzer) {}
            
        /**
         * destructor is also nil.
         */
        CAnalyzer::~CAnalyzer() {}
        
        /**
         * other canonicals.
         */
        
        CAnalyzer& CAnalyzer::operator=(const CAnalyzer& aCAnalyzer) {return *this;}
        int CAnalyzer::operator==(const CAnalyzer& aCAnalyzer) { return 1;}
        int CAnalyzer::operator!=(const CAnalyzer& aCAnalyzer) { return 0;}
    
        /**
         * getEventThreshol
         *   @return 0
         */
        UInt_t
        CAnalyzer::getEventThreshold() const { return 0;}
        /**
         * getParametersInEvent
         *    Return then number of parameters defined for an event.
         *    This is the number of tree parameters defined.
         *  @return UInt_t
         */
        UInt_t
        CAnalyzer::getParametersInEvent() const {
            return CTreeParameter::getDefinitions().size();    // Not terribly efficient.
        }
        /**
         * getDecoder
         * @return CBufferDecoder* (&dummyDecoder).
         */
        CBufferDecoder*
        CAnalyzer::getDecoder() {
            return &dummyDecoder;
        }
        /**
         * entityNotDone
         *    This is not implemented in the sense that it is a no-op.  To implememnt
         *    it will require coordination with the larger skeleton.  Specifically,
         *    it will meant that the caller of the user code must not:
         *    #   Send the parameters from the call to user code to the farmer.
         *    #   Ask for a new tree parameter event.
         *  The intent was to allow an event's parameters to be composed from two
         *  sub-events.  This was important in NSCLDAQ-8 but I don't think there are
         *  use-cases in NSCLDAQ-10 and later.
         */
        void
        CAnalyzer::entityNotDone() {}
    }
}
