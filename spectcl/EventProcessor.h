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

/** @file:  EventProcessor.h
 *  @brief: Stand in for SpecTcl Event processor base class.
 */
#ifndef ANALYSIS_EVENTPROCESSOR_H
#define ANALYSIS_EVENTPROCESSOR_H
#include "SpecTclTypes.h"
#include <string>
namespace frib {
    namespace analysis {
        class CEvent;
        class CAnalyzer;
        class CBufferDecoder;
        
        /**
         * @class CEventProcessor
         *    Base class for ports of SpecTcl event processors into the paralle
         *    analysis framework. The only methods that get called are
         *    OnInitialize, operator() and OnEventSourceOpen and OnEventSourceEOF.
         */
        class CEventProcessor {
        public:
            virtual ~CEventProcessor() {}               // for destructor chaining.
            
            virtual Bool_t operator()(const Address_t pEvent,
                            CEvent& rEvent,
                            CAnalyzer& rAnalyzer,
                            CBufferDecoder& rDecoder) = 0;
            virtual Bool_t OnEventSourceOpen(std::string name);
            virtual Bool_t OnEventSourceEOF();
            virtual Bool_t OnInitialize();

                
        };
    }
}


#endif