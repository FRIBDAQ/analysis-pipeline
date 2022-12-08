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

/** @file:  spectclTest.cpp
 *  @brief: Full MPI integration test of SpecTcl event processor.
 */

// This test uses SpecTcl event processors to create a set of parameters that
// are a counting pattern and a sum in two stages of an event processing
// pipeline.  It is intended to be run with mpirun -n4 (one worker).

#include "AbstractApplication.h"
#include "SpecTclWorker.h"
#include "EventProcessor.h"

#include <MPIRawToParametersWorker.h>
#include <MPIParameterFarmer.h>
#include <MPIParameterOutput.h>
#include <MPITriggerSorter.h>
#include <MPIRawReader.h>
#include <TreeParameter.h>
#include <TreeParameterArray.h>
#include <ParameterReader.h>


#include <string>
#include <stdexcept>
#include <cstdint>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <mpi.h>

// For unit test support:

#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <iostream>
#include <stdexcept>

using namespace frib::analysis;

void runTests(const std::string& fname);

// This parameter reader is a dummy that just makes an array and a single parameter.
// This is run in all ranks so all ranks know the parameter indexes involved.
// Specifically the output stage knows how to write the parameter definition record.
class DummyParameterReader : public CParameterReader {
public:
    DummyParameterReader() : CParameterReader("/dev/null")  {}
        virtual void read() {
            CTreeParameterArray array ("array",16, 0); // counting pattern.
            CTreeParameter      sum("sum");            // Sum of counting pattern.
        }
    
};
// The event processors:



//    Raw event processor - fills in "array"
class Raw : public CEventProcessor {
private:
    CTreeParameterArray m_array;
    unsigned m_event;
public:
    Raw() : m_array("array", 16, 0), m_event(0) {}
    virtual Bool_t operator()(
        const Address_t pEvent, CEvent& rEvent, CAnalyzer& rAnalyzer,
        CBufferDecoder& rDecoder
    );
};
// Implement the event processor 'unpacking' code.  Just count
// in 0 - (m_event) % m_array.size() from m_event.
// Note a real unpacker would reference the ring itme body pointed to by
// pEvent just like in SpecTcl.

Bool_t
Raw::operator()(
    const Address_t pEvent, CEvent& rEvent, CAnalyzer& rAnalyzer,
    CBufferDecoder& rDecoder
) {
    unsigned last = m_event % m_array.size();
    
    for (int i =0; i < last; i++) {
        m_array[i] = m_event+i;
    }
    
    m_event++;
    return kfTRUE;
}
//     sum event processor - fills in "sum"
class Sum : public CEventProcessor {
private:
    CTreeParameterArray m_array;
    CTreeParameter m_sum;
    
public:
    Sum() : m_array("array", 16, 0), m_sum("sum") {}
    virtual Bool_t operator()(
        const Address_t pEvent, CEvent& rEvent, CAnalyzer& rAnalyzer,
        CBufferDecoder& rDecoder
    );
};
Bool_t
Sum::operator() (
     const Address_t pEvent, CEvent& rEvent, CAnalyzer& rAnalyzer,
    CBufferDecoder& rDecoder
) {
    m_sum = 0.0;
    for (int i =0; i < m_array.size(); i++) {
        if(m_array[i].isValid()) m_sum += m_array[i];
    }
    return  kfTRUE;
}
///////////////  Define the application class:

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

// We can use the packaged dealer for raw event data; though we need to
// actually build the output file:

void
Application::dealer(int argc, char** argv, AbstractApplication* pApp) {
    
    // Make the inputfile:
    
    std::string filename = getInputFilename(argc, argv);
    makeEventFile(filename);
    
    // Make and run its reader.
    
    CMPIRawReader reader(argc, argv, pApp);
    reader();
    
    // So we know when tests can be run.
    
    MPI_Barrier(MPI_COMM_WORLD);
    removeFile(filename);
    
}
// Get the filename (second word on the command line):

std::string
Application::getInputFilename(int argc, char** argv) {
    if (argc >=2) {
        return std::string(argv[1]);
    } else {
        throw std::invalid_argument("Not enough command line parameters");
    }
}

const std::uint32_t BEGIN_RUN = 1;
const std::uint32_t END_RUN   = 2;
const std::uint32_t PHYSICS_EVENT = 30;
const size_t NUM_EVENTS=10000;              // Just some number of events.

// Create a dummy event file.  Since the event file data are ignored,
// we can just create items that look like headers.
// We'll create a file that contains a BEGIN_RUN< an END_RUN and NUM_EVENTS
// PHYSICS_EVENT items.

void
Application::makeEventFile(const std::string& filename) {
    int fd = creat(filename.c_str(),  S_IRWXU);
    if (fd < 0) {
        throw std::runtime_error("Failed to create dummy event file");
    }
    
    RingItemHeader hdr;
    hdr.s_size = sizeof(hdr);
    hdr.s_type = BEGIN_RUN;
    hdr.s_unused = sizeof(hdr.s_unused);
    
    write(fd, &hdr, sizeof(hdr));
    
    hdr.s_type = PHYSICS_EVENT;
    for (int i =0; i < NUM_EVENTS; i++) {
        write(fd, &hdr, sizeof(hdr));
    }
    
    hdr.s_type = END_RUN;
    write(fd, &hdr, sizeof(hdr));
    
    close(fd);
}

void
Application::removeFile(const std::string& filename) {
    unlink(filename.c_str());
}

// The farmer can also be the packaged one:

void
Application::farmer(int argc, char** argv, AbstractApplication* pApp) {
    CMPIParameterFarmer farmer(argc, argv, *pApp);
    farmer();
    
    MPI_Barrier(MPI_COMM_WORLD);
}
// The outputter can be the packaged outputter.

void
Application::outputter(int argc, char** argv, AbstractApplication* pApp) {
    CMPIParameterOutput output;
    output(argc, argv, pApp);
    
    MPI_Barrier(MPI_COMM_WORLD);
    
    // Run unit tests on the output file which, by now is complete:
    
    std::string outFile = argv[2];
    runTests(outFile);
    
    
    // Now we could test the output file:
    
    
    removeFile(outFile.c_str());
}
// Worker:
//   Use the SpecTcl worker and build a pipeline that consists of our
//   two event processors:

void
Application::worker(int argc, char** argv, AbstractApplication* pApp) {
    CSpecTclWorker worker(*pApp);
    
    Raw raw;
    Sum sum;
    worker.addProcessor(&raw,  "Raw");
    worker.addProcessor(&sum, "Sum");
    
    worker(argc, argv);
    
    MPI_Barrier(MPI_COMM_WORLD);
}

// The main to get all this stuff started:

int main(int argc, char** argv) {
    Application app(argc, argv);
    
    DummyParameterReader reader;
    
    app(reader);
}
// Run unit tests

std::string testFile;

void runTests(const std::string& outfile) {

    testFile = outfile;
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
  if ( !wasSucessful ) {
     throw std::runtime_error("Tests failed");
  }

}