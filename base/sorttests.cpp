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

/** @file:  sorttests.cpp
 *  @brief:  Tests of CTriggerSorter
 */
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#define private public
#include "TriggerSorter.h"
#undef private
#include <vector>
#include <cstdint>

using namespace frib::analysis;

struct CMyTriggerSorter : public CTriggerSorter {
    std::vector<std::uint64_t> m_triggers;
    
    CMyTriggerSorter() {}
    virtual void emitItem(pParameterItem item) {
        m_triggers.push_back(item->s_triggerCount);
    }
};

class sorttest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(sorttest);
    CPPUNIT_TEST(construct_1);
    CPPUNIT_TEST(construct_2);
    CPPUNIT_TEST_SUITE_END();
    
private:
    CMyTriggerSorter* m_pSorter;
public:
    void setUp() {
        m_pSorter = new CMyTriggerSorter;
    }
    void tearDown() {
        delete m_pSorter;
    }
protected:
    void construct_1();
    void construct_2();
};

CPPUNIT_TEST_SUITE_REGISTRATION(sorttest);
// initial state
void sorttest::construct_1()
{
    EQ(std::uint64_t(0-1), m_pSorter->m_lastEmittedTrigger);
    ASSERT(m_pSorter->m_items.empty());
    ASSERT(m_pSorter->m_triggers.empty());
}
// flush  after construction does nothing.

void sorttest::construct_2() {
    m_pSorter->flush();
    ASSERT(m_pSorter->m_triggers.empty());
    
}