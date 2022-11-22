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

/** @file:  sortTest.cpp
 *  @brief: Run the MPIParameterFarmer in the framework.
 */
#include "AbstractApplication.h"
#include "AnalysisRingItems.h"
#include "MPIParameterFarmer.h"
#include "MPIParameterOutput.h"
#include "ParameterReader.h"
#include <mpi.h>
#include <stdexcept>

#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <string>
#include <iostream>
#include <Asserts.h>
#include <unistd.h>

using namespace frib::analysis;

static void runTests(int argc, char** argv);


// This is made to run as mpirun -np 5 so that we have a pair of workers.
// Derive from AbstractApplication:

class SortTest : public AbstractApplication {
public:
    SortTest(int argc, char** argv) : AbstractApplication(argc, argv) {}
    virtual ~SortTest() {}
    
    virtual void dealer(int argc, char** argv, AbstractApplication* pApp) {
        MPI_Barrier(MPI_COMM_WORLD);
    }
    virtual void farmer(int argc, char** argv, AbstractApplication* pApp);
    virtual void outputter(int argc, char** argv, AbstractApplication* pApp);
    virtual void worker(int argc, char** argv, AbstractApplication* pApp);
private:
    void sendEvent(int trigger, AbstractApplication & app);
    void sendEnd(AbstractApplication & app);
};

/** farmer: run MPIParaqmeterFarmer then barrier:
 */
void
SortTest::farmer(int argc, char** argv, AbstractApplication* pApp) {
    CMPIParameterFarmer f(argc, argv, *pApp);
    f();
 
   MPI_Barrier(MPI_COMM_WORLD);
}
// Outputter:  Just MPIParameterOutput:
void
SortTest::outputter(int argc, char** argv, AbstractApplication* pApp) {
    CMPIParameterOutput out;
    out(argc, argv, pApp);
    
    MPI_Barrier(MPI_COMM_WORLD);
    
    // After this barrier completes we can analyze the output file:
    
    runTests(argc, argv);   
}

/** worker
 *    Two instances of this are run.  rank 3 will produce
 *    trigger 1,3, 5... - 1000 triggers.
 *    rank 4 will produce triggers 0,2,4,6 ... 1000 triggers.
 */
void
SortTest::worker(int argc, char** argv, AbstractApplication* pApp) {
    // Figure out my starting trigger number:
    
    char error[MPI_MAX_ERROR_STRING];
    int len;
    
    int rank;
    int status = MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    if (status != MPI_SUCCESS) {
        MPI_Error_string(status, error, &len);
        std::string msg = "Unable to get world rank in worker: ";
        msg += error;
        throw std::runtime_error(msg);
    }
    
    int trigger;
    switch  (rank) {
    case 3:
        trigger = 0;
        break;
    case 4:
        trigger = 1;
        break;
    default:
        std::logic_error("Unexpected rank in workers");
    }
    for (int i =0; i < 1000; i++) {
        sendEvent(trigger, *pApp);
        trigger += 2;
    }
    sendEnd(*pApp);
    
    MPI_Barrier(MPI_COMM_WORLD);
}
/**
 * sendEvent
 *    Called by the workers to send an event with a known payload and trigger.
 *    The payload is 10 consecutive parameter numbers starting from the trigger#
 *    and containing 2*trigger#.
 * @param trigger - trigger number to assign to the 'event'.
 * @note the farmer to which we snd the output is rank 1 of MPI_COMM_WORLD.
 */
void
SortTest::sendEvent(int trigger, AbstractApplication& app) {
    FRIB_MPI_Parameter_MessageHeader header;
    header.s_triggerNumber = trigger;
    header.s_numParameters = 10;
    header.s_end = false;
    
    char error[MPI_MAX_ERROR_STRING];
    int len;
    
    int status = MPI_Send(
        &header, 1, app.parameterHeaderDataType(),
        1, MPI_HEADER_TAG, MPI_COMM_WORLD
    );
    if (status != MPI_SUCCESS) {
        MPI_Error_string(status, error, &len);
        std::string msg = "Unable to send parameters header to farmer: ";
        msg += error;
        throw std::runtime_error(msg);
    }
    
    FRIB_MPI_Parameter_Value values[10];
    for (int i =0; i < 10; i++) {
        values[i].s_number = trigger+i;
        values[i].s_value  = double(trigger+i)*2.0;
    }
    
    status = MPI_Send(
        &values, 10, app.parameterValueDataType(),
        1, MPI_DATA_TAG, MPI_COMM_WORLD
    );
    if (status != MPI_SUCCESS) {
        MPI_Error_string(status, error, &len);
        std::string msg = "Unable to send parameters: ";
        msg += error;
        throw std::runtime_error(msg);
    }
}
/**
 * sendEnd
 *   Send an end item to the farmer (rank 1).
 */
void
SortTest::sendEnd(AbstractApplication& app)
{
    FRIB_MPI_Parameter_MessageHeader header;
    header.s_triggerNumber = 0;
    header.s_numParameters = 0;
    header.s_end = true;
    
    char error[MPI_MAX_ERROR_STRING];
    int len;
    
    int status = MPI_Send(
        &header, 1, app.parameterHeaderDataType(),
        1, MPI_HEADER_TAG, MPI_COMM_WORLD
    );
    if (status != MPI_SUCCESS) {
        MPI_Error_string(status, error, &len);
        std::string msg = "Unable to send parameters header to farmer: ";
        msg += error;
        throw std::runtime_error(msg);
    }
    
}

// Read parameters/variables (null).

class CDummyReader : public CParameterReader {
public:
    CDummyReader() : CParameterReader("/dev/null") {}
    virtual void read() {}
};

// entry point, create a sortTest and then exit.



int main(int argc, char** argv) {
    SortTest app(argc, argv);
    CDummyReader reader;
    app(reader);
}

std::string testFile;
/// Test runner for the output file structure.
static void runTests(int argc, char** argv) {
    testFile = argv[2];
    
    CppUnit::TextUi::TestRunner
               runner; // Control tests.
    CppUnit::TestFactoryRegistry&
                 registry(CppUnit::TestFactoryRegistry::getRegistry());
  
    runner.addTest(registry.makeTest());
  
    bool wasSucessful;
    try {
      wasSucessful = runner.run("",false);
    }
    catch(...) {
      
      wasSucessful = false;
    }
    unlink(testFile.c_str());
    if (!wasSucessful) {
        throw std::runtime_error("test failure");
    }

}