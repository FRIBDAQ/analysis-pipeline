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
    
    CPPUNIT_TEST(lookupdef_1);
    CPPUNIT_TEST(lookupdef_2);
    
    CPPUNIT_TEST(names_1);
    CPPUNIT_TEST(names_2);
    
    CPPUNIT_TEST(getdef_1);
    CPPUNIT_TEST(getdef_2);
    
    CPPUNIT_TEST(iter_1);
    CPPUNIT_TEST(iter_2);
    
    CPPUNIT_TEST(size_1);
    CPPUNIT_TEST(size_2);
    
    CPPUNIT_TEST(construct_1);
    CPPUNIT_TEST(construct_2);
    CPPUNIT_TEST(construct_3);
    CPPUNIT_TEST(construct_4);
    CPPUNIT_TEST(construct_5);
    CPPUNIT_TEST(construct_6);
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
    
    void lookupdef_1();
    void lookupdef_2();
    
    void names_1();
    void names_2();
    
    void getdef_1();
    void getdef_2();
    
    void iter_1();
    void iter_2();
    
    void size_1();
    void size_2();
    
    void construct_1();
    void construct_2();
    void construct_3();
    void construct_4();
    void construct_5();
    void construct_6();
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
// NOthing to see here:

void TVTest::lookupdef_1() {
    auto pDef = CTreeVariable::lookupDefinition("Not found");
    ASSERT(!pDef);
}
// Found
void TVTest::lookupdef_2() {
    auto pDef = CTreeVariable::createDefinition("test", 1.234,"furlong");
    auto another = CTreeVariable::createDefinition("junk", 3.1416, "radians");
    
    EQ(pDef, CTreeVariable::lookupDefinition("test"));
}

// empty names:
void TVTest::names_1() {
    auto v = CTreeVariable::getNames();
    ASSERT(v.empty());
}
// Some names:
void TVTest::names_2() {
    CTreeVariable::createDefinition("test4", 1.0, "");
    CTreeVariable::createDefinition("test1", 1.0, "");
    CTreeVariable::createDefinition("test3", 1.0, "");
    CTreeVariable::createDefinition("test2", 1.0, "");
    
    // We rely on the fact that tree iteration is lexically ordered:
    
    auto v = CTreeVariable::getNames();
    EQ(size_t(4), v.size());
    EQ(std::string("test1"), v[0]);
    EQ(std::string("test2"), v[1]);
    EQ(std::string("test3"), v[2]);
    EQ(std::string("test4"), v[3]);    
}
// nothing :
//
void TVTest::getdef_1() {
    auto v = CTreeVariable::getDefinitions();
    ASSERT(v.empty());
    
}
// something
void TVTest::getdef_2() {
    CTreeVariable::createDefinition("test4", 4.0, "");
    CTreeVariable::createDefinition("test1", 1.0, "");
    CTreeVariable::createDefinition("test3", 3.0, "");
    CTreeVariable::createDefinition("test2", 2.0, "");
    
    auto v = CTreeVariable::getDefinitions();
    EQ(size_t(4), v.size());
    EQ(std::string("test1"), v[0].first);
    EQ(double(1.0), v[0].second->s_value);
    
    EQ(std::string("test2"), v[1].first);
    EQ(double(2.0), v[1].second->s_value);
    
    EQ(std::string("test3"), v[2].first);
    EQ(double(3.0), v[2].second->s_value);
    
    EQ(std::string("test4"), v[3].first);
    EQ(double(4.0), v[3].second->s_value);
}

// iterate on empty:

void TVTest::iter_1()
{
    ASSERT(CTreeVariable::begin() == CTreeVariable::end());
}
// Iterate on something:

void TVTest::iter_2() {
    CTreeVariable::createDefinition("test4", 4.0, "");
    CTreeVariable::createDefinition("test1", 1.0, "");
    CTreeVariable::createDefinition("test3", 3.0, "");
    CTreeVariable::createDefinition("test2", 2.0, "");
    
    // Iteration order is lexical:
    
    std::vector<std::string> names ={"test1", "test2", "test3", "test4"};
    std::vector<double> values = {1.0, 2.0, 3.0, 4.0};
    unsigned i =0;
    for (auto p = CTreeVariable::begin(); p != CTreeVariable::end(); p++ ) {
        EQ(names[i], p->first);
        EQ(values[i], p->second.s_value);
        i++;
    }
}
// empty:
void TVTest::size_1() {
    EQ(size_t(0), CTreeVariable::size());
}
// Has 4:
void TVTest::size_2() {
    CTreeVariable::createDefinition("test4", 4.0, "");
    CTreeVariable::createDefinition("test1", 1.0, "");
    CTreeVariable::createDefinition("test3", 3.0, "");
    CTreeVariable::createDefinition("test2", 2.0, "");
    
    EQ(size_t(4), CTreeVariable::size());
}
// Default constructor.

