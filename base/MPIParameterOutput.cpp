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

/** @file:  MPIParameterOutput.cpp
 *  @brief:  Implement the MPI Parameter output class.
 */
#include "MPIParameterOutput.h"
#include "AbstractApplication.h"
#include "AnalysisRingItems.h"
#include "DataWriter.h"
#include <mpi.h>
#include <string>
#include <stdexcept>
#include <memory>
#include <iostream>

namespace frib {
    namespace analysis {
        /**
         * constructor
         */
        CMPIParameterOutput::CMPIParameterOutput() :
            m_pApp(nullptr), m_pWriter(nullptr)
        {
            
        }
        /**
         * destructor -- leave the application alone but kill off the writer.
         *   that closes the output file.
         */
        CMPIParameterOutput::~CMPIParameterOutput() {
            delete m_pWriter;
        }
        /**
         * operator()
         *     Called to run the process:
         *     - Use the virtual getOutputFile to get the output filename.
         *     - Create the data writer object.
         *     - Until we get an end message from the sender (there is one),
         *       get data and write it to the m_pWriter.
         * @param argc, argv - command line arguments, used by getOutputFile.
         * @param app        - The application.  Used to get the synthetic
         *                     MPI data types.
         */
        void
        CMPIParameterOutput::operator()(
            int argc, char** argv, AbstractApplication* app
        ) {
            char errorWhy[MPI_MAX_ERROR_STRING];
            int len;
            
            // Parameter buffering:
            
            std::unique_ptr<FRIB_MPI_Parameter_Value> pData;
            size_t nParamsAllocated = 0;
            std::vector<std::pair<unsigned, double>> event;
            
            m_pApp  = app;
            auto filename = getOutputFile(argc, argv);
            m_pWriter = new CDataWriter(filename.c_str());
            FRIB_MPI_Parameter_MessageHeader header;
            header.s_end = false;
            MPI_Status mpistat;
            do {
                int status = MPI_Recv(
                    &header, 1, app->parameterHeaderDataType(),
                    MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &mpistat
                );
                if (status != MPI_SUCCESS)  {
                    std::string msg = "Failed MPI_Recv for header in parameter output : ";
                    MPI_Error_string(status, errorWhy, &len);
                    msg += errorWhy;
                    throw std::runtime_error(msg);
                }
                
                if (mpistat.MPI_TAG == MPI_HEADER_TAG) {
                    // If it's a header we have actual data:
            
                    if (header.s_numParameters > nParamsAllocated) {
                        nParamsAllocated = header.s_numParameters;
                        pData.reset(new FRIB_MPI_Parameter_Value[nParamsAllocated]);
                    }
                    
                    status = MPI_Recv(
                        pData.get(), header.s_numParameters, app->parameterValueDataType(),
                        mpistat.MPI_SOURCE, MPI_DATA_TAG, MPI_COMM_WORLD, &mpistat
                    );
                    
                    if (status != MPI_SUCCESS) {
                        std::string msg = "Failed MPI_Recv for parameter data in parameter output: ";
                        MPI_Error_string(status, errorWhy, &len);
                        msg += errorWhy;
                        throw std::runtime_error(msg);
                    }
                    // New we have the data, we can send it to the output
                    
                    event.clear();
                    event.reserve(header.s_numParameters);
                    
                    for (size_t i =0; i < header.s_numParameters; i++) {
                        event.emplace_back(
                            pData.get()[i].s_number, pData.get()[i].s_value
                        );
                
                    }
                    m_pWriter->writeEvent(event, header.s_triggerNumber);
                } else if (mpistat.MPI_TAG == MPI_PASSTHROUGH_TAG) {
                    // Passthrough item- m_numParameters is the # bytes.
                    // These are rare so we can allocate each time.
                    // Get the data item and pass it to writeItem...as the
                    // payload is assumed to be just a raw ring item.
                    //
                    std::unique_ptr<std::uint8_t> pPassThroughData(new std::uint8_t[header.s_numParameters]);
                    status = MPI_Recv(pPassThroughData.get(), header.s_numParameters, MPI_UINT8_T,
                        mpistat.MPI_SOURCE, MPI_DATA_TAG, MPI_COMM_WORLD, &mpistat       
                    );
                    if (status != MPI_SUCCESS) {
                        std::string msg = "Failed MPI_Recv for passthrough data block output: ";
                        MPI_Error_string(status, errorWhy, &len);
                        msg += errorWhy;
                        throw std::runtime_error(msg);
                    }
                    m_pWriter->writeItem(pPassThroughData.get());
                    
    
                } else if (mpistat.MPI_TAG == MPI_DATA_TAG) {
                    throw std::logic_error(
                        "CMPIParameterOutput - expected MPI Header got data"
                    );
                } else if (mpistat.MPI_TAG == MPI_END_TAG) {

                     // Do nothing - s_end will be true.
                     header.s_end = true;             // Just in case.
                     
                } else {
                    throw std::logic_error("Invalid tag type in message");
                }
                
                
            } while (!header.s_end);
            
        }
        /**
         * getOutputFile
         *    This is virtual so it can be overriden.  The naive assumption
         *    in this default implementation is that the program is invoked:
         *
         *  \varbatim
         *
         *  some-program input-file output-file.
         *  \endverbatim
         *
         */
        std::string
        CMPIParameterOutput::getOutputFile(int argc, char** argv) {
            if (argc < 3) {
                throw std::invalid_argument("Not enough command line parameters");
            }
            return argv[2];
        }
    }
}