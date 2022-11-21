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

/** @file:  MPITriggerSorter.h
 *  @brief: Sorts triggers sent by workers sending them to outputter.
 *  
 */
#ifndef MPITRIGGERSORTER_H
#define MPITRIGGERSORTER_H

#include "TriggerSorter.h"
#include <mpi.h>
#include <vector>
#include <memory>

namespace frib {
    namespace analysis {
        /**
         * @class CMPITRiggerSorter
         *
         * Specializes the CTriggerSorter class so that
         * emitItem method pushes items to an MPIParameterOutpu object.
         * running in the specified rank.
         */
        class CMPITriggerSorter : public CTriggerSorter {
        private:
            int          m_outputRank;
            MPI_Datatype& m_headerType;    // MPI Data type for header data.
            MPI_Datatype& m_parameterType; // MPI Data type for the parameters.
            unsigned      m_maxItems;      // # items that can fit in m_item.
            std::unique_ptr<FRIB_MPI_Parameter_Value> m_items; // To avoid allocation each event.
        public:
            CMPITriggerSorter(
                int outputterRank, MPI_Datatype& headers, MPI_Datatype& param
            );
            virtual ~CMPITriggerSorter();
            
            virtual void emitItem(pParameterItem item);
        };
    }
}



#endif