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

/** @file:  TriggerSorter.h
 *  @brief: Sort items by triggers that could come in out of order.
 */
#ifndef TRIGGERSORTER_H
#define TRIGGERSORTER_H

#include <cstdint>
#include <AnalysisRingItems.h>
#include <map>
namespace frib {
    namespace analysis {
        /**
         * @class CTriggerSorter
         *     CTriggerSorter has methods to add parameter ring items
         *     which may be out of order and then to emit ring items later
         *     in order.
         *
         *    This is implemented in a manner that does not depend on MPI so that
         *    ordinary unit tests can be used to verify the operation of the class.
         *    The idea is that addItem is called to add items to the sorting
         *    system.  When one or more items can be emitted (because they have
         *    triggers sequential to the last emitted item), emitItem is called
         *    with those items.
         *
         *    emitItem is pure virtual so that derived classes can decide what to
         *    actually do with items that are sorted.
         */
        class CTriggerSorter {
        private:            
            std::map<std::uint64_t, pParameterItem> m_items;
            std::uint64_t                           m_lastEmittedTrigger;
        public:
            CTriggerSorter();
            virtual ~CTriggerSorter();
            
            void addItem(pParameterItem item);
            void flush();
            virtual void emitItem(pParameterItem item) = 0;
        };
    }
}

#endif
