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

/** @file:  AbstractApplication.cpp
 *  @brief: Implement the non-pure methods of the AbstractApplication
 */
#include "AbstractApplication.h"
#include <mpi.h>
#include <stdlib.h>
#include <stdexcept>
#include "ParameterReader.h"
#include <iostream>
#include "AnalysisRingItems.h"

static const unsigned MINIMUM_SIZE(4);

namespace frib {
    namespace analysis {
        /**
         * constructor
         *   @param argc -number of command line arguments.
         *   @param argv - pointer to the command line arguments.
         */
        AbstractApplication::AbstractApplication(int argc, char** argv) :
            m_argc(argc), m_argv(argv) {}
        
        /**
         *  destructor
         */
        AbstractApplication::~AbstractApplication() {}
        /**
         * operator()
         *    Entry point to the MPI pattern.
         * @param paramReader - object that knows how to read the parameter file.
         */
        void
        AbstractApplication::operator()(CParameterReader& reader) {
            int reslen;
            char msg[MPI_MAX_ERROR_STRING];
                
            reader.read();
            int status = MPI_Init(&m_argc, &m_argv);
            if (status != MPI_SUCCESS) {
                
                MPI_Error_string(status, msg, &reslen);
                throw std::runtime_error(msg);
            }
            try {
                int rank;
                int size;
                makeDataTypes();
                status = MPI_Comm_rank(MPI_COMM_WORLD, &rank);
                if (status != MPI_SUCCESS) {
                    MPI_Error_string(status, msg, &reslen);
                    throw std::runtime_error(msg);
                }
                m_rank = rank;
                status = MPI_Comm_size(MPI_COMM_WORLD, &size);
                if (status != MPI_SUCCESS) {
                    MPI_Error_string(status, msg, &reslen);
                    throw std::runtime_error(msg);
                }
                if (size < MINIMUM_SIZE) {
                    // Only rank 0 emits the errror to stderr:
                    
                    if (rank == 0) {
                        std::cerr << "Program size: " << size << " is too small "
                            << " Minimum processes are: " << MINIMUM_SIZE
                            << std::endl;
                                                    
                    }
                    throw std::logic_error("Too few processes to run program");
                

                }
                m_nWorkers = size - 3;
                // Run in the appropriate role:
                
                 switch (rank) {
                    case 0:
                        dealer(m_argc, m_argv, this);
                        break;
                    case 1:
                        farmer(m_argc, m_argv, this);
                        break;
                    case 2:
                        outputter(m_argc, m_argv, this);
                        break;
                    default:
                        worker(m_argc, m_argv, this);
                        break;
                }
                // Finalize the application:
                
                MPI_Finalize();
                
            }
            catch (...) {
                MPI_Finalize();    // So MPI App does not hang.
                throw;
            }
            // Caller is expected to exit.
            
        }
        ////////////////////////////////  Getters //////////////////////////////
        
