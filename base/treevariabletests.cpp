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

/** @file:  treevariabletests.cpp
 *  @brief: Tests of CTreeVariable
 */
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#define private public
#include "TreeVariable.h"
#undef private

using namespace frib::analysis;

class TVTest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(TVTest);
    CPPUNIT_TEST(crdef_1);
    CPPUNIT_TEST(crdef_2);
    CPPUNIT_TEST(crdef_3);
    CPPUNIT_TEST_SUITE_END();
    
private:

public:
    void setUp() {
        
    }
    void tearDown() {
        CTreeVariable::m_dictionary.clear();        
    }
protected:
    void crdef_1();
    void crdef_2();
    void crdef_3();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TVTest);

// Create definition returns a goo def:

void TVTest::crdef_1()
{
    auto p = CTreeVariable::createDefinition("test", 1.234, "furlongs");
    
    EQ(double(1.234), p->s_value);
    EQ(std::string("furlongs"), p->s_units);
    ASSERT(!p->s_definitionChanged);
    ASSERT(!p->s_valueChanged);
}
// Create definition puts it in the dict:

void TVTest::crdef_2() {
    auto pDef = CTreeVariable::createDefinition("test", 1.234, "furlongs");
    auto p   = CTreeVariable::m_dictionary.find("test");
    ASSERT(p != CTreeVariable::m_dictionary.end());
    EQ(pDef, &(p->second));
}
// Creating an existing def gives a logic error:

void TVTest::crdef_3() {
    auto pDef = CTreeVariable::createDefinition("test", 1.234, "furlongs");
    CPPUNIT_ASSERT_THROW(
        CTreeVariable::createDefinition("test", 1.234, "furlongs"),
        std::logic_error
    );
}