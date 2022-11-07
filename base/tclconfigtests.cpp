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

/** @file:  tclconfigtests.cpp
 *  @brief: Test the TCLParameterReader
 */
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "TCLParameterReader.h"
#define private public
#include "TreeParameter.h"
#include "TreeVariable.h"
#undef private
using namespace frib::analysis;

class TclConfigtest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(TclConfigtest);
    CPPUNIT_TEST(test_1);
    CPPUNIT_TEST_SUITE_END();
    
private:

public:
    void setUp() {
        
    }
    void tearDown() {
        CTreeParameter::m_parameterDictionary.clear();
        CTreeVariable::m_dictionary.clear();
    }
protected:
    void test_1();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TclConfigtest);

void TclConfigtest::test_1()
{
}