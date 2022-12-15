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
#include "ParmaeterReader.h"
#include "AnalysisRingItems.h"
#include <stdexcept>
#include <memory>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sstream>

using namespace frib::analysis;

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
private:
    std::string getInputFile(int argc, char** argv);
    void makeDataFile(const std::string& filename);
    std::string getOutputFile(int argc, char** argv);
    std::string getParameterOutputFile(int argc, char**argv);
    
    void writeParaqmeterDefs(int fd);
    void writeVariableDefs(int fd);
    void beginRun(int fd);
    void events(fd);
    void endRun(fd);
    
    
}
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
    int fd = creat(file.c_str(), I_RUSR | IWUSR);
    if (fd < 0) {
        throw std::runtime_error("ouputter failed to open outpu file");
    }
    while(1) {
        FRIB_MPI_Parameter_MessageHeader header;
        MPI_Status status;
        int stat = MPI_Recv(
            &header, 1, pApp->parameterHeaderType(),
            MPI_ANY_SOURCE, MPI_PASSTHROUGH_TAG, MPI_COMM_WORLD, &status
        );
        pApp->throwMPIError("Unable to receive header in outputter");
        if (header.s_end) break;        
        
        std::unique_ptr<std::uint8_t> pcontents(new std::uint8_t[header.s_numParameters]);
        stat = MPI_Recv(
            pcontents.get(), header.s_numParameters, MPI_UINT8_T,
            status.MPI_SOURCE, MPI_DATA_TAG, MPI_COMM_WORLD, &status
        );
        pApp->throwMPIError("Unable to receive data block for passthrough").
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
        stat = MPI_Recv(&numItems, 1, MPI_UINT32_t, 0, MPI_PARAMDEF_TAG, MPI_COMM_WORLD, &status)
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
        
        if (write(fd pData.get(), numItems*sizeof(FRIB_MPI_ParameterDef)) < 0) {
            throw std::runtime_error("Unable to write parameter defs to file");
        }
        
    }
    // Variable defs/values:
    {
        stat = MPI_Recv(&numItems, 1, MPI_UINT32_t, 0, MPI_PARAMDEF_TAG, MPI_COMM_WORLD, &status)
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
        FRIB_MPI_ParameterMessageHeader header;
        stat = MPI_Recv(
            &header, 1, pApp->messageHeaderType(),
            0, MPI_HEADER_TAG, MPI_COMM_WORLD, &status
        );
        pApp->throwMPIError(stat, "Unable to get data header");
    
        if (write(fd, &hdr, sizeof(hdr)) < 0) {
            throw std::runtime_error("Failed to write header");
        }
        if (hdr.s_end) break;
        
        std::unique_ptr<FRIB_MPI_Parameter_Value> pData(new FRIB_MPI_Parameter_Value[header.s_numParameters]);
        stat = MPI_Recv(
            pData.get(), header.s_numParameters, pApp->parameterValueDataType(),
            0 MPI_DATA_TAG, MPI_COMM_WORLD, &status
        );
        pApp->throwMPIError(stat, "Could not get data body");
        
        if (write(
            fd, pData.get(), header.s_numParameters*sizeof(FRIB_MPI_Parameter_Value
        )) < 0) {
            throw std::runtime_error("Unable to write payload data");
        }
        
    }
    close(fd);
    
    MPI_Barrier(MPI_COMM_WORLD);
    
    
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
MyApp::getOuputFile(int argc, char** argv) {               // argv[2]
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
    
    writeParmeterDefs(fd);
    writeVariableDefs(fd);
    
    beginRun(fd);
    events(fd);
    endRun(fd);
}
// ring item types we don't have in AnalysisRingItems.h:

const std::uint32_t BEGIN_RUN(1);
const std::uint32_t END_RUN(2);

// We'll have parameters:
//    "scalar"       - id 1
//    "array.00 - array.16" - ids 2-17

void
MyApp::writeParameterDefs(int fd) {
    // This should be big enough storage:
    
    union {
        ParamterDefinitions item;
        std::uint8_t   data[8192];    // Just storage..
    } item;
    // header.
    
    item.item.s_header.s_type = PARAMETER_DEFINITIONS
    item.item.s_header.s_size = sizeof(ParameterDefinitions);
    item.item.s_header.s_unused = sizeof(std::uint32_t);
    item.item.s_numParameters = 17;    // Magic number.
    
    // Scalar:
    union {
        ParameterDefinition* pItem 
        std::uint8_t*        p8;
    } p
    p.pItem = = reinterpret_cast<ParameterDefinition*>(&hdr + 1); // after header.
    
    p.pItem->s_parameterNumber = 1;
    strcpy(p.pItem->s_parameterName, "scalar");        // This is safe.
    std::uint32_t n = sizeof(ParameterDefinition) + strlen(p.pItem->s_parameterName) + 1;
    item.s_header.s_size += n;
    p.p8 += n;
    
    std::uint32_t index = 2;    
    for (int i =0; i < 16; i++) {
        std::stringstream s;
        s << setw(2) << setfill('0') << "array." << i;
        std::string name = s.str();
        
        p->pItem.s_parameterNumber = index;
        strcpy(p.pItem->s_parameterName, name.c_str(), name.size());
        std::uint32_t n = sizeof(ParameterDefinition) + strlen(p.pItem->s_parameterName) + 1;
        item.hdr.s_size += n;
        p.p8 += n;
        
        index++:
    }
    if (write(fd, &item, item.s_header.s_size) < 0) {
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
    
    // The scalar
    
    // The array
    
    // Write it:
    
    if (write(fd, &item, item.item._header.s_size) < 0) {
        throw std::runtime_error("Filed to write variable def/values to file");
    }
}

// We need a parameter reader.  It's not going to produce anything

// The input file:

