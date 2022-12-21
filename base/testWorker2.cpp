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

/** @file:  testWorker2.cpp
 *  @brief: Tests for parameters --> parameters worker(s).
 */
// Note this can be run with parallel workers.


#include "AbstractApplication.h"
#include "MPIParameterDealer.h"
#include "MPIParametersToParametersWorker.h"
#include "MPIParameterFarmer.h"
#include "MPIParameterOutput.h"
#include "ParameterReader.h"
#include "AnalysisRingItems.h"
#include "TreeParameter.h"
#include "TreeParameterArray.h"


#include <stdexcept>
#include <memory>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <string.h>
#include <stdlib.h>

#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>

using namespace frib::analysis;

const std::uint32_t BEGIN_RUN(1);
const std::uint32_t END_RUN(2);
const std::uint32_t EVENT_COUNT(10000);

// MyWorker derived from CMPIParametersToParmaetersWorker to
// Do some simple, predictable processing of the input data.

class MyWorker : public CMPIParametersToParametersWorker {
    // Input tree parameters:
    
    CTreeParameter      scalar;
    CTreeParameterArray array;
    
    // Output tree parameters:
    
    CTreeParameter      doubled;       // scaler*2
    CTreeParameter      sum;           // sum(array)
    
public:
    MyWorker(int argc, char** argv, AbstractApplication* pApp);
    virtual ~MyWorker();
    
    virtual void process();
};
// construct the parameters -- note the ids will have been set by 'reading'
// the input file.

MyWorker::MyWorker(int argc, char** argv, AbstractApplication* pApp) :
    CMPIParametersToParametersWorker(argc, argv, pApp),
    scalar("scalar"), array("array", 16, 0), doubled("doubled"), sum("sum")
    {}
MyWorker::~MyWorker() {}

void
MyWorker::process() {
    if (scalar.isValid()) doubled = scalar*2.0;
    
    sum = 0.0;
    for (int i =0; i < 16; i++) {
        if (array[i].isValid()) sum += array[i];
    }
}


 // My application is, for the most part 'normal' for parameter to paraneter *but*
 // we need to create the test input file.,
 
 
 class MyApp : public AbstractApplication {
 public:
    MyApp(int argc, char** argv);
    virtual ~MyApp();
    
    virtual void dealer(int argc, char** argv, AbstractApplication* pApp);
    virtual void farmer(int argc, char** argv, AbstractApplication* pApp) ;
    virtual void outputter(int argc, char** argv, AbstractApplication* pApp);
    virtual void worker(int argc, char** argv, AbstractApplication* pApp);
    
    std::string getOutputFile(int argc, char** argv);
    std::string getInputFile(int argc, char** argv);
private:
    
    void makeDataFile(const std::string& filename);
    void writeParameterDefs(int fd);
    void writeVariableDefs(int fd);
    void beginRun(int fd);
    void events(int fd);
    void endRun(int fd);
    
    
 };
 // Implement MyApp::
 
 MyApp::MyApp(int argc, char** argv) : AbstractApplication(argc, argv) {}
 MyApp::~MyApp() {}
 
 // the dealer is just a normal MPIParameterDealer -- but we need to make
 // the input file:
 
 void
 MyApp::dealer(int argc, char** argv, AbstractApplication* pApp) {
    
    auto filename = getInputFile(argc, argv);
    makeDataFile(filename);
    
    CMPIParameterDealer dealer(argc, argv, pApp);
    dealer();
    
    MPI_Barrier(MPI_COMM_WORLD);
 }
 
 // the worker is our specialized worker:
 
 void
 MyApp::worker(int argc, char** argv, AbstractApplication* pApp) {
    MyWorker worker(argc, argv, pApp);
    worker();
    MPI_Barrier(MPI_COMM_WORLD);
 }
 
 // The farmer is just the ordinary MPI Parameter Farmer.
 
 void
 MyApp::farmer(int argc, char** argv, AbstractApplication* pApp) {
    CMPIParameterFarmer farmer(argc, argv, *pApp);
    farmer();
    
    MPI_Barrier(MPI_COMM_WORLD);
 }
 // The ouputter is just the parameter outputter:
 
 
 void runTests(const char* outputFile, unsigned numEvents);
 
 void
 MyApp::outputter(int argc, char** argv, AbstractApplication* pApp) {
    CMPIParameterOutput outputter;
    outputter(argc, argv, pApp);
    
    MPI_Barrier(MPI_COMM_WORLD);
    
    // Everything should have run down once this barrier is cleared so:
    
    runTests(argv[2], EVENT_COUNT);
    unlink(getInputFile(argc, argv).c_str());
 }
 // Utilitie methods for MyApp:

