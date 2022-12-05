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

/** @file:  Analyzer.h
 *  @brief: Stub replacement for SpecTcl CAnalyzer class.
 */
#ifndef ANALYSIS_ANALYZER_H
#define ANALYSIS_ANALYZER_H
#include "SpecTclTypes.h"

namespace frib {
    namespace analysis {
        class CBufferDecoder;

        
        /**
         * @class CAnalyzer
         *    This is intended as a stub class to satisfy refernces to the
         *    SpecTcl analyzer that, of necessity, SpecTcl event processors will
         *    have.  Some services are provided to that code (I think) regardless
         *    an instance is passed to event processors.
         *
         *    @note some CAnalyzer methods are missing as they are purely
         *    internal to SpecTcl and even the analyzer itself.
         *    
         */
        class CAnalyzer {
        private:
            static const  UInt_t m_nDefaultEventThreshold = 0;
        public:
            CAnalyzer();
            virtual ~CAnalyzer();      
            CAnalyzer(UInt_t am_nParametersInEvent,
                  UInt_t nThreshold = CAnalyzer::m_nDefaultEventThreshold);
            CAnalyzer(const CAnalyzer& aCAnalyzer);
          
            // Operator= Assignment Operator
            CAnalyzer& operator=(const CAnalyzer& aCAnalyzer);
            int operator==(const CAnalyzer& aCAnalyzer);
            int operator!=(const CAnalyzer& aCAnalyzer);
            UInt_t getEventThreshold() const;
            UInt_t getParametersInEvent() const;
          
            CBufferDecoder* getDecoder();           
            void            entityNotDone();

        };
        
    }
}

#endif