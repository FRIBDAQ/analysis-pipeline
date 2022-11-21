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

/** @file:  TriggerSorter.cpp
 *  @brief:  Implement the trigger sorter class.
 */
#include "TriggerSorter.h"

namespace frib {
    namespace analysis {
        /**
         * constructor
         *    The hardest part of the constructor is initializing the
         *    last emitted trigger... we emit it to 0-1 unsigned so that
         *    it + 1 (0) is the next trigger.
         */
        CTriggerSorter::CTriggerSorter() :
            m_lastEmittedTrigger(0-1)
        {
            
        }
        /**
         * destructor
         * 
         *   Flush the items in case the client has not done that
         */
        CTriggerSorter::~CTriggerSorter() {
            flush();
        }
        /**
         * addItem
         *    - If the item is the next trigger just emit it.
         *       otherwise shove it into m_items indexed by trigger#
         *    - while the map is not empty and the 'first' item's trigger
         *      is sequential, emit it and remove it from the map.
         *  A bit on ownereship
         *     Ownership of the item is ours and passes to emitItem or whatever it
         *     does.  Note that in most of the frameworks we put his class into,
         *     delete should should be called by emitItem to get rid of the
         *     item.
         * @param item   pointer to the item to add/sort/emit.
         * @note the 'sorting' via the map is amortized O(nLog(m)) where m
         * is the number of in-flight items and n is the total number of items.
         */
        void
        CTriggerSorter::addItem(pParameterItem item) {
            auto trigger = item->s_triggerCount;
            if((m_lastEmittedTrigger +1) == trigger) {
                emitItem(item);
                m_lastEmittedTrigger++;
                // See if this unblocked emitting other items:
                
                while (!m_items.empty()) {
                    auto p = m_items.begin();
                    if (p->first == (m_lastEmittedTrigger+1)) {  // can emit?
                        auto pItem = p->second;
                        m_items.erase(p);
                        emitItem(pItem);
                        m_lastEmittedTrigger++;
                    } else {                              // no so done.
                        break;
                    }
                }
                
            } else {
                m_items[trigger] = item;
                // If we did this, we can't emit.
            }
            
        }
        /**
         * flush
         *   flush all elements of m_items -> emitItem
         *   @note that at the end of this m_items will be empty.
         *   @note if the application operates properly, this should not really
         *   do anything as the application is supposed to hand us empty parameter
         *   item placeholders for events that were software filtered out.
         */
        void CTriggerSorter::flush() {
            for (auto& p: m_items) {
                emitItem(p.second);
            }
            m_items.clear();
        }
    }
}