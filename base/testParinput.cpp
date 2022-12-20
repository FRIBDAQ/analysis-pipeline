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

/** @file:  testParinput.cpp
 *  @brief: Test apps using the parameter input dealer.
 */

#include "AbstractApplication.h"
#include "MPIParameterDealer.h"
#include "ParameterReader.h"
#include "AnalysisRingItems.h"
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

// ring item types we don't have in AnalysisRingItems.h:

const std::uint32_t BEGIN_RUN(1);
const std::uint32_t END_RUN(2);
const std::uint32_t EVENT_COUNT(1000);    // events to create.

void runTests(std::string out, std::string work, int nevents);

class MyApp : public AbstractApplication {
public:
    MyApp(int argc, char** argv) : AbstractApplication(argc, argv) {}
    virtual ~MyApp() {}
    
    //
    
    virtual void dealer(int argc, char** argv, AbstractApplication* pApp);
    virtual void farmer(int argc, char** argv, AbstractApplication* pApp) {
        MPI_Barrier(MPI_COMM_WORLD);     // Not going to get data.
    }
    virtual void outputter(int argc, char** argv, AbstractApplication* pApp);
    virtual void worker(int argc, char** argv, AbstractApplication* pApp);
    
    std::string getOutputFile(int argc, char** argv);
    std::string getParameterOutputFile(int argc, char**argv);
    std::string getInputFile(int argc, char** argv);
private:
    
    void makeDataFile(const std::string& filename);
    
    
    void writeParameterDefs(int fd);
    void writeVariableDefs(int fd);
    void beginRun(int fd);
    void events(int fd);
    void endRun(int fd);
    
    
};
// Dealer Make an input file and let the dealer process it:

void
MyApp::dealer(int argc, char** argv, AbstractApplication* pApp) {
    auto filename = getInputFile(argc, argv);
    makeDataFile(filename);
    
    CMPIParameterDealer dealer(argc, argv, pApp);
    dealer();
    
    MPI_Barrier(MPI_COMM_WORLD);
}
// The outputter will get any non parameter data items pushed to it
// our worker will push an eof at it when it gets one from the dealer.
// Note we need to write those out as there's no other way to communicate
// to the testing framework if it runs in a different rank.
///
void
MyApp::outputter(int argc, char** argv, AbstractApplication* pApp) {
    std::string file = getOutputFile(argc, argv);
    int fd = creat(file.c_str(), S_IRUSR | S_IWUSR);
    if (fd < 0) {
        throw std::runtime_error("ouputter failed to open outpu file");
    }
    while(1) {
        FRIB_MPI_Parameter_MessageHeader header;
        MPI_Status status;
        int stat = MPI_Recv(
            &header, 1, pApp->parameterHeaderDataType(),
            MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status
        );
        pApp->throwMPIError(stat, "Unable to receive header in outputter");
        if (header.s_end) break;        
        
        std::unique_ptr<std::uint8_t> pcontents(new std::uint8_t[header.s_numParameters]);
        stat = MPI_Recv(
            pcontents.get(), header.s_numParameters, MPI_UINT8_T,
            status.MPI_SOURCE, MPI_DATA_TAG, MPI_COMM_WORLD, &status
        );
        pApp->throwMPIError(stat, "Unable to receive data block for passthrough");
        if (write(fd, pcontents.get(), header.s_numParameters) < 0) {
            throw std::runtime_error("Failed to write data in ouputter");
        }
    }
    
    close(fd);
    MPI_Barrier(MPI_COMM_WORLD);
    
}
/**
 * worker
 *    We open an output file and just put the data we get from the
 *    dealer into it for later examination.  Note that both the header and
 *    contents will be written.
 */
