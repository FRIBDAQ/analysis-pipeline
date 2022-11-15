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

/** @file:  testOutput.cpp
 *  @brief: Test the MPIParameterOutput.cpp
 *         - Build an MPI applicationusing MPIParameterOutput as the outputter.
 *         - Send it some data and end.
 *         - Start running tests on the data file produced.
 *         
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

#include "TreeParameter.h"
#include "TreeParameterArray.h"
#include "TreeVariable.h"
#include "TreeVariableArray.h"

#include "MPIParameterOutput.h"
#include "ParameterReader.h"
#include "AnalysisRingItems.h"

// Derive our application from AbstractApplication.

using namespace frib::analysis;

// This forward def is for the function that runs the CPPUNIT tests:

static void tests(const char* fname);

// Simple
//  'parameter reader'

class DummyConfigReader : public CParameterReader {
public:
    DummyConfigReader() : CParameterReader("/dev/null") {}
    virtual void read();
};
// Actually just define some:
void
DummyConfigReader::read() {
    
    // Hardwired tree parameters.

    CTreeParameter simple("simple", 100, -1.0, 1.0, "mm");
    CTreeParameterArray a("array", "degrees", 16, 0);

    // Hardwired treevariables:
    
    CTreeVariable inc("simple-inc", 0.1, "mm");
    CTreeVariableArray amult("multipliers", 0.0, "unitless", 16, 0);
    CTreeVariableArray adds("additions", 0.0, "degrees", 16, 0);
    for (int i =0; i< 16; i++) {
        amult[i] = double(i);
        adds[i]  = double(i) *1.5;
    }
    
    
}

class Application : public AbstractApplication  {
public:
    Application(int argc, char** argv) : AbstractApplication(argc, argv) {}
    virtual void dealer(  int argc, char** argv, AbstractApplication* pApp) {
        MPI_Barrier(MPI_COMM_WORLD);
    }
    virtual void farmer(int argc, char** argv, AbstractApplication* pApp);
    virtual void outputter(int argc, char** argv, AbstractApplication* pApp);
    virtual void worker(int argc, char** argv, AbstractApplication* pApp) {
        MPI_Barrier(MPI_COMM_WORLD);
    }
private:
    std::vector<std::pair<unsigned, double>> makeEvent();
    void sendEvent(std::uint64_t trigger, const std::vector<std::pair<unsigned, double>>& event);
    void sendEnd();
};


// outputter - just instantiate the outputter  and let it run:

void
Application::outputter(int argc, char** argv, AbstractApplication* pApp) {
    CMPIParameterOutput outputter;
    outputter(argc, argv, pApp);
    
    // done.
    MPI_Barrier(MPI_COMM_WORLD);
}

// farmer
//   Generate some data for the outputter.
//   End the data.
//   Run the test suite on the output (argv[2]).
//

void
Application::farmer(int argc, char** argv, AbstractApplication* pApp) {
    for (int i =0; i < 100; i++) {
        auto event = makeEvent();
        sendEvent(i, event);
    }
    sendEnd();
    
    // This ensures everybody is done before running the tests
    //
    MPI_Barrier(MPI_COMM_WORLD);
    
    
    tests(argv[2]);
}
// Make an event - for testing this is a pattern with
// simple always set and every other array set.
// We don't recommend recreating the tree parameters for each
// call to event processing but I'm being simplistic here:
std::vector<std::pair<unsigned, double>>
Application::makeEvent() {
    
    CTreeParameter simple("simple", 100, -1.0, 1.0, "mm");
    CTreeParameterArray a("array", "degrees", 16, 0);
    
    CTreeParameter::nextEvent();
    simple = -0.5;
    for (int i =0; i < 16; i+=2) {
        a[i] = i*5;
    }
    return CTreeParameter::collectEvent();
    
}
// send an event to the output rank (rank 2)

void
Application::sendEvent(
    std::uint64_t trigger, const std::vector<std::pair<unsigned, double>>& event
) {
    MPI_Datatype& headerType(parameterHeaderDataType());
    MPI_Datatype& bodyType(parameterValueDataType());
    
    FRIB_MPI_Parameter_MessageHeader header;
    header.s_triggerNumber = trigger;
    header.s_numParameters = event.size();
    header.s_end = false;
    
    int status = MPI_Send(&header, 1, headerType, 2, MPI_HEADER_TAG, MPI_COMM_WORLD);
    if (status != MPI_SUCCESS) {
        char reason[MPI_MAX_ERROR_STRING];
        int len;
        MPI_Error_string(status, reason, &len);
        std::string msg = "Failed to send parameter header block: ";
        msg += reason;
        throw std::runtime_error(reason);
    }
    std::unique_ptr<FRIB_MPI_Parameter_Value> params(new FRIB_MPI_Parameter_Value[event.size()]);
    for (int i =0; i < event.size(); i++) {
        params.get()[i].s_number = event[i].first;
        params.get()[i].s_value  = event[i].second;
    }
    status = MPI_Send(params.get(), event.size(), bodyType, 2, MPI_DATA_TAG, MPI_COMM_WORLD);
    if (status != MPI_SUCCESS) {
        char reason[MPI_MAX_ERROR_STRING];
        int len;
        MPI_Error_string(status, reason, &len);
        std::string msg = "Failed to send parameter data block: ";
        msg += reason;
        throw std::runtime_error(reason);
    }
    
}
// send end  to the output process.

void
Application::sendEnd() {
    FRIB_MPI_Parameter_MessageHeader hdr;
    hdr.s_triggerNumber = 0;
    hdr.s_numParameters = 0;
    hdr.s_end = true;
    
    int status =
        MPI_Send(&hdr, 1, parameterHeaderDataType(), 2, MPI_END_TAG, MPI_COMM_WORLD);
    if (status != MPI_SUCCESS) {
        char reason[MPI_MAX_ERROR_STRING];
        int len;
        MPI_Error_string(status, reason, &len);
        std::string msg = "Failed to send end of data block: ";
        msg += reason;
        throw std::runtime_error(reason);
    }
}


int main(int argc, char** argv) {
    DummyConfigReader reader;
    Application app(argc, argv);
    
    app(reader);
    
}
// Run any unit tests that have been linked into this app:

std::string testFile;
static void tests(const char* fname) {
    testFile = fname;
    CppUnit::TextUi::TestRunner   
               runner; // Control tests.
    CppUnit::TestFactoryRegistry& 
               registry(CppUnit::TestFactoryRegistry::getRegistry());

    runner.addTest(registry.makeTest());
    bool wasSuccessful = runner.run("", false);
    
    unlink(testFile.c_str());         // Full teardown - kill off the file.
    
    if (!wasSuccessful) {
        throw std::runtime_error("Unit tests failed");
    }
    
}