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

/** @file:  worker2tests.cpp
 *  @brief: Test the output from testWorker2.
 */
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "AnalysisRingItems.h"
#include "DataReader.h"

using namespace frib::analysis;

extern std::string outputFile;
extern unsigned numEvents;

class worker2test : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(worker2test);
    CPPUNIT_TEST(test_1);
    CPPUNIT_TEST_SUITE_END();
    
private:
    CDataReader *m_pReader;
public:
    void setUp() {
        m_pReader = new CDataReader(outputFile.c_str(), 32768); // require multigulp.
    }
    void tearDown() {
        delete m_pReader;        
    }
protected:
    void test_1();
};

CPPUNIT_TEST_SUITE_REGISTRATION(worker2test);

void worker2test::test_1()
{
}