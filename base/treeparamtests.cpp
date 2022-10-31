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

/** @file:  treeparamtests.cpp
 *  @brief: Test of CTreeParameter
 */
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#define private public
#include "TreeParameter.h"
#undef private


using namespace frib::analysis;

class TPTest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(TPTest);
    CPPUNIT_TEST(initial);
    CPPUNIT_TEST(next_1);
    CPPUNIT_TEST(next_2);
    CPPUNIT_TEST(next_3);
    CPPUNIT_TEST(next_4);
    
    CPPUNIT_TEST(collect_1);
    CPPUNIT_TEST(collect_2);
    
    CPPUNIT_TEST(dlimits);
    CPPUNIT_TEST(dbins);
    CPPUNIT_TEST(dunits);
    
    CPPUNIT_TEST(getEvent);
    CPPUNIT_TEST(getsb);
    CPPUNIT_TEST_SUITE_END();
    
private:

public:
    void setUp() {
        
    }
    void tearDown() {
        CTreeParameter::m_parameterDictionary.clear();
        CTreeParameter::m_scoreboard.clear();
        CTreeParameter::m_generation = 1;
        CTreeParameter::m_nextId = 0;
        CTreeParameter::m_event.clear();
        CTreeParameter::m_defaultSpecification = {
            .s_low = 0,                 // Will need updating if it
            .s_high = 100,              // changes in TreeParameter.cpp
            .s_chans = 100,
            .s_units = "Chans"
        };
    }
protected:
    void initial();
    
    void next_1();
    void next_2();
    void next_3();
    void next_4();
    
    void collect_1();
    void collect_2();
    
    void dlimits();
    void dbins();
    void dunits();
    
    void getEvent();
    void getsb();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TPTest);

// initial state of static stuff.
void TPTest::initial()
{
    EQ(std::uint64_t(1), CTreeParameter::m_generation);
    ASSERT(CTreeParameter::m_parameterDictionary.empty());
    EQ(unsigned(1), CTreeParameter::m_nextId);
    ASSERT(CTreeParameter::m_event.empty());
    ASSERT(CTreeParameter::m_scoreboard.empty());
    
    // these need to be updated if the initial default values
    // change.
    
    EQ(double(0), CTreeParameter::m_defaultSpecification.s_low);
    EQ(double(100), CTreeParameter::m_defaultSpecification.s_high);
    EQ(unsigned(100), CTreeParameter::m_defaultSpecification.s_chans);
    EQ(std::string("Chans"), CTreeParameter::m_defaultSpecification.s_units);
}
// nextEvent increments  the generation:

void TPTest::next_1() {
    CTreeParameter::nextEvent();
    EQ(std::uint64_t(2), CTreeParameter::m_generation);
    
}
// next event does not affect empties the parameter dictionary.

void TPTest::next_2() {
    CTreeParameter::m_parameterDictionary[std::string("test")] = CTreeParameter::m_defaultSpecification;
    CTreeParameter::nextEvent();
    
    EQ(size_t(1), CTreeParameter::m_parameterDictionary.size());
}
// next event does not modify m_nextId

void TPTest::next_3() {
    auto old = CTreeParameter::m_nextId;
    CTreeParameter::nextEvent();
    
    EQ(old, CTreeParameter::m_nextId);
}
// next event clears the scoreboard and does not touch the event:

void TPTest::next_4() {
    CTreeParameter::m_scoreboard.push_back(1);
    CTreeParameter::m_scoreboard.push_back(2);
    CTreeParameter::m_event.push_back(1234.5);
    CTreeParameter::m_event.push_back(3.1416);
    
    CTreeParameter::nextEvent();
    EQ(size_t(0), CTreeParameter::m_scoreboard.size());
    EQ(size_t(2), CTreeParameter::m_event.size());
}
// nothing to collect gives empty vector:
void TPTest::collect_1() {
    EQ(size_t(0), CTreeParameter::collectEvent().size());
}
// Put some stuff in event and a sparse scoreboard we get the right struff out.

void TPTest::collect_2() {
    std::vector<double> eventData = {1.0, 2.1, 3.2, 5.3, 7.5, 13.7}; // See the pattern?
    std::vector<unsigned> sbdata = {2, 3, 5};                        // A prime example.
    CTreeParameter::m_event.insert(CTreeParameter::m_event.begin(), eventData.begin(), eventData.end());
    CTreeParameter::m_scoreboard.insert(CTreeParameter::m_scoreboard.begin(), sbdata.begin(), sbdata.end());
    
    auto result = CTreeParameter::collectEvent();
    
    EQ(sbdata.size(), result.size());
    for (int i =0;i < result.size(); i++) {
        EQ(result[i].first, sbdata[i]);
        EQ(result[i].second, eventData[sbdata[i]]);
    }
}

// default limits

void TPTest::dlimits() {
    CTreeParameter::setDefaultLimits(-1.0, 1.0);
    
    EQ(double(-1.0), CTreeParameter::m_defaultSpecification.s_low);
    EQ(double(1.0), CTreeParameter::m_defaultSpecification.s_high);
}
// default bins:
void TPTest::dbins() {
    CTreeParameter::setDefaultBins(1024);
    EQ(unsigned(1024), CTreeParameter::m_defaultSpecification.s_chans);
}
void TPTest::dunits() {
    CTreeParameter::setDefaultUnits("furlong/fortnight");
    EQ(std::string("furlong/fortnight"), CTreeParameter::m_defaultSpecification.s_units);
}
// Get reference to event:

void TPTest::getEvent() {
    for (int i =0; i < 100; i++) {
        CTreeParameter::m_event.push_back(i);
    }
    auto& e = CTreeParameter::getEvent();
    EQ(size_t(100), e.size());
    
    for (int i =0;i < 100; i++) {
        EQ(double(i), e.at(i));
    }
}
// Get scoreboard

void TPTest::getsb() {
    std::vector<unsigned> sbdata = {2, 3, 5};                        // A prime example.
    CTreeParameter::m_scoreboard.insert(CTreeParameter::m_scoreboard.begin(), sbdata.begin(), sbdata.end());
    
    auto& s = CTreeParameter::getScoreboard();
    EQ(sbdata.size(), s.size());
    for (int i =0; i < s.size(); i++) {
        EQ(sbdata[i], s.at(i));
    }
}