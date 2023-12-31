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
    CPPUNIT_TEST(get_2);
    CPPUNIT_TEST(get_3);
    CPPUNIT_TEST(get_4);
    CPPUNIT_TEST(get_5);
    CPPUNIT_TEST(get_6);
    CPPUNIT_TEST(get_7);
    CPPUNIT_TEST(get_8);
    CPPUNIT_TEST(get_9);
    CPPUNIT_TEST(get_10);
    
    CPPUNIT_TEST(baddone);
    CPPUNIT_TEST_SUITE_END();
protected:
    void construct_1();
    void construct_2();
    void construct_3();
    
    void get_1();
    void get_2();
    void get_3();
    void get_4();
    void get_5();
    void get_6();
    void get_7();
    void get_8();
    void get_9();
    void get_10();
    
    void baddone();
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
    // Utilities:
private:    
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
// get from file with a single ring item (smaller than block):

void readertest::get_2() {
    writeCountPattern(100, 0, 1);
    lseek(m_fd, 0, SEEK_SET);               // rewind fd.
    
    CDataReader d(m_fd, 1024);
    EQ(std::size_t(100), d.m_nBytes);                   // That's what we could read.
    
    auto r = d.getBlock(1024);
    EQ(size_t(100), r.s_nbytes);
    EQ(size_t(1), r.s_nItems);
    EQ(const_cast<const void*>(d.m_pBuffer), r.s_pData);
    
    // Contents:
    
    union {
        const std::uint32_t* u_32;
        const std::uint8_t*  u_8;
    } p;
    p.u_32 = reinterpret_cast<const std::uint32_t*>(r.s_pData);
    EQ(std::uint32_t(100), *p.u_32);
    p.u_32++;
    
    for (int i =0; i < r.s_nbytes - sizeof(std::uint32_t); i++) {
        EQ(std::uint8_t(i), *p.u_8);
        p.u_8++;
    }
}
// Get without done is a logic error:

void readertest::get_3() {
    writeCountPattern(100, 0, 1);
    lseek(m_fd, 0, SEEK_SET);               // rewind fd.
    
    CDataReader d(m_fd, 1024);
    EQ(std::size_t(100), d.m_nBytes);                   // That's what we could read.
    
    auto r = d.getBlock(1024);    // ok.
    CPPUNIT_ASSERT_THROW(d.getBlock(1024), std::logic_error);
}
// get after done gives eof indication in this case:

void readertest::get_4() {
    writeCountPattern(100, 0, 1);
    lseek(m_fd, 0, SEEK_SET);               // rewind fd.
    
    CDataReader d(m_fd, 1024);
    EQ(std::size_t(100), d.m_nBytes);                   // That's what we could read.
    
    auto r = d.getBlock(1024);    // ok.
    d.done();
    CPPUNIT_ASSERT_NO_THROW(r = d.getBlock(1024));     // eof.
    EQ(size_t(0), r.s_nbytes);
    EQ(size_t(0), r.s_nItems);
    ASSERT(!r.s_pData);
}
// Can get 2 ring items if my request is big enough:

void readertest::get_5() {
    writeCountPattern(100, 0, 1);
    writeCountPattern(50, 0, 2);
    lseek(m_fd, 0, SEEK_SET);               // rewind fd.
    
    CDataReader d(m_fd, 1024);             // one gulp.
    
    auto r = d.getBlock(1024);
    EQ(size_t(150), r.s_nbytes);
    EQ(size_t(2),   r.s_nItems);
    
    // only look at the second item:
    
     union {
        const std::uint32_t* u_32;
        const std::uint8_t*  u_8;
    } p;
    p.u_32 = reinterpret_cast<const std::uint32_t*>(r.s_pData);
    p.u_8 += *p.u_32;
    EQ(std::uint32_t(50), *p.u_32);
    p.u_32++;
    for (int i =0; i < 50 - sizeof(uint32_t); i++ ) {
        EQ(int(std::uint8_t(i*2)), int(*p.u_8));
        p.u_8++;
    }
    
    
}
// need 2 gets to get both items.

