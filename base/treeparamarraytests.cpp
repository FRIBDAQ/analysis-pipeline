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

/** @file:  treeparamarraytests.cpp
 *  @brief: Tests frib::analysis::CTreeParameterArray
 */
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#define private public
#include "TreeParameterArray.h"
#include "TreeParameter.h"
#undef private
#include <sstream>
#include <iomanip>
#include <string>

using namespace frib::analysis;

class TPATest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(TPATest);
    CPPUNIT_TEST(construct_1);
    CPPUNIT_TEST(construct_2);
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
    void construct_1();
    void construct_2();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TPATest);

// default construction depends on Initialize:
void TPATest::construct_1()
{
    CTreeParameterArray a;
    EQ(0, a.m_nFirstIndex);
    ASSERT(a.m_Parameters.empty());
}
// Construct an array ith base, resolution, element count and nonzero base:

void TPATest::construct_2() {
    CTreeParameterArray a("test", 12, 16, -1);
    
    EQ(-1, a.m_nFirstIndex);
    EQ(size_t(16), a.m_Parameters.size());
    for (int i =0; i < 16; i++) {
        CTreeParameter& el(*a.m_Parameters.at(i));
        EQ(double(0), el.getStart());
        EQ(double(4096), el.getStop());
        EQ(unsigned(4096), el.getBins());
        EQ(CTreeParameter::m_defaultSpecification.s_units, el.getUnit());
        
        std::stringstream nameStream;
        nameStream << "test." ;
        if (i-1 < 0) {
            nameStream << "-" << std::setfill('0') << std::setw(2) <<   -(i-1);
        } else {
            nameStream << std::setfill('0') << std::setw(2) <<  i-1;
        }
        std::string sbname(nameStream.str());
        EQ(sbname, el.getName());
    }
}