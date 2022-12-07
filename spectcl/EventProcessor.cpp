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

/** @file:  Eventprocessor.cpp
 *  @brief: Provide default (no-op) implementations for CEventProcessor
 */

#include "EventProcessor.h"

namespace frib {
    namespace analysis {
        
        Bool_t
        CEventProcessor::OnEventSourceOpen(std::string name) {
            return kfTRUE;
        }
        
        Bool_t
        CEventProcessor::OnInitialize() {
            return kfTRUE;
        }
    }
}