void
MyApp::worker(int argc, char** argv, AbstractApplication* pApp) {
    
    std::string file = getParameterOutputFile(argc, argv);
    int fd = creat(file.c_str(), S_IRUSR | S_IWUSR);
    if (fd < 0) {
        throw std::runtime_error("Failed to open worker output file");
    }
    
    
    // Get the definitions these are pushed:
    
    std::uint32_t numItems;
    MPI_Status status;
    int stat;
    // These block determine when the dynamic data are released.
    
    // Parameter definitions:
    {
        stat = MPI_Recv(&numItems, 1, MPI_UINT32_T, 0, MPI_PARAMDEF_TAG, MPI_COMM_WORLD, &status);
        pApp->throwMPIError(stat, "Unable to get number of parameter definitions");
        if (write(fd, &numItems, sizeof(numItems)) < 0) {
            throw std::runtime_error("Failed to write # param defs to file");
        }
        
        
        std::unique_ptr<FRIB_MPI_ParameterDef> pData(new FRIB_MPI_ParameterDef[numItems]);
        stat = MPI_Recv(
            pData.get(), numItems, pApp->parameterDefType(),
            0, MPI_PARAMDEF_TAG, MPI_COMM_WORLD, &status
        );
        pApp->throwMPIError(stat, "Could not read parameter defs");
        
        if (write(fd, pData.get(), numItems*sizeof(FRIB_MPI_ParameterDef)) < 0) {
            throw std::runtime_error("Unable to write parameter defs to file");
        }
        
        
    }
    // Variable defs/values:

    {
        
        stat = MPI_Recv(&numItems, 1, MPI_UINT32_T, 0, MPI_VARIABLES_TAG, MPI_COMM_WORLD, &status);
        pApp->throwMPIError(stat, "Unable to get number of Variable definitions");
        if (write(fd, &numItems, sizeof(numItems)) < 0) {
            throw std::runtime_error("Failed to write # variable defs to file");
        }
        
        
        std::unique_ptr<FRIB_MPI_VariableDef> pData(new FRIB_MPI_VariableDef[numItems]);
        stat = MPI_Recv(
            pData.get(), numItems, pApp->variableDefType(),
            0, MPI_VARIABLES_TAG, MPI_COMM_WORLD, &status
        );
        pApp->throwMPIError(stat, "Could not read variable def/value block");
        
        if (write(fd, pData.get(), numItems*sizeof(FRIB_MPI_VariableDef)) < 0) {
            throw std::runtime_error("Could not write variable defs");
        }
        
    }
    
    
    // Get the parameter data
    
    while (1) {
        pApp->requestData(1024*1024);
        FRIB_MPI_Parameter_MessageHeader hdr;
        stat = MPI_Recv(
            &hdr, 1, pApp->parameterHeaderDataType(),
            0, MPI_HEADER_TAG, MPI_COMM_WORLD, &status
        );
        pApp->throwMPIError(stat, "Unable to get data header");
        
        if (hdr.s_end) break;
        
        if (write(fd, &hdr, sizeof(hdr)) < 0) {
            throw std::runtime_error("Failed to write header");
        }
        
        
        std::unique_ptr<FRIB_MPI_Parameter_Value> pData(new FRIB_MPI_Parameter_Value[hdr.s_numParameters]);
        stat = MPI_Recv(
            pData.get(), hdr.s_numParameters, pApp->parameterValueDataType(),
            0, MPI_DATA_TAG, MPI_COMM_WORLD, &status
        );
        pApp->throwMPIError(stat, "Could not get data body");
        
        if (write(
            fd, pData.get(), hdr.s_numParameters*sizeof(FRIB_MPI_Parameter_Value
        )) < 0) {
            throw std::runtime_error("Unable to write payload data");
        }
        
    }
    
    close(fd);
    // normally the sorter (which is stubbed out) sends the eof to the outputter:
    
    FRIB_MPI_Parameter_MessageHeader msg;
    msg.s_triggerNumber =0;
    msg.s_numParameters = 0;
    msg.s_end = true;
    
    stat = MPI_Send(
        &msg, 1, pApp->parameterHeaderDataType(),
        2, MPI_HEADER_TAG, MPI_COMM_WORLD
    );
    pApp->throwMPIError(stat, "Unable to send eof.");
    
    
    MPI_Barrier(MPI_COMM_WORLD);
    
    // At this point everyone is done so we can run the tests.
    
    MyApp* pMyApp = dynamic_cast<MyApp*>(pApp);
    std::string outfile = pMyApp->getOutputFile(argc, argv);
    std::string parfile = file;
    
    runTests(outfile, parfile, EVENT_COUNT);
    unlink(pMyApp->getInputFile(argc, argv).c_str());
    
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

std::string
MyApp::getParameterOutputFile(int argc, char** argv) { // argv[3]
    if (argc < 4) {
        throw std::invalid_argument("Too few command arguments");
    }
    return std::string(argv[3]);
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
        s << std::setw(2) << std::setfill('0') << "array." << i;
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
        sname << std::setfill('0') << std::setw(2) << "offset." << i;
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




// We need a parameter reader.  It's not going to produce anything

class NullParameterReader : public CParameterReader {
public:
    NullParameterReader(const char* pFilename) : CParameterReader(pFilename) {}
    virtual void read() {}
};

// Create and start the app:

int main (int argc, char** argv) {
    MyApp app(argc, argv);
    NullParameterReader reader("/dev/null");
    
    app(reader);
}


std::string outputFile;
std::string workerFile;
int         numberEvents;
void runTests(std::string outfile, std::string workerfile, int count) {
    bool wasSucessful;
    outputFile = outfile;
    workerFile = workerfile;
    numberEvents  = count;
    
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
    unlink(workerFile.c_str());
    
}