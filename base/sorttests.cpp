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
        delete item;
    }
};

class sorttest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(sorttest);
    CPPUNIT_TEST(construct_1);
    CPPUNIT_TEST(construct_2);
    
    CPPUNIT_TEST(add_1);
    CPPUNIT_TEST(add_2);
    CPPUNIT_TEST(add_3);
    CPPUNIT_TEST(add_4);
    
    CPPUNIT_TEST(flush_1);
    CPPUNIT_TEST(flush_2);
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
    
    void add_1();
    void add_2();
    void add_3();
    void add_4();
    
    void flush_1();
    void flush_2();
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
// adding one ready to go will emit:
void sorttest::add_1()
{
    // only need the header really:
    
    pParameterItem pItem = new ParameterItem;
    pItem->s_header.s_size = sizeof(ParameterItem);
    pItem->s_header.s_type = PARAMETER_DATA;
    pItem->s_header.s_unused = sizeof(std::uint32_t);
    pItem->s_triggerCount = 0;
    pItem->s_parameterCount = 0;
    
    m_pSorter->addItem(pItem);
    
    EQ(size_t(1), m_pSorter->m_triggers.size());
    EQ(std::uint64_t(0), m_pSorter->m_triggers.at(0));
    
}
// out of order by one gets held until it can be released:

void sorttest::add_2()
{
    pParameterItem pItem = new ParameterItem;
    pItem->s_header.s_size = sizeof(ParameterItem);
    pItem->s_header.s_type = PARAMETER_DATA;
    pItem->s_header.s_unused = sizeof(std::uint32_t);
    pItem->s_triggerCount = 1;
    pItem->s_parameterCount = 0;
    m_pSorter->addItem(pItem);
    // can't re-use...
    
    pParameterItem pItem1 = new ParameterItem;
    pItem1->s_header.s_size = sizeof(ParameterItem);
    pItem1->s_header.s_type = PARAMETER_DATA;
    pItem1->s_header.s_unused = sizeof(std::uint32_t);
    pItem1->s_triggerCount = 0;
    pItem1->s_parameterCount = 0;
    m_pSorter->addItem(pItem1);      // both emitted.
    
    EQ(size_t(2), m_pSorter->m_triggers.size());
    EQ(std::uint64_t(0), m_pSorter->m_triggers.at(0));
    EQ(std::uint64_t(1), m_pSorter->m_triggers.at(1));
}
// Add a bunch in reverse order -- worst case.
void sorttest::add_3() {
    for (int i = 5; i >= 0; i--) {   // 6 items:
        
        pParameterItem pItem = new ParameterItem;
        pItem->s_header.s_size = sizeof(ParameterItem);
        pItem->s_header.s_type = PARAMETER_DATA;
        pItem->s_header.s_unused = sizeof(std::uint32_t);
        pItem->s_triggerCount = i;
        pItem->s_parameterCount = 0;
        m_pSorter->addItem(pItem);
    }
    // All should have been emitted
    
    EQ(size_t(6), m_pSorter->m_triggers.size());
    for (int i =0; i <= 5; i++) {
        EQ(std::uint64_t(i), m_pSorter->m_triggers.at(i));
    }
}
// 'random' order

void sorttest::add_4()
{
    std::uint64_t triggers[6] = {1, 3, 4, 5, 0, 2};    // Trigger values:
    for (int i = 5; i >= 0; i--) {   // 6 items:
        
        pParameterItem pItem = new ParameterItem;
        pItem->s_header.s_size = sizeof(ParameterItem);
        pItem->s_header.s_type = PARAMETER_DATA;
        pItem->s_header.s_unused = sizeof(std::uint32_t);
        pItem->s_triggerCount = triggers[i];
        pItem->s_parameterCount = 0;
        m_pSorter->addItem(pItem);
    }
    // All should have been emitted
    
    EQ(size_t(6), m_pSorter->m_triggers.size());
    for (int i =0; i <= 5; i++) {
        EQ(std::uint64_t(i), m_pSorter->m_triggers.at(i));
    }
    
}
// put a bunch of items in that can't be emitted...flush will:

void sorttest::flush_1()
{
    for (int i = 5; i > 0; i--) {   // 6 items:
        
        pParameterItem pItem = new ParameterItem;
        pItem->s_header.s_size = sizeof(ParameterItem);
        pItem->s_header.s_type = PARAMETER_DATA;
        pItem->s_header.s_unused = sizeof(std::uint32_t);
        pItem->s_triggerCount = i;
        pItem->s_parameterCount = 0;
        m_pSorter->addItem(pItem);
    }
    m_pSorter->flush();
    // All should have been emitted
    
    EQ(size_t(5), m_pSorter->m_triggers.size());
    for (int i =1; i <= 5; i++) {
        EQ(std::uint64_t(i), m_pSorter->m_triggers.at(i-1));
    }
}
// Destruction without flush won't call pure virts:

void sorttest::flush_2()
{
     for (int i = 5; i > 0; i--) {   // 6 items:
        
        pParameterItem pItem = new ParameterItem;
        pItem->s_header.s_size = sizeof(ParameterItem);
        pItem->s_header.s_type = PARAMETER_DATA;
        pItem->s_header.s_unused = sizeof(std::uint32_t);
        pItem->s_triggerCount = i;
        pItem->s_parameterCount = 0;
        m_pSorter->addItem(pItem);
    }
    delete m_pSorter;
    
    m_pSorter = nullptr;
}