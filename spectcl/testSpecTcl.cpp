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

/** @file:  testSpecTcl.cpp
 *  @brief: Test contents of file made by spectclTest program.
 */
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <AnalysisRingItems.h>
#include <TreeParameter.h>
#include <TreeParameterArray.h>

using namespace frib::analysis;

extern std::string testFile;

class spectest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(spectest);
    CPPUNIT_TEST(test_1);
    CPPUNIT_TEST_SUITE_END();
    
private:

public:
    void setUp() {
        
    }
    void tearDown() {
        
    }
protected:
    void test_1();
};

CPPUNIT_TEST_SUITE_REGISTRATION(spectest);

void spectest::test_1()
{
}