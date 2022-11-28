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

/** @file:  testsorttests.cpp
 *  @brief: Tests for the output of sortTest.
 */
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <string>
#include <AnalysisRingItems.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "DataReader.h"

using namespace frib::analysis;

extern std::string testFile;
static const std::size_t BUFFER_SIZE(100*1024*1024);   // 100Mbyte buffer should do fine.
class sortouttest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(sortouttest);
    CPPUNIT_TEST(contents_1);
    CPPUNIT_TEST(contents_2);
    CPPUNIT_TEST(contents_3);
    CPPUNIT_TEST(contents_4);
    CPPUNIT_TEST_SUITE_END();
    
private:
    int m_fd;
public:
    void setUp() {
        m_fd = open(testFile.c_str(), O_RDONLY);
        ASSERT(m_fd >= 0);
    }
    void tearDown() {
        close(m_fd);
    }
protected:
    void contents_1();
    void contents_2();
    void contents_3();
    void contents_4();
};

CPPUNIT_TEST_SUITE_REGISTRATION(sortouttest);

// There should be 2002 items:
// The tree variable and treeparameter entries, then 1000 data items
// per worker - 2 workers.
void sortouttest::contents_1()
{
    CDataReader reader(testFile.c_str(), BUFFER_SIZE);
    auto info = reader.getBlock(BUFFER_SIZE);
    EQ(std::size_t(2002), info.s_nItems);
}
// Should be a PARAMETER_DEFINITIOSN, VARIABLE_VALUES and 2000 PARAMETER_DATA items:

void sortouttest::contents_2() {
    CDataReader reader(testFile.c_str(), BUFFER_SIZE);
    auto info = reader.getBlock(BUFFER_SIZE);
    ASSERT(info.s_pData);
    
    union {
        const std::uint8_t* p8;
        const RingItemHeader* ph;
    } p;
    p.p8 = reinterpret_cast<const std::uint8_t*>(info.s_pData);
    
    // parameter defs:
    
    EQ(PARAMETER_DEFINITIONS, p.ph->s_type);
    p.p8 += p.ph->s_size;
    
    // Variable defs:
    
    EQ(VARIABLE_VALUES, p.ph->s_type);
    
    for (int i =0; i < 2000; i++) {
        p.p8 += p.ph->s_size;
        EQ(PARAMETER_DATA, p.ph->s_type);
    }
    
}
// trigger and counts are right:

void sortouttest::contents_3() {
    CDataReader reader(testFile.c_str(), BUFFER_SIZE);
    auto info = reader.getBlock(BUFFER_SIZE);
    
    union {
        const std::uint8_t* p8;
        const RingItemHeader* ph;
        const ParameterItem*  pp;
    } p;
    p.p8 = reinterpret_cast<const std::uint8_t*>(info.s_pData);
    
    
    // Skip the two doc items:
    
    p.p8 += p.ph->s_size;
    for (int i =0; i < 2000; i++) {
        p.p8 += p.ph->s_size;
        EQ(std::uint64_t(i), p.pp->s_triggerCount);
        EQ(std::uint32_t(10), p.pp->s_parameterCount);
    }
}
// Check parameter contents:
void sortouttest::contents_4() {
    CDataReader reader(testFile.c_str(), BUFFER_SIZE);
    auto info = reader.getBlock(BUFFER_SIZE);
    
    union {
        const std::uint8_t* p8;
        const RingItemHeader* ph;
        const ParameterItem*  pp;
    } p;
    p.p8 = reinterpret_cast<const std::uint8_t*>(info.s_pData);
    
    
    // Skip the two doc items:
    
    p.p8 += p.ph->s_size;
    
    // Check the contents of the physics items.
    
    for (int i =0; i < 2000; i++) {
        p.p8 += p.ph->s_size;
        auto startingNum = p.pp->s_triggerCount;
        auto n           = p.pp->s_parameterCount;
        for (int j = 0; j < n; j++) {
            EQ(std::uint32_t(startingNum+j), p.pp->s_parameters[j].s_number);
            EQ(p.pp->s_parameters[j].s_number*2.0, p.pp->s_parameters[j].s_value);
        }
    }
}