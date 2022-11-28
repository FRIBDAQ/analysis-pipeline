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

/** @file:  passthruTest.cpp
 *  @brief: Test passthrough ring items.
 */
#include <AbstractApplication.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <string>
#include <iostream>
#include <vector>
#include <memory>
#include <mpi.h>
#include <unistd.h>
#include <stdlib.h>
#include <cstdint>
#include <stdexcept>
#include "AnalysisRingItems.h"
#include "TCLParameterReader.h"
#include "MPIParameterOutput.h"


using namespace frib::analysis;

/**
 * Application
 *   We're going to have rank 0 (dealer) just deal a few passthrough items
 *   directly at the Outputter and then send an end..
 *   No worker or farmers are needed.
 */
class Application : public AbstractApplication
{
public:
    Application(int argc, char** argv) : AbstractApplication(argc, argv) {}
    virtual void dealer(int argc, char** argv, AbstractApplication* pApp);
    virtual void farmer(int argc, char** argv, AbstractApplication* pApp) {
        MPI_Barrier(MPI_COMM_WORLD);            // Sync with end of app.
    }
    virtual void outputter(int argc, char** argv, AbstractApplication* pApp);
    virtual void worker(int argc, char** argv, AbstractApplication* pApp) {
        MPI_Barrier(MPI_COMM_WORLD);
    }
};

/**
 * dealer:
 *    We just create a few random passthrough ring items for the outputter to
 *    send.  They don't actually mean anything but they have known contents.
 *    and are therefore amenable to testing.
 */
void
Application::dealer(int argc, char** argv, AbstractApplication* pApp) {
    // The header type is needed the body is just sent as MPI_UINT8_T.
    
    auto headerType = parameterHeaderDataType();
    
    // A ring item:
#pragma pack(push, 1)
    struct Item {
        RingItemHeader s_header;
        std::uint8_t   s_body[100];
    } item;
#pragma pack(pop)

    item.s_header.s_size = sizeof(item);
    item.s_header.s_type = 6;                 // arbitrary.
    item.s_header.s_unused = sizeof(std::uint32_t);
    
    // the header is fixed too since the trigger number is ignored:
    
    FRIB_MPI_Parameter_MessageHeader header;
    header.s_triggerNumber = 0;
    header.s_numParameters = sizeof(item);
    header.s_end = false;
    
    // For error handling:
    
    char error[MPI_MAX_ERROR_STRING];
    int len;
    
    for (int ino = 0; ino < 100; ino++) {   // Loop over items to send:
        // FIll in a body:
        
        for (int i =0; i < 100; i++) {
            item.s_body[i] = ino + i;         //So each one is different.
        }
        
        // Send the header to the outputter.
        
        int stat = MPI_Send(
            &header, 1, headerType,
            2, MPI_PASSTHROUGH_TAG, MPI_COMM_WORLD);
        if (stat != MPI_SUCCESS) {
            MPI_Error_string(stat, error, &len);
            std::string msg("Header send failed: ");
            msg += error;
            throw std::runtime_error(msg);
        }
        
        // send the body.
        
        stat = MPI_Send(
            &item, sizeof(item), MPI_UINT8_T,
            2, MPI_DATA_TAG, MPI_COMM_WORLD
        );
        if (stat != MPI_SUCCESS) {
            MPI_Error_string(stat, error, &len);
            std::string msg("data send failed: ");
            msg += error;
            throw std::runtime_error(msg);
        }
    }
    // Send the end to the outputter so it can finish.
    
    header.s_end = true;
    
    int stat = MPI_Send(&header, 1, headerType, 2, MPI_END_TAG, MPI_COMM_WORLD);
    if (stat != MPI_SUCCESS) {
        MPI_Error_string(stat, error, &len);
        std::string msg("End  send failed: ");
        msg += error;
        throw std::runtime_error(msg);
    }
    
    // Synch the end of the app:
    
    MPI_Barrier(MPI_COMM_WORLD);
}
// The outputter instantiates an outputter and run sit.
// Then we  hit the barrier.
//
void
Application::outputter(int argc, char** argv, AbstractApplication* app) {
    CMPIParameterOutput outputTask;
    outputTask(argc, argv, app);
    
    MPI_Barrier(MPI_COMM_WORLD);
}

// main just instantioates and run our application.  Actual tests will be added
// to be run from the outputter since it will know the name of the output
// file (though the 'dealer' will know what the file looks like *sigh*).

int main(int argc, char**argv) {
    Application app(argc, argv);
    CTCLParameterReader reader("/dev/null");
    app(reader);
}