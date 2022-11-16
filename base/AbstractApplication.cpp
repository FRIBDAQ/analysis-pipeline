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
         * return the number of worker processes in the application.
         * This is just size-3 (dealer, farmer, outputer).
         */
        unsigned AbstractApplication::numWorkers() {
            return m_nWorkers;    
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
                2, lengths, offsets, types, &m_messageHeaderType
            );
            if (status != MPI_SUCCESS) {
                throw std::runtime_error("Unable to create data request  MPI type");
            }
            
            status = MPI_Type_commit(&m_messageHeaderType);
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
            
        }
    }

}