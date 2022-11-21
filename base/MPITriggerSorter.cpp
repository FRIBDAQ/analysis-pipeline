/*
 *
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

/** @file:  MPITriggerSorter.cpp
 *  @brief: Implement the CMPITriggerSorter class.
 */

#include "MPITriggerSorter.h"
#include <stdexcept>
const unsigned INITIAL_MAX_ITEMS(100);

namespace frib {
    namespace analysis {
        /**
         * constructor
         *   The parameters really tell us how to talk to the outputter:
         * @param outputRank - the rank to which we send our sorted items.
         * @param headers    - MPI Data type for FRIB_MPI_Parameter_MessageHeader
         *                     normally AbstractApplication computed this.
         * @param param      - MPIData type for FRIB_MPI_Parameter_Value which, again,
         *                     is normally created by AbstractApplication.
         * @note - we're going to pre-allocate 100 FRIB_MPI_Parameter_Value items
         *         just to get us started.
         */
        CMPITriggerSorter::CMPITriggerSorter(
            int outputterRank, MPI_Datatype& headers, MPI_Datatype& param
        ) :m_outputRank(outputterRank), m_headerType(headers),
        m_parameterType(param), m_maxItems(INITIAL_MAX_ITEMS),
        m_items(new FRIB_MPI_Parameter_Value[INITIAL_MAX_ITEMS]) {}
        
        /**
         *  Destructor -The cool thing about unique pointers is they destroy themselves.,\
         */
        CMPITriggerSorter::~CMPITriggerSorter() {
            
        }
        /**
         * emitItem
         *    Marshall the event up into a header and parameter block and send it on
         *    its way to the m_outputRank receiver.
         * @param item -pointer to the ring item that contains the parameters.
         * @note item will be deleted after we no longer need its data.
         */
        void
        CMPITriggerSorter::emitItem(pParameterItem item) {
            //Make the header:
            
            FRIB_MPI_Parameter_MessageHeader header;
            header.s_triggerNumber = item->s_triggerCount;
            header.s_numParameters = item->s_parameterCount;
            header.s_end           = false;
            
            // Make the item:
            
            if (header.s_numParameters > m_maxItems) {
                m_items.reset(new FRIB_MPI_Parameter_Value[header.s_numParameters]);
                m_maxItems = header.s_numParameters;
            }
            auto pItems = m_items.get();
            for (int i =0; i < header.s_numParameters; i++) {
                pItems->s_number = item->s_parameters[i].s_number;
                pItems->s_value  = item->s_parameters[i].s_value;
                pItems++;
            }
            
            delete item;                  // No longer needed.
            
            // Send them to m_outputRank
            
            char reason[MPI_MAX_ERROR_STRING];
            int len;
            
            int status = MPI_Send(
                &header, 1, m_headerType,
                m_outputRank, MPI_HEADER_TAG, MPI_COMM_WORLD
            );
            if (status != MPI_SUCCESS) {
                MPI_Error_string(status, reason, &len);
                std::string msg = "Unable to send parameter header to output: ";
                msg += reason;
                throw std::runtime_error(msg);
            }
            
            status = MPI_Send(
                m_items.get(), header.s_numParameters, m_parameterType,
                m_outputRank, MPI_DATA_TAG, MPI_COMM_WORLD
            );
            if (status != MPI_SUCCESS) {
                MPI_Error_string(status, reason, &len);
                std::string msg = "Unable to send parameter data to output: ";
                msg += reason;
                throw std::runtime_error(msg);
            }
        }
    }
}