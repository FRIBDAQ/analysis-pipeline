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

extern std::string filename;

using namespace frib::analysis;

class worker1test : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(worker1test);
    CPPUNIT_TEST(test_1);
    CPPUNIT_TEST_SUITE_END();
    
private:
    int m_fd;
public:
    void setUp() {
        m_fd = open(filename.c_str(), O_RDONLY);
        ASSERT(m_fd >= 0);
    }
    void tearDown() {
        close(m_fd);
    }
protected:
    void test_1();
};

CPPUNIT_TEST_SUITE_REGISTRATION(worker1test);

void worker1test::test_1()
{
}