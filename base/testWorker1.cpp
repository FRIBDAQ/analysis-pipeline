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

/** @file:  testWorker1.cpp
 *  @brief: Test the worker's ability to handle input data.
 */
#include "AbstractApplication.h"
#include "MPIRawToParametersWorker.h"
#include "MPIParameterFarmer.h"
#include "MPIParameterOutput.h"
#include "MPITriggerSorter.h"
#include "MPIRawReader.h"
#include "TreeParameterArray.h"
#include "ParameterReader.h"

#include <string>
#include <stdexcept>
#include <cstdint>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

// For unit test support:

#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <iostream>
#include <stdexcept>

using namespace frib::analysis;

class DummyParameterReader : public CParameterReader {
public:
    DummyParameterReader() : CParameterReader("/dev/null") {}
    virtual void read() {
        CTreeParameterArray array("array", 16, 0);  // Registers the array.
    }
};


/**
 * dummy parameter reader that defines a tree Parameter array:
 * we're going to 'unpack' a set of parameters that depends on the
 * 'index' of the event for this worker...ignoring the actual data.
 */
class Worker : public CMPIRawToParametersWorker {
    unsigned trigger;
    CTreeParameterArray* m_pParams;
public:
    Worker(AbstractApplication& app) :
        CMPIRawToParametersWorker(app), trigger(0), m_pParams(nullptr)
    {}
    virtual ~Worker() {}
    virtual void unpackData(const void* pData) {
        if (!m_pParams) {
            m_pParams = new CTreeParameterArray("array", 16, 0);
        }

        CTreeParameterArray& array(*m_pParams);
        for (int i =0; i < trigger+1; i++) {
            array[i] = trigger;
        }
        trigger++;
        trigger = trigger % 10;
    }
    
    
};

// My worker needs to implement the unpackData method.
// We're going to ignroe 

// the application:

class Application : public AbstractApplication {
public:
    Application(int argc,char** argv) : AbstractApplication(argc, argv) {}
    virtual ~Application() {}
    
    virtual void dealer(int argc, char** argv, AbstractApplication* pApp);  // Rank 0
    virtual void farmer(int argc, char** argv, AbstractApplication* pApp);  // Rank 1
    virtual void outputter(int argc, char** argv, AbstractApplication* pApp); // Rank 2
    virtual void worker(int argc, char** argv, AbstractApplication* pApp);  // Rank 3-n.
    
    // Application utilities.
private:
    // for the dealer:
    
    std::string getInputFilename(int argc, char**argv);
    void makeEventFile(const std::string& filename);
    void removeFile(const std::string& filename);
    
    
};

// dealer - we need to create a file with a bunch of ring items....then we can read it
// to distribute it.  When we're done we also need to kill off the file (argv[1]).

void
Application::dealer(int argc, char** argv, AbstractApplication* pApp) {
    auto fname = getInputFilename(argc, argv);
    makeEventFile(fname);
    CMPIRawReader dealer(argc, argv, pApp);
    
    dealer();
    
    removeFile(fname);                      // Clean up the input file.
    
    MPI_Barrier(MPI_COMM_WORLD);            // Sync at the end of the app.
    
}
// Farmer:
void
Application::farmer(int argc, char** argv, AbstractApplication* pApp) {
    CMPIParameterFarmer farmer(argc, argv, *pApp);
    
    farmer();

    MPI_Barrier(MPI_COMM_WORLD);
}

// outputter

std::string filename;
static void tests();

void
Application::outputter(int argc, char** argv, AbstractApplication* pApp) {
    CMPIParameterOutput outputter;
    outputter(argc, argv, pApp);
    
    MPI_Barrier(MPI_COMM_WORLD);
    
    filename = argv[2];              // save for tests.
    
    tests();
    
}

//worker:


void
Application::worker(int argc, char** argv, AbstractApplication* pApp) {
    Worker worker(*pApp);
    worker(argc, argv);
    
    MPI_Barrier(MPI_COMM_WORLD);
}

//utilities:

// the input filename is argv[1].

std::string
Application::getInputFilename(int argc, char** argv) {
    if (argc < 2) {
        throw std::invalid_argument("incorrect # of command line parameters");
    }
    return argv[1];
}
// Create an event file with a minimal begin run, 10,000 minimal events
// and a minimal end run.
// The worker ignores what's in the file so we can make empty physics events.


static const std::uint32_t PHYSICS_EVENT = 30;
static const std::uint32_t BEGIN_RUN = 1;
static const std::uint32_t END_RUN = 2;

void
Application::makeEventFile(const std::string& filename) {
    int fd = creat(filename.c_str(), S_IRWXU );
    if (fd < 0) {
        throw std::runtime_error("failed to make a new event file");
    }
    
    // minimal event -- just need to change the type from time to time:
    
    RingItemHeader hdr;
    hdr.s_type = BEGIN_RUN;
    hdr.s_size = sizeof(hdr);
    hdr.s_unused= sizeof(std::uint32_t);
    
    write(fd, &hdr, sizeof(hdr));
    hdr.s_type = PHYSICS_EVENT;
    for (int i = 0; i < 10000; i++) {
        write(fd, &hdr, sizeof(hdr));
    }
    hdr.s_type = END_RUN;
    write(fd, &hdr, sizeof(hdr));
    
    close(fd);
    
}
// unlink

void
Application::removeFile(const std::string& filename) {
    unlink(filename.c_str());
}

int main(int argc, char** argv) {
    DummyParameterReader preader;
    Application app(argc, argv);
    app(preader);
    
    return 0;
    
}


// test runner for unit tests:

void tests() {
    
    CppUnit::TextUi::TestRunner
               runner; // Control tests.
    CppUnit::TestFactoryRegistry&
                 registry(CppUnit::TestFactoryRegistry::getRegistry());

    runner.addTest(registry.makeTest());

    bool wasSucessful;
    try {
      wasSucessful = runner.run("",false);
    }
    catch(std::string& rFailure) {
      std::cerr << "Caught a string exception from test suites.: \n";
      std:: cerr << rFailure << std::endl;
      wasSucessful = false;
    }
    unlink(filename.c_str());     // Remove the test output file.
    if (!wasSucessful) {
        throw std::runtime_error("Tests failed!");
    }

}


