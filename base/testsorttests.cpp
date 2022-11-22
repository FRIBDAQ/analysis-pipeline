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

/** @file:  testsorttests.cpp
 *  @brief: Tests for the output of sortTest.
 */
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <string>
#include <AnalysisRingItems.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "DataReader.h"

using namespace frib::analysis;

extern std::string testFile;
static const std::size_t BUFFER_SIZE(100*1024*1024);   // 100Mbyte buffer should do fine.
class aTestSuite : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(aTestSuite);
    CPPUNIT_TEST(contents_1);
    CPPUNIT_TEST_SUITE_END();
    
private:
    int m_fd;
public:
    void setUp() {
        m_fd = open(testFile.c_str(), O_RDONLY);
        ASSERT(m_fd >= 0);
    }
    void tearDown() {
        close(m_fd);
    }
protected:
    void contents_1();
};

CPPUNIT_TEST_SUITE_REGISTRATION(aTestSuite);

// There should be 2002 items:
// The tree variable and treeparameter entries, then 1000 data items
// per worker - 2 workers.
void aTestSuite::contents_1()
{
    CDataReader reader(testFile.c_str(), BUFFER_SIZE);
    auto info = reader.getBlock(BUFFER_SIZE);
    EQ(std::size_t(2002), info.s_nItems);
}