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
#include <stdexcept>

using namespace frib::analysis;

class TPATest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(TPATest);
    CPPUNIT_TEST(construct_1);
    CPPUNIT_TEST(construct_2);
    CPPUNIT_TEST(construct_3);
    CPPUNIT_TEST(construct_4);
    CPPUNIT_TEST(construct_5);
    CPPUNIT_TEST(construct_6);
    
    // Note construtors call initialize so there's no need to test that(?).
    
    CPPUNIT_TEST(index_1);
    CPPUNIT_TEST(index_2);
    
    CPPUNIT_TEST(reset);
    
    CPPUNIT_TEST(iteration);
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
    void construct_3();
    void construct_4();
    void construct_5();
    void construct_6();
    
    void index_1();
    void index_2();
    
    void reset();
    
    void iteration();
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
// construct with default metadata.

void TPATest::construct_3() {
    CTreeParameterArray a("test", 16, -1);
    EQ(-1, a.m_nFirstIndex);
    EQ(size_t(16), a.m_Parameters.size());
    for (int i =0; i < 16; i++) {
        CTreeParameter& el(*a.m_Parameters.at(i));
        EQ(CTreeParameter::m_defaultSpecification.s_low, el.getStart());
        EQ(CTreeParameter::m_defaultSpecification.s_high, el.getStop());
        EQ(CTreeParameter::m_defaultSpecification.s_chans, el.getBins());
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
// construct with only units specified:

void TPATest::construct_4() {
    CTreeParameterArray a("test", "mm", 16, -1);
    EQ(-1, a.m_nFirstIndex);
    EQ(size_t(16), a.m_Parameters.size());
    for (int i =0; i < 16; i++) {
        CTreeParameter& el(*a.m_Parameters.at(i));
        EQ(CTreeParameter::m_defaultSpecification.s_low, el.getStart());
        EQ(CTreeParameter::m_defaultSpecification.s_high, el.getStop());
        EQ(CTreeParameter::m_defaultSpecification.s_chans, el.getBins());
        EQ(std::string("mm"), el.getUnit());
        
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
// low/high units for each element.
void TPATest::construct_5() {
    CTreeParameterArray a("test", -1.0, 1.0, "mm", 16, -1);
    EQ(-1, a.m_nFirstIndex);
    EQ(size_t(16), a.m_Parameters.size());
    for (int i =0; i < 16; i++) {
        CTreeParameter& el(*a.m_Parameters.at(i));
        EQ(double(-1.0), el.getStart());
        EQ(double(1.0), el.getStop());
        EQ(CTreeParameter::m_defaultSpecification.s_chans, el.getBins());
        EQ(std::string("mm"), el.getUnit());
        
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
// Full construction of elements:
void TPATest::construct_6() {
    CTreeParameterArray a("test", 1024, -1.0, 1.0, "mm", 16, -1);
    EQ(-1, a.m_nFirstIndex);
    EQ(size_t(16), a.m_Parameters.size());
    for (int i =0; i < 16; i++) {
        CTreeParameter& el(*a.m_Parameters.at(i));
        EQ(double(-1.0), el.getStart());
        EQ(double(1.0), el.getStop());
        EQ(unsigned(1024), el.getBins());
        EQ(std::string("mm"), el.getUnit());
        
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
// Test valid indexing:

void TPATest::index_1() {
    CTreeParameterArray a("test", 1024, -1.0, 1.0, "mm", 16, -1);
    
    for (int i =-1; i < 15; i++) {
        CTreeParameter& sb(*a.m_Parameters.at(i+1));
        CTreeParameter& is(a[i]);
        
        
        EQ(sb.getStart(), is.getStart());
        EQ(sb.getStop(), is.getStop());
        EQ(sb.getBins(), is.getBins());    // Functionally the same.
        EQ(sb.getUnit(), is.getUnit());
        EQ(sb.getName(), is.getName());
        EQ(&sb, &is);                     // Actually the same.
    }
}
// invalid index tosses std::out_of_range:

void TPATest::index_2()
{
    CTreeParameterArray a("test", 1024, -1.0, 1.0, "mm", 16, -1);
    CPPUNIT_ASSERT_THROW(a[-2], std::out_of_range);
    CPPUNIT_ASSERT_THROW(a[15], std::out_of_range);
}
// Reset invalidates all elements.

void TPATest::reset() {
    CTreeParameterArray a("test", 1024, -1.0, 1.0, "mm", 16, -1);
    for (int i =-1; i < 15; i++) {
        a[i] = i*3.1416;
    }
    // All ar now valid.
    
    a.Reset();    // all should now be invalid:
    
    for (int i =-1; i< 15; i++) {
        ASSERT(!a[i].isValid());
    }
}
/// Use begin/end iteration.

void TPATest::iteration() {
    CTreeParameterArray a("test", 1024, -1.0, 1.0, "mm", 16, -1);
    
    int i = -1;
    for (auto p = a.begin(); p != a.end(); p++) {
        CTreeParameter* is = *p;
        CTreeParameter* sb(&a[i]);
        EQ(sb, is);
        i++;
    }
}