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

/** @file:  testInput.cpp
 *  @brief: Test the input stage of an application (MPIRawREader).
 */

/**
 * This file contains a test application for the MPIRawReader.
 * It consists of a dealer that uses a raw reader to read a pre-existing
 * event file.  A single worker then makes requests of the dealer and does
 * nothing more or less than output the work items it gets without any
 * modification - which should result in a copy of the input file.
 * Since there's a single worker there's no need to reorder the data.
 * This really is just testing the messaging of MPIRawReader.
 */
#include "AbstractApplication.h"
#include "AnalysisRingItems.h"
#include "MPIRawReader.h"
#include "ParameterReader.h"
#include <mpi.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <memory>
#include <cstdint>
#include <iostream>

#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>

// Item types:

static const std::uint32_t BEGIN_RUN = 1;
static const std::uint32_t END_RUN   = 2;
static const std::uint32_t PHYSICS_EVENT= 30;

using namespace frib::analysis;

static void runTests(const std::string& file);

// We have a null parameterReader:


class NullParameters : public CParameterReader {
public:
    NullParameters() : CParameterReader("/dev/null") {}
    virtual void read() {}
};

class MyApp : public AbstractApplication {
public:
    MyApp(int argc, char** argv) : AbstractApplication(argc, argv) {}
    virtual void dealer(int argc, char** argv, AbstractApplication* pApp);
    virtual void farmer(int argc, char** argv, AbstractApplication* pApp) {
        MPI_Barrier(MPI_COMM_WORLD);
    }
    virtual void outputter(int argc, char** argv, AbstractApplication* pApp) {
        MPI_Barrier(MPI_COMM_WORLD);
    }
    virtual void worker(int argc, char** argv, AbstractApplication* pApp);
private:
    void makeInputFile(const char* filename, unsigned numitems);
    const char* getFilename(int argc, char** argv);
    void writeBegin(int fd);
    void writeEvent(int fd);
    void writeEnd(int fd);
    
    // worker utilities:
    
    bool nextItem(int fd);
    void writeItem(int fd, const void* pData,  FRIB_MPI_Message_Header& header);
    
    
    
    
};
/////////////////////////////// dealer ////////////////////////////////////////
/**
 * The dealer
 *    First we create a raw event file - then we can distribute it to the
 *    worker(s) -- run this so there's only one worker so that  the output file
 *    is faithful.
 *
 *  @param argc, argv - command line parameters
 *  @param pApp       - the application.
 */
void
MyApp::dealer(int argc, char** argv, AbstractApplication* pApp)  {
    CMPIRawReader reader(argc, argv, pApp);
    makeInputFile(getFilename(argc, argv), 1000);  // modest input file.
    
    reader();
    
    // Make a barrier so that tests are assured the file is closed:
    
    MPI_Barrier(MPI_COMM_WORLD);
}
/**
 * getFilename
 *   The filename is argv[1] -- and argc must say it's supplied:
 */
const char*
MyApp::getFilename(int argc, char** argv) {
    if (argc < 2) {
        throw std::invalid_argument("Not enough command line arguments");
    }
    return argv[1];
}
/**
 * makeInputFile
 *    Create an input file with predictable contents for the dealer to send
 *    out to its worker.  The file will be a begin run
 *    followed by some events with counting pattern bodies, followed by an end run.
 *    We're going to be minimal with the non-event data just putting in the
 *    type and no real body.
 *  @param filename - file to write to.
 *  @param numitems - number of PHYSICS_EVENT items.
 */
void
MyApp::makeInputFile(const char* filename, unsigned numItems) {
    // open the output file:
    
    int fd = creat(filename, S_IRUSR | S_IWUSR | S_IRGRP);
    if (fd < 0) {
        const char* reason = strerror(errno);
        std::string msg("Unable to create output file: ");
        msg += reason;
        throw std::runtime_error(msg);
    }
    writeBegin(fd);
    for (int i =0; i < numItems; i++) {
        writeEvent(fd);
    }
    writeEnd(fd);
    close(fd);
}
/**
 * writeBegin
 *    write a minimal begin run - should not be counted as a trigger.
 * @param fd - file descriptor to write to:
 */
void
MyApp::writeBegin(int fd) {
    RingItemHeader header;
    header.s_size = sizeof(header);
    header.s_type = BEGIN_RUN;
    header.s_unused = sizeof(std::uint32_t);
    
    auto status = write(fd, &header, sizeof(header));
    if (status != sizeof(header)) {
        const char* reason = " not fully written";
        if (status < 0) reason = strerror(errno);
        std::string msg = "Unable to write begin run item: ";
        msg += reason;
        throw std::runtime_error(msg);
    }
}
/**
 * writeEnd
 *    Write a minimal end run item:
 * @param fd - file descriptor to write to
 */
void
MyApp::writeEnd(int fd) {
    RingItemHeader header;
    header.s_size = sizeof(header);
    header.s_type = END_RUN;
    header.s_unused = sizeof(std::uint32_t);
    
    auto status = write(fd, &header, sizeof(header));
    if (status != sizeof(header)) {
        const char* reason = " not fully written";
        if (status < 0) reason = strerror(errno);
        std::string msg = "Unable to write end run item: ";
        msg += reason;
        throw std::runtime_error(msg);
    }
}
/**
 * writeEvent
 *    Write an event that consists of a counting pattern:
 * @param fd - file descriptor on which to write the event.
 */
