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

/** @file:  Event.cpp
 *  @brief: Implement frib::analysis::CEvent
 */
#include "Event.h"
#include "TreeParameter.h"
#include <algorithm>
#include <sstream>
namespace frib {
    namespace analysis {
        /**
         * constructors do nothing as there's no data to initialize.
         */
        CEvent::CEvent() {}
        CEvent::CEvent(UInt_t nInitialSize) {}
        CEvent::CEvent(const CEvent& aEvent) {}
        
        /**
         * destructor is also nill because there's not any data to destroy.
         */
        CEvent::~CEvent() {}
        
        // Other canonicals:
        
        CEvent& CEvent::operator=(const CEvent& aEvent) {return *this;}
        int CEvent::operator==(const CEvent& aEvent) {return 1;}
        int CEvent::operator!=(const CEvent& anEvent) {return 0;}
        
        /**
         * operator[]
         *    Indexing the revent is mostly as simple as providing the
         *    index to the array.  _However_
         *    -  If the index does not exist a new tree parameter is created.
         *    -  If the index is not in the scoreboard it's added.
         * @parameter nParam - index into the parameter vector.
         * @return CParameterValue&  where CParameterValue is just a double.
         * @note Since we don't touch the underlying tree parameter and its shared data,
         *    isValid isn't going to work on this   That's ok because there's
         *    no isValid in CParameterValue anymore.  We strongly suggest the
         *    use of CTreeParameters and not CEvent as targets of the
         *    unpacking.
         *    
         */
        CParameterValue&
        CEvent::operator[](UInt_t nParam) {
            // If necessary make the tree parameter.
            
            
            if (CTreeParameter::m_event.size() <= nParam) {
                makeParameter(nParam);
            }
            if (std::find(
                CTreeParameter::m_scoreboard.begin(),
                CTreeParameter::m_scoreboard.end(),
                nParam) == CTreeParameter::m_scoreboard.end()) {
                CTreeParameter::m_scoreboard.push_back(nParam);
            }
            return CTreeParameter::m_event[nParam];
            
        }
        
        /**
         * begin - provide an iterator to the event data.
         * @return CEventIterator
         */
        CEventIterator
        CEvent::begin() {
            return CTreeParameter::m_event.begin();
        }
        /**
         * end
         *  end iterator for the event.
         *  @return CEventIterator
         */
        CEventIterator
        CEvent::end() {
            return CTreeParameter::m_event.end();
        }
        /**
         * size()
         *    Size of the event array.
         */
        UInt_t
        CEvent::size() {
            return CTreeParameter::m_event.size();
        }
        /**
         * clear
         *     Just a stand in for nextevent
         */
        void
        CEvent::clear() {
            CTreeParameter::nextEvent();
        }
        /**
         * getDopeVector
         *   A stand in for getScoreboard
         */
        DopeVector
        CEvent::getDopeVector() {
            return CTreeParameter::getScoreboard();
        }
        //////////////////////////////////////////////////////////////////////
        // Utility methods
        //
        
        /**
         * makeParameter
         *     Well this really might make many parameters.  Given an index
         *     we add Tree parameter instances until CTreeParameter::m_event.size()
         *     can accomodate a specific index.
         *     These tree parameters have names but should be thought of as anonymous.
         *     
         */
        void
        CEvent::makeParameter(unsigned index) {
            unsigned n = CTreeParameter::m_event.size();
            do {
               std::stringstream namestr;
               namestr << "_unnamed." << n;
               std::string name = namestr.str();
               CTreeParameter p(name);
               n++;
            } while (n <= (index));
        }
        
        
    }
}