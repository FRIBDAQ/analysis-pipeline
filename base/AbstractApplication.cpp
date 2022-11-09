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
    }

}