void
MyApp::writeEvent(int fd)
{
#pragma pack(push, 1)
    struct {
        RingItemHeader  s_header;
        uint32_t        s_data[300];
    } event;
#pragma pack(pop)

    event.s_header.s_size = sizeof(event);
    event.s_header.s_type = PHYSICS_EVENT;
    event.s_header.s_unused = sizeof(std::uint32_t);
    for (int i =0; i < 300; i++) {
        event.s_data[i] = i;
    }
    
    auto status = write(fd, &event, sizeof(event));
    if (status != sizeof(event)) {
        const char* reason = " not fully written";
        if (status < 0) reason = strerror(errno);
        std::string msg = "Unable to write event item: ";
        msg += reason;
        throw std::runtime_error(msg);
    }
}

////////////////////////////// worker /////////////////////////////////////
/**
 * the worker - it's just going to get data from the
 * dealer and write it to file - note that the headers in the response are written
 * as well as the ring items so tests can look at those and be sure they're right.
 *  @param argc, argv - command line parameters
 *  @param pApp       - the application.
 */
void
MyApp::worker(int argc, char** argv, AbstractApplication* pApp)  {
    std::string outfile = getFilename(argc, argv);
    outfile += ".out";             // distinguish it.
    
    int fd;
    fd  = creat(outfile.c_str(), S_IRUSR | S_IWUSR | S_IRGRP);
    if (fd < 0) {
        const char* reason = strerror(errno);
        std::string msg = "Failed to create output file ";
        msg +=  outfile;
        msg += " : ";
        msg += reason;
        throw std::runtime_error(msg);
    }
    
    while(nextItem(fd))    // Request and process an item.
        ;
    
    
    MPI_Barrier(MPI_COMM_WORLD);   // Finish the app before going on to tests.
    runTests(outfile);
}
/**
 * nextItem
 *   Request the next item:
 *   - If the item is an end - return false.
 *   - If the item is a data - get the data and output it to file return true.
 * @param fd - file descriptor open on the output file.
 * @return bool -see above.
 * @note  really I should ask MPI for my rank but since this test program must
 * run -n 4 I know it's 3:
 */
bool
MyApp::nextItem(int fd) {
    FRIB_MPI_Request_Data request;
    request.s_requestor =  3;    // My rank.
    request.s_maxdata   = 1024;  //Ignored anyway.
    
    // for MPI_Error_string:
    
    char errorReason[MPI_MAX_ERROR_STRING];
    int len;
    
    int status = MPI_Send(
            &request, 1, requestDataType(), 0, MPI_REQUEST_TAG,
            MPI_COMM_WORLD
    );
    if (status != MPI_SUCCESS) {
        MPI_Error_string(status, errorReason, &len);
        std::string msg("Failed to send request for data: ");
        msg += errorReason;
        throw std::runtime_error(msg);
    }
    // The response to this request should be a header and a data item:
    
    FRIB_MPI_Message_Header hdr;
    MPI_Status mpistat;
    status = MPI_Recv(
        &hdr, 1, messageHeaderType(), 0, MPI_HEADER_TAG, MPI_COMM_WORLD,
        &mpistat
    );
    if (status != MPI_SUCCESS) {
        MPI_Error_string(status, errorReason, &len);
        std::string msg("Failed to receive header: ");
        msg += errorReason;
        throw std::runtime_error(msg);
    }
    if (hdr.s_end) return false;    // all done
    // How much data do we need:
    
    std::unique_ptr<char> pData(new char[hdr.s_nBytes]);
    status = MPI_Recv(
        pData.get(), hdr.s_nBytes, MPI_UINT8_T, 0, MPI_DATA_TAG, MPI_COMM_WORLD,
        &mpistat
    );
    writeItem(fd, pData.get(), hdr);
    return true;
}
/**
 *  Write a header and data item to file:
 *  @fd - the file descripter open on the file.
 *  @param pData- pointer to the data item.
 *  @param header - reference to the header which is also written.
 *  @note since we're writing to file most likely we don't have to worry about
 *      chunking the data in multiple writes.
 */
void
MyApp::writeItem(int fd, const void* pData,  FRIB_MPI_Message_Header& header) {
    ssize_t n = write(fd, &header, sizeof(header));
    if (n != sizeof(header)) {
        const char* reason = " full item not written";
        if (n < 0) {
            reason = strerror(errno);
        }
        std::string msg("Write of header to file failed or incomplete: " );
        msg += reason;
        throw std::runtime_error(msg);
    }
    n = write(fd, pData, header.s_nBytes);
    if (n != header.s_nBytes) {
        const char* reason = " full item not written";
        if (n < 0) {
            reason = strerror(errno);
        }
        std::string msg("Write of data block to file failed or incomplete: " );
        msg += reason;
        throw std::runtime_error(msg);
    }
}

// Get it all going:

int main(int argc, char** argv) {
    MyApp app(argc, argv);
    NullParameters parReader;
    
    app(parReader);
}
// run unit tests:

std::string testFilename;
using namespace std;

static void runTests(const std::string& file) {
    testFilename = file;
      CppUnit::TextUi::TestRunner   
               runner; // Control tests.
    CppUnit::TestFactoryRegistry& 
                 registry(CppUnit::TestFactoryRegistry::getRegistry());
  
    runner.addTest(registry.makeTest());
  
    bool wasSucessful;
    try {
      wasSucessful = runner.run("",false);
    } 
    catch(string& rFailure) {
      cerr << "Caught a string exception from test suites.: \n";
      cerr << rFailure << endl;
      wasSucessful = false;
    }
    if (!wasSucessful) {
        throw std::runtime_error("Tests failed!");
    }
   
}