std::string
MyApp::getInputFile(int argc, char** argv)   { // argv[1]
    if (argc < 2) {
        throw std::invalid_argument("Too few command arguments");
    }
    return std::string(argv[1]);
}

std::string
MyApp::getOutputFile(int argc, char** argv) {               // argv[2]
    if (argc < 3) {
        throw std::invalid_argument("Too few command arguments");
    }
    return std::string(argv[2]);
    
}


// The input file:

// Make the data file.
// We'll make a few parameter defs and variable defs in the output file.
// We'll make a minimal begin run (passthrough)
// A passel of parameter records.
// A minimal end run (passthrough)

void
MyApp::makeDataFile(const std::string& filename) {
    int fd = creat(filename.c_str(), S_IRUSR | S_IWUSR);
    if (fd < 0) {
        throw std::runtime_error("Failed to creat input file");
    }
    
    writeParameterDefs(fd);
    writeVariableDefs(fd);
    
    beginRun(fd);
    events(fd);
    endRun(fd);
}


// We'll have parameters:
//    "scalar"       - id 1
//    "array.00 - array.16" - ids 2-17

void
MyApp::writeParameterDefs(int fd) {
    // This should be big enough storage:
    
    union {
        ParameterDefinitions item;
        std::uint8_t   data[8192];    // Just storage..
    } item;
    // header.
    
    item.item.s_header.s_type = PARAMETER_DEFINITIONS;
    item.item.s_header.s_size = sizeof(ParameterDefinitions);
    item.item.s_header.s_unused = sizeof(std::uint32_t);
    item.item.s_numParameters = 17;    // Magic number.
    
    // Scalar:
    union {
        ParameterDefinition* pItem;
        std::uint8_t*        p8;
    } p;
    p.pItem  = item.item.s_parameters; // after header.
    
    p.pItem->s_parameterNumber = 1;
    strcpy(p.pItem->s_parameterName, "scalar");        // This is safe.
    std::uint32_t n = sizeof(ParameterDefinition) + strlen(p.pItem->s_parameterName) + 1;
    item.item.s_header.s_size += n;
    p.p8 += n;
    
    std::uint32_t index = 2;    
    for (int i =0; i < 16; i++) {
        std::stringstream s;
        s  << "array." << std::setw(2) << std::setfill('0') << i;
        std::string name = s.str();
        
        p.pItem->s_parameterNumber = index;
        strcpy(p.pItem->s_parameterName, name.c_str());
        std::uint32_t n = sizeof(ParameterDefinition) + strlen(p.pItem->s_parameterName) + 1;
        item.item.s_header.s_size += n;
        p.p8 += n;
        
        index++;
    }
    if (write(fd, &item, item.item.s_header.s_size) < 0) {
        throw std::runtime_error("Failed to write the parameter definitions");    
    }
}
// Variables - scalar item "slope" 16 array item offset.00-offset.16.
//  Slope value is 3.1416  - offsets are that plus index.
void
MyApp::writeVariableDefs(int fd) {
    union {
        VariableItem  item;
        std::uint8_t  buffer[8192];
    } item;
    
    item.item.s_header.s_type = VARIABLE_VALUES;
    item.item.s_header.s_size = sizeof(VariableItem);
    item.item.s_header.s_unused = sizeof(std::uint32_t);
    item.item.s_numVars = 17;
    
    // The scalar
    
    pVariable p = item.item.s_variables;
    p->s_value = 1.234;
    strcpy(p->s_variableUnits,"unitless");
    strcpy(p->s_variableName, "slope");
    
    
    
    // The array
    
    for (int i=0; i < 16; i++) {
        // Point to next variable:
        
        size_t len = sizeof(Variable) + strlen(p->s_variableName) +1;
        std::uint8_t* p8 = reinterpret_cast<std::uint8_t*>(p);
        p8 += len;
        item.item.s_header.s_size += len;
        p   = reinterpret_cast<pVariable>(p8);
        
        p->s_value = i;
        strcpy(p->s_variableUnits, "mm");
        std::stringstream sname;
        sname  << "offset." << std::setfill('0') << std::setw(2) << i;
        std::string name = sname.str();
        
        strcpy(p->s_variableName, name.c_str());
    }
    // Include size of last variable:
    
    item.item.s_header.s_size += sizeof(Variable) + strlen(p->s_variableName) +1;
    
    // Write it:
    
    if (write(fd, &item, item.item.s_header.s_size) < 0) {
        throw std::runtime_error("Failed to write variable def/values to file");
    }
}
// Write a *minimal* begin run - this is just going to be the header:

