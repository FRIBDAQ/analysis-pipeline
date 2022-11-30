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

/** @file:  MPIRawToParametersWorker.cpp
 *  @brief: Implement the CMPIRawToParametersWorker class.
 */
#include "MPIRawToParametersWorker.h"
#include "AnalysisRingItems.h"
#include "AbstractApplication.h"
#include "TreeParameter.h"
#include <mpi.h>
#include <memory>
#include <stdexcept>
#include <string>
#include <iostream>

namespace frib {
    namespace analysis {
        const std::uint32_t PHYSICS_EVENT=30;
        /**
         * Constructor
         *    @param App - referencees the application
         */
        CMPIRawToParametersWorker::CMPIRawToParametersWorker(
            AbstractApplication& App
        ) : m_App(App), m_pParameterBuffer(nullptr), m_paramBufferSize(0)
        {
            int status = MPI_Comm_rank(MPI_COMM_WORLD, &m_rank);
            throwMPIError(status, "Unable to obtain worker rank: ");
        }
        
        /** Destructor
         *    Kill off the parameter bufer:
         */
        CMPIRawToParametersWorker::~CMPIRawToParametersWorker() {
            delete []m_pParameterBuffer;
        }
        
        /**
         * operator()
         *    The entry point for the application object.
         *    - Initialize the user code.
         *    - Until we get an end header, request data/get data
         *    -   Process the data block.
         *    
         * @param argc,argv - the program parameters.
         */
        void
        CMPIRawToParametersWorker::operator()(int argc, char** argv) {
            initializeUserCode(argc, argv, m_App);
            std::unique_ptr<std::uint8_t> pData;
            size_t                         bytesReserved(0);
            while (1) {
                
                requestData();
                FRIB_MPI_Message_Header header;
                getHeader(header);
                
                
                if (!header.s_end) {
                    // If necessary , resize the data block.
                    if (header.s_nBytes > bytesReserved) {
                        pData.reset(new std::uint8_t[header.s_nBytes]);
                        bytesReserved = header.s_nBytes;
                    }
                    getData(pData.get(), header.s_nBytes);
                    processDataBlock(pData.get(), header.s_nBytes, header.s_nBlockNum);
                    
                } else {
                    // End of data.
                    
                    sendEnd();
                    break;
                }
            }
        }
        /**
         * requestData
         *    Make a data request from rank 0 (the dealer). Error result in
         *    a runtime error.
         */
        void
        CMPIRawToParametersWorker::requestData() {
            FRIB_MPI_Request_Data req;
            req.s_requestor = m_rank;
            req.s_maxdata   = 1024*1024;   // currently gets ignored anyway.
            
            int status = MPI_Send(
                &req, 1, m_App.requestDataType(),
                0, MPI_REQUEST_TAG, MPI_COMM_WORLD
            );
            throwMPIError(status, "Unable to send work request: ");
        }
        /**
         * getHeader
         *    Read the data header from the rank 0 (dealer).
         * @param header - references where to put the data.
         */
        void
        CMPIRawToParametersWorker::getHeader(FRIB_MPI_Message_Header&  header) {
            MPI_Status info;
            int status = MPI_Recv(
                &header, 1, m_App.messageHeaderType(),
                0, MPI_HEADER_TAG, MPI_COMM_WORLD, &info
            );
            throwMPIError(status, "Unable to read data header: ");
        }
        /**
         * getData
         *    Having gottena header from the dealer, get the data itself.
         * @param pData -buffer for the data.
         * @param nBytes - size of pData.
         */
        void
        CMPIRawToParametersWorker::getData(void* pData, size_t nBytes) {
            MPI_Status s;
            int status = MPI_Recv(
                pData, nBytes, MPI_UINT8_T, 0, MPI_DATA_TAG, MPI_COMM_WORLD, &s
            );
            throwMPIError(status, "Unable to receive data block from dealer: ");
        }
        /**
         * forwardPassthrough
         *    Sends data to the outputter for a passthrough item.
         *    Passthrough items are ring items that are not PHYSICS_EVEN types and
         *    therefore are not processed by the framework.
         *  @param pData - pointer to the passthrough data.
         *  @param nBytes - Number of data bytes to passthrough.
         */
        void CMPIRawToParametersWorker::forwardPassthrough(
            const void* pData, size_t nBytes
        )  {
            // The data uses a parameter header but a passthrough tag:
            
            FRIB_MPI_Parameter_MessageHeader header;
            header.s_triggerNumber = 0;       // ignored.
            header.s_numParameters = nBytes;  // Actualy block size...
            header.s_end           = false;   // not an end.
            int status = MPI_Send(
                &header, 1, m_App.parameterHeaderDataType(),
                2, MPI_PASSTHROUGH_TAG, MPI_COMM_WORLD
            );
            throwMPIError(status, "Failed to send passthrough header: ");
            
            // Now the data block itself:
            
            status = MPI_Send(
                pData, nBytes, MPI_UINT8_T,
                2, MPI_DATA_TAG, MPI_COMM_WORLD
            );
            throwMPIError(status, "Failed to send passthrough data block: ");
        }
        /**
         * sendParameters
         *    Sends a parameters data block to the farmer (rank 1).
         * @param event - the event represented as pairs of parmeter id/values.
         * @param trigger - thrigger number to associated with the event.
         */
        void
        CMPIRawToParametersWorker::sendParameters(
            const std::vector<std::pair<unsigned, double>>& event,
            std::uint64_t trigger
        ) {
            
            // See if we need to expand the parameter buffer:
            
            if (event.size() > m_paramBufferSize) {
                delete m_pParameterBuffer;
                m_pParameterBuffer = new FRIB_MPI_Parameter_Value[event.size()];
                m_paramBufferSize= event.size();
            }
            // Marshall the data into the parameter buffer:
            
            auto p = m_pParameterBuffer;
            for (auto& item: event) {
                p->s_number = item.first;
                p->s_value = item.second;
                p++;
            }
            
            // Format and send the header:
            
            FRIB_MPI_Parameter_MessageHeader header;
            header.s_triggerNumber = trigger;
            header.s_numParameters = event.size();
            header.s_end           = false;
            
            int status = MPI_Send(
                &header, 1, m_App.parameterHeaderDataType(),
                1, MPI_HEADER_TAG, MPI_COMM_WORLD
            );
            throwMPIError(status, "Failed to send parameter header: ");
            
            // Send the data:
            
            status = MPI_Send(
                m_pParameterBuffer, event.size(), m_App.parameterValueDataType(),
                1, MPI_DATA_TAG, MPI_COMM_WORLD
            );
            
            throwMPIError(status, "Failed to send parameter data to farmer: ");
        }
        /**
         *  sendEnd
         *     Send an end of data for us to the farmer (rank1).  Note that
         *     the farmer will keep getting data until all worker ranks have
         *     sent ends.
         */
        void
        CMPIRawToParametersWorker::sendEnd()
        {
            FRIB_MPI_Parameter_MessageHeader header;
            header.s_triggerNumber = 0;
            header.s_numParameters = 0;
            header.s_end           = true;
            
            int status = MPI_Send(
                &header, 1, m_App.parameterHeaderDataType(),
                1, MPI_END_TAG, MPI_COMM_WORLD
            );
            
            throwMPIError(status, "Failed to send end flag to farme4r: ");
        }
        /**
         * processDataBLock
         *    Datablocks received from the dealer contain ring items. Most
         *    of these ring items are of type PHYSICS_EVENT but some are other
         *    types. This method goes through all of the ring items in a block
         *    and does a forwardPassthrough on all non PHYSICS_EVENT items.
         *    for PHYSCIS_EVENT items:
         *     - unpackData is called with a pointer to the ring item.
         *     - the resulting event is marshalled from the tree parameters.
         *     - the marshalled event is sent to the farmer (rank 1).
         *     - The tree parameter subsystem is told to re-initialize for the next
         *        event.
         *  @note - since MPI is process level parallelism, each worker has its own
         *       independent set of tree parameters and all the nasties of
         *       threaded SpecTcl are side-stepped.
         *  @param pData - pointer to the data block.
         *  @param nBytes - number of bytes in the block.
         *  @param firstTrigger - trigger number to assign to the first physics item.
         *  @note nBytes is assured to contain complete ring items.
         */
        void
        CMPIRawToParametersWorker::processDataBlock(
            const void* pData, size_t nBytes, std::uint64_t firstTrigger
        ) {            
            union {
                const RingItemHeader* pH;
                const std::uint8_t*   p8;
            } p;
            p.p8 = reinterpret_cast<const std::uint8_t*>(pData);
            
            
            while (nBytes) {
                
                if (p.pH->s_type == PHYSICS_EVENT) {
                
                    unpackData(p.pH);
                    auto event = CTreeParameter::collectEvent();
                    sendParameters(event, firstTrigger);
                    CTreeParameter::nextEvent();
                    firstTrigger++;
                    
                } else {
                    // passthrough:
                    
                    forwardPassthrough(p.p8, p.pH->s_size);
                }
                
                // Next item:
                
                nBytes -= p.pH->s_size;
                p.p8   += p.pH->s_size;
            }
        }
        /**
         * throwMPIError
         *    Common code utility to check the status of an MPI call and report
         *    an error if necessasry.  Errors are reported by throwing an
         *    std::runtime_error with a message constructed of the string
         *    from MPI_Error_string and a prefix passed in by the caller.
         * @param status - the MPI status.
         * @param prefix - prefix to the error message.
         */
        void
        CMPIRawToParametersWorker::throwMPIError(int status, const char* prefix) {
            if (status != MPI_SUCCESS) {
                std::string msg = prefix;
                int len;
                char error[MPI_MAX_ERROR_STRING];
                MPI_Error_string(status, error, &len);
                msg += error;
                throw std::runtime_error(msg);
            }                
            
        }
        
        
    }
}