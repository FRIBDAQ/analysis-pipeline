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

/** @file:  writertests.cpp
 *  @brief: Test CDataWriter.
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

#include "DataWriter.h"
using namespace frib::analysis;

static const char* templateFilename="testXXXXXX.dat";


class writertest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(writertest);
    CPPUNIT_TEST(test_1);
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
         // CLose the tempfile and remove it...b/c/ temp files aren't atually temp

        close(m_fd);      // Might have been closed in test so don't check status
        unlink(m_filename.c_str());

    }
protected:
    void test_1();
};

CPPUNIT_TEST_SUITE_REGISTRATION(writertest);

void writertest::test_1()
{
}