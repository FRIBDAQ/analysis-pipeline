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

/** @file:  
 *  @brief: 
 */
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


#include "AnalysisRingItems.h"

using namespace frib::analysis;
extern std::string testFilename;

static const std::uint32_t BEGIN_RUN = 1;
static const std::uint32_t END_RUN   = 2;
static const std::uint32_t PHYSICS_EVENT= 30;

class inputtest  : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(inputtest);
    CPPUNIT_TEST(test_1);
    CPPUNIT_TEST_SUITE_END();
    
private:
    int m_fd;
public:
    void setUp() {
        m_fd = open(testFilename.c_str(), O_RDONLY);
    }
    void tearDown() {
        close(m_fd);
    }
protected:
    void test_1();
};

CPPUNIT_TEST_SUITE_REGISTRATION(inputtest);

void inputtest::test_1()
{
}