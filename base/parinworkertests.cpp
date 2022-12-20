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

/** @file:  parinworkertests.cpp
 *  @brief:  Test the output file from the worker for testParinput
 */
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "AnalysisRingItems.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace frib::analysis;

extern std::string workerFile;
extern int         numberEvents;

const size_t BUFFER_SIZE = 1024*1024*32;
class parinworkertest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(parinworkertest);
    CPPUNIT_TEST(params_1);
    CPPUNIT_TEST_SUITE_END();
    
private:
    int m_fd;
    std::uint8_t*   m_contents;
    size_t         m_nBytes;
public:
    void setUp() {
        m_contents = new std::uint8_t[BUFFER_SIZE];
        m_fd = open(workerFile.c_str(), O_RDONLY);
        ASSERT(m_fd >= 0);
        m_nBytes = read(m_fd, m_contents, BUFFER_SIZE);
        ASSERT(m_nBytes > 0);
    }
    void tearDown() {
        close(m_fd);
        delete []m_contents;
    }
protected:
    void params_1();
};

CPPUNIT_TEST_SUITE_REGISTRATION(parinworkertest);

void parinworkertest::params_1()
{
}