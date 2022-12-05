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

/** @file:  Event.h
 *  @brief: Stubbed CEvent for parallel analysis.
 */
#ifndef ANALYSIS_EVENT_H
#define ANALYSIS_EVENT_H
#include <vector>
#include "SpecTclTypes.h"

namespace frib {
    namespace analysis {
        typedef std::vector<unsigned> DopeVector;  // the scoreboard.
        typedef double CParameterValue;
        typedef std::vector<double>::iterator CEventIterator;
        /**
         * @class CEvent
         *    This is an important class in SpecTcl.  It holds the event
         *    that event processors generate from the raw data or other
         *    event processor results.  As much as possible we'll try
         *    to implement this in a sensible way. Specifically, if an
         *    index already has a tree parameter assigned to it, we'll use
         *    that tree parameter as a target for references into the
         *    CEventObject.   If there is no TreeParameter we'll generate
         *    one named 'unamed.n'  where .n is the index into the
         *    array referenced.  This is not recommended as by the time this happens
         *    the outputter will arleady have output the Treeparameter definitions.
         *    
         */
        class CEvent {
        public:
            // Canonical functions, construction, destruction, assignment and comparison.
            // Ensure initial values entered.
            CEvent();
            virtual ~CEvent();
            CEvent(UInt_t nInitialSize);
            CEvent(const CEvent& aEvent);
          
            CEvent& operator=(const CEvent& aEvent);
            int operator==(const CEvent& aEvent);
            int operator!=(const CEvent& anEvent) ;
            CParameterValue& operator[](UInt_t nParam);
          
            CEventIterator begin();
            CEventIterator end();
            UInt_t size();
            void clear() ;
            DopeVector getDopeVector();
         private:
            void makeParameter(unsigned index);
           
        };
        
    }
}
#endif