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

/** @file:  eventtests.cppa
 *  @brief: 
 */
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#define private public
#include <TreeParameter.h>
#undef private
#include "Event.h"

using namespace frib::analysis;

class eventtest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(eventtest);
    CPPUNIT_TEST(exist_1);
    CPPUNIT_TEST(exist_2);
    CPPUNIT_TEST(exist_3);
    
    CPPUNIT_TEST(new_1);
    CPPUNIT_TEST(new_2);
    CPPUNIT_TEST(new_3);
    CPPUNIT_TEST_SUITE_END();
    
private:

public:
    void setUp() {
        
        // Empty everything.
        
        CTreeParameter::m_generation = 1;
        CTreeParameter::m_parameterDictionary.clear();
        CTreeParameter::m_nextId = 0;
        CTreeParameter::m_event.clear();
        CTreeParameter::m_scoreboard.clear();
        
    }
    void tearDown() {
        
    }
protected:
    void exist_1();
    void exist_2();
    void exist_3();
    
    void new_1();
    void new_2();
    void new_3();
};

CPPUNIT_TEST_SUITE_REGISTRATION(eventtest);

// Just can get and set values.
void eventtest::exist_1()
{
    CTreeParameter t1("t1");
    t1 = 0.0;                  // So it's valid.
    CEvent e;
    EQ(double(0.0), e[t1.getId()]);
    e[t1.getId()] = 2.0;
    EQ(double(2.0), e[t1.getId()]);
}
// metadata:
// - didn't make any new ones,
// - didn't change the scoreboard.
//
void eventtest::exist_2() {
    CTreeParameter t1("t1");
    t1 = 0.0;                  // So it's valid.
    CEvent e;
    e[t1.getId()] = 2.0;
    
    EQ(size_t(1), CTreeParameter::m_scoreboard.size());
    EQ(size_t(1), CTreeParameter::m_parameterDictionary.size());
}
// setting via array does not set valid (flaw).
void eventtest::exist_3() {
    CTreeParameter t1("t1");
    CEvent e;
    e[t1.getId()] = 2.0;
    ASSERT(!t1.isValid());
}
// Have to create a new tree parameter:

void eventtest::new_1() {
    CEvent e;
    e[0] = 1.2345;       // Made a new one.
    EQ(double(1.2345), e[0]);
    EQ(size_t(1), CTreeParameter::m_parameterDictionary.size());
    EQ(size_t(1), CTreeParameter::m_scoreboard.size());
    
}
// Created a tree parameter.

void eventtest::new_2() {
    CEvent e;
    e[0] = 1.2345;
    
    auto p = CTreeParameter::lookupParameter("_unnamed.0");
    ASSERT(p != nullptr);
    
}
// make several if needed:

void eventtest::new_3() {
    CEvent e;
    e[2] = 1.2345;
    EQ(size_t(3), CTreeParameter::m_parameterDictionary.size());
    EQ(size_t(1), CTreeParameter::m_scoreboard.size());
    double d = e[1];
    EQ(size_t(2),  CTreeParameter::m_scoreboard.size()); // peculiarity of CEvent
}