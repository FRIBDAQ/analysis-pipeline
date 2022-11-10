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

/** @file:  readertests.cpp
 *  @brief: Tests CReader class.
 */
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <new>
#include <stdexcept>
#include <string>
#include <string.h>

#define private public
#include "DataReader.h"
#undef private

using namespace frib::analysis;

static const char* templateFilename="testXXXXXX.dat";

class readertest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(readertest);
    CPPUNIT_TEST(construct_1);
    CPPUNIT_TEST(construct_2);
    CPPUNIT_TEST(construct_3);
    
    CPPUNIT_TEST(get_1);
    CPPUNIT_TEST_SUITE_END();
    
private:
    int m_fd;
    std::string m_filename;
public:
    void setUp() {
        
        // Make a temporary file in which the data can be written.
        
        char ftemplate[100];
        strncpy(ftemplate, templateFilename, sizeof(ftemplate));
        m_fd = mkstemps(ftemplate, 4);     // 4 '.dat'
        if (m_fd < 0) {
            std::string failmsg = "Failed to make tempfile: ";
            failmsg += strerror(errno);
            throw std::runtime_error(failmsg);
        }
        m_filename = ftemplate;
    }
    void tearDown() {
        // CLose the tempfile and remove it...b/c/ temp files aren't atually temp.
        
        close(m_fd);      // Might have been closed in test so don't check status
        unlink(m_filename.c_str());
        
    }
protected:
    void construct_1();
    void construct_2();
    void construct_3();
    
    void get_1();
    // Utilities:
    
    void writeCountPattern(std::uint32_t nBytes, std::uint8_t start, std::uint8_t incr);
};


/////////////////////////////////////////////////////////////////////////////////////
// Utility methods:

/**
 * writeCountPattern
 *   Write a ring item with a payload that consists of a byte sized counting
 *   pattern:
 *
 * @param nBytes - total number of bytes -inluding self.
 * @param start  - Initial value of couting pattern.
 * @param incr   - increment between items.
 */
void
readertest::writeCountPattern(
    std::uint32_t nBytes, std::uint8_t start, std::uint8_t incr
) {
    // For now assume the writes will work:
    
    
    write(m_fd, &nBytes, sizeof(uint32_t));
    nBytes -= sizeof(std::uint32_t);              // Payload size.
    for (int i=0; i < nBytes; i++) {
        write(m_fd, &start, sizeof(start));
        start += incr;
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(readertest);

// Construct on empty file by name:
void readertest::construct_1()
{
    CDataReader d(m_filename.c_str(), 100);
    EQ(size_t(0), d.m_nBytes);
    ASSERT(d.m_pBuffer);
    EQ(size_t(100), d.m_nBufferSize);
    ASSERT(d.m_eof);
    ASSERT(d.m_fReleased);
       
}
// Construct on empty file by fd:

void readertest::construct_2()
{
    CDataReader d(m_fd, 100);
    EQ(m_fd, d.m_nFd);
     EQ(size_t(0), d.m_nBytes);
    ASSERT(d.m_pBuffer);
    EQ(size_t(100), d.m_nBufferSize);
    ASSERT(d.m_eof);
    ASSERT(d.m_fReleased);
    
}
// Very likely a petabyte buffer will fail:

void readertest::construct_3() {
    CPPUNIT_ASSERT_THROW(
        CDataReader d(m_fd, size_t(1024UL*1024UL*1024UL*1024UL)),
        std::bad_alloc
    );
}
// Get empty file yeilds an EOF result:

void readertest::get_1() {
    CDataReader d(m_fd, 1024);
    auto r = d.getBlock(1024);
    EQ(size_t(0), r.s_nbytes);
    EQ(size_t(0), r.s_nItems);
    ASSERT(!r.s_pData);
}