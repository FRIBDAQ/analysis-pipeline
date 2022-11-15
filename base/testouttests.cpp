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

/** @file:  testouttests.cpp
 *  @brief: Tests for the success of the testOutput parallel program.
 */
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "TreeParameter.h"
#include "TreeVariable.h"
#include "AnalysisRingItems.h"
#include "DataReader.h"

#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

using namespace frib::analysis;

extern std::string testFile;

class outputtest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(outputtest);
    CPPUNIT_TEST(test_1);
    CPPUNIT_TEST_SUITE_END();
    
private:
    int m_fd;               // File open on the test output file.
    CDataReader* m_pReader;
public:
    void setUp() {
        m_fd = open(testFile.c_str(), O_RDONLY);
        if (m_fd < 0) {
            const char* reason = strerror(errno);
            std::string msg = "Test file open failed: ";
            msg += reason;
            CPPUNIT_FAIL(msg);
        }
        m_pReader = new CDataReader(m_fd, 10*1024*1024);   // Sufficient to read all.
    }
    void tearDown() {
        close(m_fd);
        delete m_pReader;
    }
protected:
    void test_1();
};

CPPUNIT_TEST_SUITE_REGISTRATION(outputtest);

// Should have 102 items:
void outputtest::test_1()
{
    auto r = m_pReader->getBlock(10*1024*1024);
    m_pReader->done();
    EQ(std::size_t(102), r.s_nItems);
}