void
MyApp::beginRun(int fd) {
    RingItemHeader item;
    item.s_size = sizeof(item);
    item.s_type = BEGIN_RUN;
    item.s_unused = sizeof(std::uint32_t);
    
    if (write(fd, &item, sizeof(item)) < 0) {
        throw std::runtime_error("Failed to write a begin run item");
    }
}
// Write minimal end run.

void
MyApp::endRun(int fd) {
    RingItemHeader item;
    item.s_size = sizeof(item);
    item.s_type = END_RUN;
    item.s_unused = sizeof(std::uint32_t);
    
    if (write(fd, &item, sizeof(item)) < 0) {
        throw std::runtime_error("Failed to write a end run item");
    }
}

// Write some parameter events.
// parameters are in the range 1-17.  Contents are predicable/boring.
// as are the set of parameters - that's what makes this testable.

void
MyApp::events(int fd) {
    union {
        ParameterItem  item;
        std::uint8_t   buffer[8192];      // for the data.
    } item;
    
    // Commmon header item contents:
    
    item.item.s_header.s_type = PARAMETER_DATA;
    item.item.s_header.s_unused= sizeof(std::uint32_t);
    
    for (int i =0; i < EVENT_COUNT; i++) {
        // select number of parameters:
        
        unsigned numParams = i % 16 + 1;   // [1-17] range.
        
        item.item.s_header.s_size =
            sizeof(ParameterItem) + numParams*sizeof(ParameterValue);
        item.item.s_triggerCount = i;
        item.item.s_parameterCount = numParams;
        pParameterValue pPar = item.item.s_parameters;
        for (int p= 0; p < numParams; p++) {
            pPar->s_number = p;
            pPar->s_value  = p*10;
            pPar++;
        }
        if (write(fd, &item, item.item.s_header.s_size) < 0)  {
            throw std::runtime_error("Could not write event");
        }
    }
}
// A reader will define the scalar, array, doubled and sum params in a way that
// forces mapping:

class MyReader : public CParameterReader {
public:
    MyReader() : CParameterReader("/dev/null") {}
    virtual void read() {
        // Just make the tree parameters in an order that will require mapping:
        
        CTreeParameter s("sum");
        CTreeParameter d("doubled");
        CTreeParameter sc("scalar");
        CTreeParameterArray a("array", 16, 0);
    }
};

 
 // Main

int main(int argc, char** argv) {
    MyApp app(argc, argv);
    MyReader reader;
    app(reader);
}

// Fire up the unit tests for the contents of the output file:

std::string outputFile;
unsigned    numEvents;

void
runTests(const char* fname, unsigned evts) {
    outputFile = fname;
    numEvents = evts;
    bool wasSucessful;
    
    CppUnit::TextUi::TestRunner
               runner; // Control tests.
    CppUnit::TestFactoryRegistry&
                 registry(CppUnit::TestFactoryRegistry::getRegistry());
  
    runner.addTest(registry.makeTest());
  
    try {
      wasSucessful = runner.run("",false);
    }
    catch(...) {
      wasSucessful = false;
    }
    if (!wasSucessful) {
      throw std::runtime_error("Tests threw a caught exception");
    }
    unlink(outputFile.c_str());   
}