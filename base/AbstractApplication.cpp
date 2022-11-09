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
                // Run in the appropriate role:
                
                                switch (rank) {
                    case 0:
                        dealer(m_argc, m_argv);
                        break;
                    case 1:
                        farmer(m_argc, m_argv);
                        break;
                    case 2:
                        outputter(m_argc, m_argv);
                    default:
                        worker(m_argc, m_argv);
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
            int lengths[2] = {
                1,1
            };
            MPI_Datatype types[2] = {
                MPI_INT, MPI_INT
            };
            MPI_Aint offsets[2] = {
                offsetof(FRIB_MPI_Message_Header, s_nBytes),
                offsetof(FRIB_MPI_Message_Header, s_nBlockNum)
            };
            
            int status = MPI_Type_create_struct(
                2, lengths, offsets, types, &m_messageHeaderType
            );
            if (status != MPI_SUCCESS) {
                throw std::runtime_error("Unable to create message header MPI type");
            }
            
            status = MPI_Type_commit(&m_messageHeaderType);
            if (status != MPI_SUCCESS) {
                throw std::runtime_error("Unable to commit message header MPI type");
            }
            
        }
    }

}