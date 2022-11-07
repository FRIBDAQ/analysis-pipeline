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
#include <sstream>
#include <iomanip>
#include <stdexcept>

#define private public
#include "TreeVariable.h"
#include "TreeVariableArray.h"
#undef private

using namespace frib::analysis;

class TVATest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(TVATest);
    CPPUNIT_TEST(construct_1);
    CPPUNIT_TEST(construct_2); // implicitly tests Initialize.
    CPPUNIT_TEST(construct_3);
    
    CPPUNIT_TEST(index_1);
    CPPUNIT_TEST(index_2);
    
    CPPUNIT_TEST(size);
    CPPUNIT_TEST(firstIndex);
    
    CPPUNIT_TEST(assign_1);
    CPPUNIT_TEST(assign_2);
    CPPUNIT_TEST_SUITE_END();
    
private:

public:
    void setUp() {
        
    }
    void tearDown() {
         CTreeVariable::m_dictionary.clear();
    }
protected:
    void construct_1();
    void construct_2();
    void construct_3();
    
    void index_1();
    void index_2();
    
    void size();
    void firstIndex();
    
    void assign_1();
    void assign_2();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TVATest);
// default constructor:
void TVATest::construct_1()
{
    CTreeVariableArray a;
    EQ(0, a.m_nFirstIndex);
    ASSERT(a.m_TreeVariables.empty());
}
// Tree name, intial value, units, size, first index:

void TVATest::construct_2() {
    CTreeVariableArray a("test", 1.234, "mm", 16, -1);
    
    EQ(-1, a.m_nFirstIndex);
    EQ(size_t(16), a.m_TreeVariables.size());
    
    int index = -1;
    for (int i=0; i < a.m_TreeVariables.size(); i++) {
        CTreeVariable& v(*a.m_TreeVariables[i]);
        EQ(1.234, double(v));
        EQ(std::string("mm"), v.getUnit());
 
        // the name:
        
        std::stringstream strName;
        strName << "test." ;
        
        // This because width/fill operates differently between
        // sprintf and stream characters if negative numbers appear.
        
        if (index < 0) {
            strName << "-"
            << std::setfill('0') << std::setw(2) << -index;
        } else {
            strName << std::setfill('0') << std::setw(2) << index;
        }
        std::string name = strName.str();
        EQ(name, v.getName());
        
        index++;
    }
}
// copy construction:

void TVATest::construct_3() {
    CTreeVariableArray a1("test", 1.234, "mm", 16, -1);
    CTreeVariableArray a(a1);
    
    EQ(-1, a.m_nFirstIndex);
    EQ(size_t(16), a.m_TreeVariables.size());
    
    int index = -1;
    for (int i=0; i < a.m_TreeVariables.size(); i++) {
        CTreeVariable& v(*a.m_TreeVariables[i]);
        EQ(1.234, double(v));
        EQ(std::string("mm"), v.getUnit());
 
        // the name:
        
        std::stringstream strName;
        strName << "test." ;
        
        // This because width/fill operates differently between
        // sprintf and stream characters if negative numbers appear.
        
        if (index < 0) {
            strName << "-"
            << std::setfill('0') << std::setw(2) << -index;
        } else {
            strName << std::setfill('0') << std::setw(2) << index;
        }
        std::string name = strName.str();
        EQ(name, v.getName());
        
        index++;
    }
}
// valid indexing:

void TVATest::index_1() {
    CTreeVariableArray a("test", 1.234, "mm", 16, -1);
    
    for (int index =-1; index < 15; index++) {
        CTreeVariable& v = a[index];
        EQ(1.234, double(v));
        EQ(std::string("mm"), v.getUnit());
 
        // the name:
        
        std::stringstream strName;
        strName << "test." ;
        
        // This because width/fill operates differently between
        // sprintf and stream characters if negative numbers appear.
        
        if (index < 0) {
            strName << "-"
            << std::setfill('0') << std::setw(2) << -index;
        } else {
            strName << std::setfill('0') << std::setw(2) << index;
        }
        std::string name = strName.str();
        EQ(name, v.getName());
        
        index++;
    }
}
// Test invalid indexing yields std::out_of_range exception.

void TVATest::index_2() {
    CTreeVariableArray a("test", 1.234, "mm", 16, -1);
    CPPUNIT_ASSERT_THROW(a[-2], std::out_of_range);
    CPPUNIT_ASSERT_THROW(a[15], std::out_of_range);
}
// test size method:

void TVATest::size() {
    CTreeVariableArray a("test", 1.234, "mm", 16, -1);
    EQ(unsigned(16), a.size());
}

// Test firstIndex method:

void TVATest::firstIndex() {
    CTreeVariableArray a("test", 1.234, "mm", 16, -1);
    EQ(-1, a.firstIndex());
}

// good assignment of values:

void TVATest::assign_1() {
    CTreeVariableArray a("test", 1.234, "mm", 16, -1);
    CTreeVariableArray b("other", 3.1416, "mm", 16, -1);
    
    CPPUNIT_ASSERT_NO_THROW(b = a);
    for (int i = -1; i < 15; i++) {
        EQ(1.234, double(b[i]));
    }
    
}
// Various bad assignments

void TVATest::assign_2() {
    CTreeVariableArray a("test", 1.234, "mm", 16, -1);
    CTreeVariableArray b("other1", 1.111, "mm", 15, -1);   // size does not match
    CPPUNIT_ASSERT_THROW(b = a, std::invalid_argument);
    
    CTreeVariableArray c("other2", 1.555, "mm", 16, 0);   //Different first indices.
    CPPUNIT_ASSERT_THROW(c = a, std::invalid_argument);
}