void readertest::get_6() {
    // neeed 2 gets to get both items:
    
    writeCountPattern(100, 0, 1);
    writeCountPattern(50, 0, 2);
    lseek(m_fd, 0, SEEK_SET);               // rewind fd.
    
    CDataReader d(m_fd, 1024);             // one gulp.
    
    auto r = d.getBlock(110);              // NOt big enough for both.
    EQ(size_t(100), r.s_nbytes);
    EQ(size_t(1),   r.s_nItems);
    
    d.done();
    r = d.getBlock(110);
    EQ(size_t(50), r.s_nbytes);
    EQ(size_t(1),   r.s_nItems);
    union {
        const std::uint32_t* u_32;
        const std::uint8_t*  u_8;
    } p;
    p.u_32 = reinterpret_cast<const std::uint32_t*>(r.s_pData);
    EQ(std::uint32_t(50), *p.u_32);
    p.u_32++;
    for (int i =0; i < 50 - sizeof(uint32_t); i++ ) {
        EQ(int(std::uint8_t(i*2)), int(*p.u_8));
        p.u_8++;
    }
}
// reader needs two reads:

void readertest::get_7()
{
    writeCountPattern(100, 0, 1);
    writeCountPattern(50, 0, 2);
    lseek(m_fd, 0, SEEK_SET);               // rewind fd.
    
    CDataReader d(m_fd, 100);             // two gulps
    
    auto r = d.getBlock(110);              // NOt big enough for both.
    EQ(size_t(100), r.s_nbytes);
    EQ(size_t(1),   r.s_nItems);
    
    d.done();
    r = d.getBlock(110);
    EQ(size_t(50), r.s_nbytes);
    EQ(size_t(1),   r.s_nItems);
    union {
        const std::uint32_t* u_32;
        const std::uint8_t*  u_8;
    } p;
    p.u_32 = reinterpret_cast<const std::uint32_t*>(r.s_pData);
    EQ(std::uint32_t(50), *p.u_32);
    p.u_32++;
    for (int i =0; i < 50 - sizeof(uint32_t); i++ ) {
        EQ(int(std::uint8_t(i*2)), int(*p.u_8));
        p.u_8++;
    }    
}
// First gulp gets a partial:

void readertest::get_8() {
    writeCountPattern(100, 0, 1);
    writeCountPattern(50, 0, 2);
    lseek(m_fd, 0, SEEK_SET);               // rewind fd.
    
    CDataReader d(m_fd, 110);             // Partial in the first:
    
    auto r = d.getBlock(110);              // NOt big enough for both.
    EQ(size_t(100), r.s_nbytes);
    EQ(size_t(1),   r.s_nItems);
    
    d.done();
    r = d.getBlock(110);
    EQ(size_t(50), r.s_nbytes);
    EQ(size_t(1),   r.s_nItems);
    union {
        const std::uint32_t* u_32;
        const std::uint8_t*  u_8;
    } p;
    p.u_32 = reinterpret_cast<const std::uint32_t*>(r.s_pData);
    EQ(std::uint32_t(50), *p.u_32);
    p.u_32++;
    for (int i =0; i < 50 - sizeof(uint32_t); i++ ) {
        EQ(int(std::uint8_t(i*2)), int(*p.u_8));
        p.u_8++;
    }    
}
// item won't fit in user req

void readertest::get_9()
{
    writeCountPattern(100, 0, 1);
    writeCountPattern(50, 0, 2);
    lseek(m_fd, 0, SEEK_SET);               // rewind fd.
    
    CDataReader d(m_fd, 100);             // two gulps
    
    CPPUNIT_ASSERT_THROW(auto r = d.getBlock(50), std::logic_error);
    
}
// ring item can't fit in the reader's buffer:

void readertest::get_10()
{
    writeCountPattern(100, 0, 1);
    writeCountPattern(50, 0, 2);
    lseek(m_fd, 0, SEEK_SET);               // rewind fd.
    
    CDataReader d(m_fd, 50);
    CPPUNIT_ASSERT_THROW(auto r = d.getBlock(50), std::logic_error);
}
// done when released is a logic error:

void readertest::baddone() {
    CDataReader d(m_fd, 100);
    CPPUNIT_ASSERT_THROW(d.done(), std::logic_error);
}