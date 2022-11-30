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

/** @file:  MPIParamterFarmer.cpp
 *  @brief:  Implements the CMPIParameterFarmer class.
 */
#include "MPIParameterFarmer.h"
#include "AbstractApplication.h"
#include "MPITriggerSorter.h"
#include <mpi.h>
#include <iostream>

namespace frib {
    namespace analysis {
        /**
         * constructor
         *    Simply save the argc/argv in case we later need it (this implementation
         *    does not but later we might have some blocking factors).
         *    Save the application so that we can get at the pre-built data types.
         *
         *    The idea is that there's construction of the proceess and then, by
         *    calling the object as a functor, the process rus.
         * @param argc, argv - the command line parameters.
         * @param app        - References the application object.
         */
        CMPIParameterFarmer::CMPIParameterFarmer(
            int argc, char** argv, AbstractApplication& app
        ) : m_argc(argc), m_argv(argv), m_App(app),
        m_nMaxParams(100), m_parameterBuffer(new FRIB_MPI_Parameter_Value[100])
        {}
        
        /**
         * Destructor kill the buffer:
         */
        CMPIParameterFarmer::~CMPIParameterFarmer() {
            delete []m_parameterBuffer;
        }
        
        /**
         * operator()
         *   - Figure out how many workers there are so we can count down m_nEndsLeft.
         *   - Instantiate a CMPITriggerSorter to re-order the triggers properly.
         *   - Accept header/data pairs (or just headers in the case of an end) until
         *   all of the workers have sent ends - then flush the sorter and send an end
         *   to the outputter.
         */
        void
        CMPIParameterFarmer::operator()() {
            m_nEndsLeft = m_App.numWorkers();
            CMPITriggerSorter sorter(
                2, m_App.parameterHeaderDataType(),
                m_App.parameterValueDataType()
            );
            while (m_nEndsLeft) {
                pParameterItem pItem = getItem();
                if (pItem) {
                    sorter.addItem(pItem);     // If possible this will send items.
                } else {
                    m_nEndsLeft--;
                    
                }
            }
            sorter.flush();
            sendEnd();
            
        }
        ///////////////////////////////////////////////////////////////////////
        // Private utilities.
        
        /**
         * sendEnd
         *    Send an end to the outputter on rank 2.
         */
        void
        CMPIParameterFarmer::sendEnd() {
            FRIB_MPI_Parameter_MessageHeader header;
            header.s_triggerNumber = 0;
            header.s_numParameters = 0;
            header.s_end = true;
            char error[MPI_MAX_ERROR_STRING];
            int len;
            int status = MPI_Send(
                &header, 1, m_App.parameterHeaderDataType(),
                2, MPI_END_TAG, MPI_COMM_WORLD
            );
            if (status != MPI_SUCCESS) {
                MPI_Error_string(status, error, &len);
                std::string message = "Unable to send end message to outputter: ";
                message += error;
                throw std::runtime_error(message);
            }
        }
        /**
         * getItem
         *   Get an item from a worke... any worker. Put it in a pParameterItem
         *   dynamically allocated via new and return it.
         *   If the header from the worker has s_end true, a null pointer is returned.
         *   Note that in multiple workers other workers  may well have data in the pipe
         *   after the first end is received from a worker.
         *
         *   @return pParameterItem - dynamically allocated parameter item.
         */
        pParameterItem
        CMPIParameterFarmer::getItem()
        {
            pParameterItem result=nullptr;
            char error[MPI_MAX_ERROR_STRING];
            int len;
            MPI_Status mpistat;
            
            // Get the header first:
            // Note that due to the fact that multiple senders are operating
            // asynchronously we'll explicitly specify the rand from which to get
            // the data item:
            
            
            FRIB_MPI_Parameter_MessageHeader header;
            int status = MPI_Recv(
                &header, 1, m_App.parameterHeaderDataType(),
                MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD,
                &mpistat
            );
            if (status != MPI_SUCCESS) {
                MPI_Error_string(status, error, &len);
                std::string message = "Unable to receive parameter header ";
                message += error;
                throw std::runtime_error(message);
            }
            // Must be a header tag:
            
            if (
                (mpistat.MPI_TAG != MPI_HEADER_TAG) &&
                (mpistat.MPI_TAG != MPI_END_TAG)
            ) {
                throw std::logic_error("Farmer expected header or end tag");
            }
            int from = mpistat.MPI_SOURCE;
            
            // If this is an end, return null:
            
            if (!header.s_end) {
                
               // If needed, enlarge m_parameterBuffer:
               
               if (header.s_numParameters > m_nMaxParams) {
                   delete []m_parameterBuffer;
                   m_parameterBuffer = new FRIB_MPI_Parameter_Value[header.s_numParameters];
                   m_nMaxParams = header.s_numParameters;
               }
               // Receive the parameter record itself:
               
               status = MPI_Recv(
                   m_parameterBuffer, header.s_numParameters, m_App.parameterValueDataType(),
                   from, MPI_ANY_TAG, MPI_COMM_WORLD,
                   &mpistat
               );
               
               // Status must be ok and tag must be MPI_DATA_TAG.
               
               if (status != MPI_SUCCESS) {
                   MPI_Error_string(status, error, &len);
                   std::string message ="Unable to receive parameters from worker: " ;
                   message += error;
                   throw std::runtime_error(message);
               }
               if (mpistat.MPI_TAG != MPI_DATA_TAG) {
                   throw std::logic_error("Expected data tag but got something else in farmer");
               }
               // Now we can allocate the ring item and marshall the data into it:
               
               std::uint32_t totalSize = sizeof(ParameterItem) + header.s_numParameters*sizeof(ParameterValue);
               std::uint8_t* pDummy = new std::uint8_t[totalSize];
               result = reinterpret_cast<pParameterItem>(pDummy);
               
               // Marshall the data:
               // Note/TODO:  This may well become the bottleneck for dataflow.  IF
               // that's the case we need, instead, to pass the FRIB parameter data
               // we got to the sorter directly so that we can get rid of one of the
               // data movements.  In that case, we'll be allocating m_parameterBuffer
               // each event.
               
               result->s_header.s_size = totalSize;
               result->s_header.s_type = PARAMETER_DATA;
               result->s_header.s_unused = sizeof(std::uint32_t);
               
               result->s_triggerCount = header.s_triggerNumber;
               result->s_parameterCount = header.s_numParameters;
               
               for (int i =0; i < header.s_numParameters; i++) {
                   result->s_parameters[i].s_number = m_parameterBuffer[i].s_number;
                   result->s_parameters[i].s_value  = m_parameterBuffer[i].s_value;
               }
            }            
            return result;
        }
        
    }
}