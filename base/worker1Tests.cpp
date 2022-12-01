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

/** @file:  worker1Tests.cpp
 *  @brief: Tests for the output file created by worker 1.
 */


#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include "AnalysisRingItems.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <cstdint>

extern std::string filename;

using namespace frib::analysis;

class worker1test : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(worker1test);
    CPPUNIT_TEST(header_1);
    CPPUNIT_TEST_SUITE_END();
    
private:
    int m_fd;
    std::uint8_t* m_data;
public:
    void setUp() {
        m_fd = open(filename.c_str(), O_RDONLY);
        ASSERT(m_fd >= 0);
        // figure out how big the file is and suck it oll in
        // not too big since the items are minimal.
        
        struct stat statbuf;
        ASSERT(fstat(m_fd, &statbuf) >= 0);
        m_data = new std::uint8_t[statbuf.st_size];
        EQ(ssize_t(statbuf.st_size), read(m_fd, m_data, statbuf.st_size));
        
        //rewind the file in case a test wants to read it for itself:
        
        lseek(m_fd, 0, SEEK_SET);
        
    }
    void tearDown() {
        delete []m_data;
        close(m_fd);
    }
protected:
    void header_1();
};

CPPUNIT_TEST_SUITE_REGISTRATION(worker1test);

void worker1test::header_1()
{
}