void TVTest::construct_1() {
    CTreeVariable v;
    
    ASSERT(CTreeVariable::m_dictionary.empty());
    EQ(std::string(""), v.m_name);
    ASSERT(!v.m_pDefinition);
}
// name only construction
void TVTest::construct_2() {
    CTreeVariable v("test");
    EQ(size_t(1), CTreeVariable::m_dictionary.size());  // Entered in the dictionary.
    ASSERT(CTreeVariable::lookupDefinition("test"));    // With the right key.
    
    EQ(std::string("test"), v.m_name);
    ASSERT(v.m_pDefinition);
    EQ(CTreeVariable::lookupDefinition("test"), v.m_pDefinition);
    
    // Contents of the definition:
    
    EQ(double(0.0), v.m_pDefinition->s_value);
    EQ(std::string(""), v.m_pDefinition->s_units);
    ASSERT(!v.m_pDefinition->s_valueChanged);
    ASSERT(!v.m_pDefinition->s_definitionChanged);
}
// Construct with name and units:

void TVTest::construct_3()
{
    CTreeVariable v("test", "mm");
    
    EQ(size_t(1), CTreeVariable::m_dictionary.size());  // Entered in the dictionary.
    ASSERT(CTreeVariable::lookupDefinition("test"));    // With the right key.
    
    EQ(std::string("test"), v.m_name);
    ASSERT(v.m_pDefinition);
    EQ(CTreeVariable::lookupDefinition("test"), v.m_pDefinition);
    
    // Contents of the definition:
    
    EQ(double(0.0), v.m_pDefinition->s_value);
    EQ(std::string("mm"), v.m_pDefinition->s_units);
    ASSERT(!v.m_pDefinition->s_valueChanged);
    ASSERT(!v.m_pDefinition->s_definitionChanged);
}
// name units and value:
void TVTest::construct_4()
{
    CTreeVariable v("test", 3.1416, "mm");
    
    EQ(size_t(1), CTreeVariable::m_dictionary.size());  // Entered in the dictionary.
    ASSERT(CTreeVariable::lookupDefinition("test"));    // With the right key.
    
    EQ(std::string("test"), v.m_name);
    ASSERT(v.m_pDefinition);
    EQ(CTreeVariable::lookupDefinition("test"), v.m_pDefinition);
    
    // Contents of the definition:
    
    EQ(double(3.1416), v.m_pDefinition->s_value);
    EQ(std::string("mm"), v.m_pDefinition->s_units);
    ASSERT(!v.m_pDefinition->s_valueChanged);
    ASSERT(!v.m_pDefinition->s_definitionChanged);
}

// construct from a properties thing.
// Note the flags don't get carried along:
void TVTest::construct_5() {
    CTreeVariable::Definition def(3.1416, "mm");
    def.s_valueChanged = true;                 // Does not propagate.
    def.s_definitionChanged= true;             //   ""     ""
    CTreeVariable v("test", def);
    
    EQ(size_t(1), CTreeVariable::m_dictionary.size());  // Entered in the dictionary.
    ASSERT(CTreeVariable::lookupDefinition("test"));    // With the right key.
    
    EQ(std::string("test"), v.m_name);
    ASSERT(v.m_pDefinition);
    EQ(CTreeVariable::lookupDefinition("test"), v.m_pDefinition);
    
    // Contents of the definition:
    
    EQ(double(3.1416), v.m_pDefinition->s_value);
    EQ(std::string("mm"), v.m_pDefinition->s_units);
    ASSERT(!v.m_pDefinition->s_valueChanged);
    ASSERT(!v.m_pDefinition->s_definitionChanged);
}
// copy construction:

void TVTest::construct_6() {
    CTreeVariable v1("test", 3.1416, "mm");
    CTreeVariable v2(v1);
    
    EQ(v1.m_name, v2.m_name);
    EQ(v1.m_pDefinition, v2.m_pDefinition);
    EQ(size_t(1), CTreeVariable::m_dictionary.size());
}