        /**
         * messageHeaderType
         *    Returns a reference to the MPI type item for a message header:
         */
        MPI_Datatype&
        AbstractApplication::messageHeaderType() {
            return m_messageHeaderType;
        }
        /**
         * requestDataType
         *  returns a reference to the MPI type item for a data request record.
         */
        MPI_Datatype&
        AbstractApplication::requestDataType() {
            return m_requestDataType;
        }
        /**
         * parameterHeaderDataType
         * @return MPI_DataType& reference to the header data type for
         *    a parameter message.
         */
        MPI_Datatype&
        AbstractApplication::parameterHeaderDataType() {
            return m_parameterHeaderDataType;
        }
        /**
         * parameterValueDataType
         *   @return MPI_DataType& - reference to the data type for
         *        parameter values.
         */
        MPI_Datatype&
        AbstractApplication::parameterValueDataType() {
            return m_parameterValueDataType;
        }
        /**
         * parameterDefType
         *   @return MPI_Datatype&  - references data type for parameter definition.
         */
        MPI_Datatype&
        AbstractApplication::parameterDefType() {
            return m_parameterDefDataType;
        }
        /**
         * variableDefType
         *    @return MPI_Datatype& - references the data type for variable values.
         */
        MPI_Datatype&
        AbstractApplication::variableDefType() {
            return m_variableDefDataType;
        }
        /**
         * return the number of worker processes in the application.
         * This is just size-3 (dealer, farmer, outputer).
         */
        unsigned AbstractApplication::numWorkers() {
            return m_nWorkers;    
        }
        /**
         * forwardPassThrough
         *    Send bytes without any real interpretation to the output
         * @param pData - data to send.
         * @param nBytes - number of bytes to send.
         */
        void
        AbstractApplication::forwardPassThrough(const void* pData, size_t nBytes) {
            // The data uses a parameter header but a passthrough tag:
            
            FRIB_MPI_Parameter_MessageHeader header;
            header.s_triggerNumber = 0;       // ignored.
            header.s_numParameters = nBytes;  // Actualy block size...
            header.s_end           = false;   // not an end.
            int status = MPI_Send(
                &header, 1, parameterHeaderDataType(),
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
        /////////////////////////////// Utility methods for the subclasses ////////
        
        /**
         * getArgc
         *   @return int - number of command line parameters after MPI_Init is
         *     done modifying them.
         */
        int
        AbstractApplication::getArgc() const {
            
            return m_argc;
        }
        /**
         * getArgv
         *  @return const char** - pointer to list oif argument pointers.
         */
        char**
        AbstractApplication::getArgv()  {
            return m_argv;
        }
        /**
         *  makeDataTypes
         *    Creates any MPI custom data types we need.
         *    This sets member data datatypes.  Getters exist to fetch references
         *    to the data types we've created.
         */
        void
        AbstractApplication::makeDataTypes() {
            
            // Message Header:
            
            int lengths[3] = {
                1,1, 1
            };
            MPI_Datatype types[3] = {
                MPI_INT, MPI_INT, MPI_CXX_BOOL
            };
            MPI_Aint offsets[3] = {
                offsetof(FRIB_MPI_Message_Header, s_nBytes),
                offsetof(FRIB_MPI_Message_Header, s_nBlockNum),
                offsetof(FRIB_MPI_Message_Header, s_end)
            };
            
            int status = MPI_Type_create_struct(
                3, lengths, offsets, types, &m_messageHeaderType
            );
            if (status != MPI_SUCCESS) {
                throw std::runtime_error("Unable to create message header MPI type");
            }
            
            status = MPI_Type_commit(&m_messageHeaderType);
            if (status != MPI_SUCCESS) {
                throw std::runtime_error("Unable to commit message header MPI type");
            }
            // Data Request:
            
            offsets[0]  = offsetof(FRIB_MPI_Request_Data, s_requestor);
            offsets[1]  = offsetof(FRIB_MPI_Request_Data, s_maxdata);
            
            
            status = MPI_Type_create_struct(
                2, lengths, offsets, types, &m_requestDataType
            );
            if (status != MPI_SUCCESS) {
                throw std::runtime_error("Unable to create data request  MPI type");
            }
            
            status = MPI_Type_commit(&m_requestDataType);
            if (status != MPI_SUCCESS) {
                throw std::runtime_error("Unable to commit data request MPI type");
            }
            // Parameters are preceeded by a header that has
            // the number of parameters and the trigger number:
            
            types[0] = MPI_INT64_T;
            types[1] = MPI_INT32_T;
            offsets[0] = offsetof(FRIB_MPI_Parameter_MessageHeader, s_triggerNumber);
            offsets[1] = offsetof(FRIB_MPI_Parameter_MessageHeader, s_numParameters);
            offsets[2] = offsetof(FRIB_MPI_Parameter_MessageHeader, s_end);
            
            status = MPI_Type_create_struct(
                3, lengths, offsets, types, &m_parameterHeaderDataType
            );
            if (status != MPI_SUCCESS) {
                throw std::runtime_error("Unable to create parameter message header  MPI type");
            }
            
            status = MPI_Type_commit(&m_parameterHeaderDataType);
            if (status != MPI_SUCCESS) {
                throw std::runtime_error("Unable to commit parameter message header request MPI type");
            }
            
            // Parameters themselves are number/value pairs:
            
            types[0] = MPI_INT32_T;
            types[1] = MPI_DOUBLE;
            offsets[0] = offsetof(FRIB_MPI_Parameter_Value, s_number);
            offsets[1] = offsetof(FRIB_MPI_Parameter_Value, s_value);
            
            status = MPI_Type_create_struct(
                2, lengths, offsets, types, &m_parameterValueDataType
            );
            if (status != MPI_SUCCESS) {
                throw std::runtime_error("Unable to create parameter message header  MPI type");
            }
            
            status = MPI_Type_commit(&m_parameterValueDataType);
            if (status != MPI_SUCCESS) {
                throw std::runtime_error("Unable to commit parameter message header request MPI type");
            }
            // Parameter definition:
            
            types[0] = MPI_CHAR;
            offsets[0] = offsetof(FRIB_MPI_ParameterDef, s_name);
            lengths[0] = MAX_IDENT;
            
            types[1] = MPI_UNSIGNED_LONG;
            offsets[1] = offsetof(FRIB_MPI_ParameterDef, s_parameterId);
            lengths[1]  = 1;
            
            status = MPI_Type_create_struct(
                2, lengths, offsets, types, &m_parameterDefDataType
            );
            if (status != MPI_SUCCESS) {
                throw std::runtime_error("Unable to create parameter definition MPI type");
            }
            status = MPI_Type_commit(&m_parameterDefDataType);
            if (status != MPI_SUCCESS) {
                throw std::runtime_error("Unable to commit parameter definition MPI type");
            }
            
            // Variable value:
            
            types[0] = MPI_CHAR;
            offsets[0] = offsetof(FRIB_MPI_VariableDef, s_name);
            lengths[0] = MAX_IDENT;
            
            types[1] = MPI_CHAR;
            offsets[1] = offsetof(FRIB_MPI_VariableDef, s_variableUnits);
            lengths[1] = MAX_UNITS_LENGTH;
            
            types[2] = MPI_DOUBLE;
            offsets[2] = offsetof(FRIB_MPI_VariableDef, s_value);
            lengths[2] = 1;
            
            status = MPI_Type_create_struct(
                3, lengths, offsets, types, &m_variableDefDataType
            );
            if (status != MPI_SUCCESS) {
                throw std::runtime_error("Unable to create variable value MPI type");
            }
            status = MPI_Type_commit(&m_variableDefDataType);
            if (status != MPI_SUCCESS) {
                throw std::runtime_error("Unable to commit variable value MPI type");
            }
        }
        /**
         * requestData
         *    Send a request for data to the dealer
         *  @param maxBytes - maxium payload we want to accept.
         */
        void
        AbstractApplication::requestData(size_t maxBytes) {
            FRIB_MPI_Request_Data req;
            req.s_requestor = m_rank;
            req.s_maxdata   = maxBytes;   // currently gets ignored anyway.
            
            int status = MPI_Send(
                &req, 1, requestDataType(),
                0, MPI_REQUEST_TAG, MPI_COMM_WORLD
            );
            throwMPIError(status, "Unable to send work request: ");
        }
        /**
         * throwMPIError
         *    Analyzes an MPI call status return throwing a runtime error if
         *    the status is not normal
         * @param status - status from the MPI call.
         * @param prefix - Prefix to the error text from status
         */
        void
        AbstractApplication::throwMPIError(int status, const char* prefix) {
            if (status != MPI_SUCCESS) {
                std::string msg = prefix;
                int len;
                char error[MPI_MAX_ERROR_STRING];
                MPI_Error_string(status, error, &len);
                msg += error;
                throw std::runtime_error(msg);
            }                
            
        }
        /**
         *  getRequest
         *     Receive a request from a worker and return the rank of the sender.
         *  @return int - requesting worker.
         */
        int
        AbstractApplication:: getRequest() {
            

            FRIB_MPI_Request_Data req;
            MPI_Status info;
            int status = MPI_Recv(
                &req, 1, requestDataType(), MPI_ANY_SOURCE,
                MPI_ANY_TAG, MPI_COMM_WORLD, &info
            );
            throwMPIError(status, "Failed to receive a data request: ");
            
            // Consistency check the rank in the request must be the same as
            // the one that sent us the request - note in the future,
            // this could be lifted if there's an agent that determines who
            // gets the next data item:
            
            if (req.s_requestor != info.MPI_SOURCE) {
                throw std::logic_error("Mismatch between requestor in data and actual sender");
            }
            if (info.MPI_TAG != MPI_REQUEST_TAG) {
                throw std::logic_error("Request data but not a request tag");
            }
            
            // Returning this allows later support for an agent to request
            // data on behalf of another rank.
            
            return req.s_requestor;

        }
        /**
         * sendEofs
         *    Send all the EOFS to workers.
         */
        void
        AbstractApplication::sendEofs() {
            for (int i =0; i < m_nWorkers; i++) {
                sendEof();
            }
        }
        /**
         * get a request and send an EOF to it.
         */
        void
        AbstractApplication::sendEof() {
            FRIB_MPI_Message_Header header;
            header.s_nBytes = 0;
            header.s_nBlockNum = 0;
            header.s_end = true;
            
            char errorWhy[MPI_MAX_ERROR_STRING];
            int len;
            
            int dest = getRequest();
            
            int status = MPI_Send(
                &header, 1, messageHeaderType(),
                dest, MPI_HEADER_TAG, MPI_COMM_WORLD
            );
            throwMPIError(status, "Failed to send end of data message to worker: ");
        }
        
    }

}