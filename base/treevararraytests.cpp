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

/** @file:  treevararraytests.cpp
 *  @brief:  Test CTreeVariableArray class
 */
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#define private public
#include "TreeVariable.h"
#include "TreeVariableArray.h"
#undef private

using namespace frib::analysis;

class TVATest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(TVATest);
    CPPUNIT_TEST(test_1);
    CPPUNIT_TEST_SUITE_END();
    
private:

public:
    void setUp() {
        
    }
    void tearDown() {
         CTreeVariable::m_dictionary.clear();
    }
protected:
    void test_1();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TVATest);

void TVATest::test_1()
{
}