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

/** @file:  worker1Tests.cpp
 *  @brief: Tests for the output file created by worker 1.
 */


#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include "AnalysisRingItems.h"
#include "TreeParameterArray.h"
#include "TreeParameter.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <cstdint>
#include <iostream>

extern std::string filename;

const std::uint32_t BEGIN_RUN =1;
const std::uint32_t END_RUN   =2;


using namespace frib::analysis;

class worker1test : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(worker1test);
    CPPUNIT_TEST(header_1);
    CPPUNIT_TEST(statechanges_1);
    CPPUNIT_TEST(events_1);
    CPPUNIT_TEST(events_2);
    CPPUNIT_TEST(events_3);
    CPPUNIT_TEST_SUITE_END();
    
private:
    int           m_fd;
    std::uint8_t* m_data;
    off_t         m_nBytes;
public:
    void setUp() {
        m_fd = open(filename.c_str(), O_RDONLY);
        ASSERT(m_fd >= 0);
        // figure out how big the file is and suck it oll in
        // not too big since the items are minimal.
        
        struct stat statbuf;
        ASSERT(fstat(m_fd, &statbuf) >= 0);
        m_data = new std::uint8_t[statbuf.st_size];
        EQ(ssize_t(statbuf.st_size), read(m_fd, m_data, statbuf.st_size));
        m_nBytes = statbuf.st_size;
        
        //rewind the file in case a test wants to read it for itself:
        
        lseek(m_fd, 0, SEEK_SET);
        
    }
    void tearDown() {
        delete []m_data;
        close(m_fd);
    }
protected:
    void header_1();
    void statechanges_1();
    void events_1();
    void events_2();
    void events_3();
};

CPPUNIT_TEST_SUITE_REGISTRATION(worker1test);

void worker1test::header_1()
{
    union {
        pRingItemHeader pH;
        std::uint8_t*     p8;
    } p;
    p.p8 = m_data;
    
    // First two items should be just the parameter and var defs:
    
    EQ(PARAMETER_DEFINITIONS, p.pH->s_type);
    p.p8 += p.pH->s_size;
    EQ(VARIABLE_VALUES, p.pH->s_type);
    
    
}
/** Somewhere in all of those items there should be a begin and an end item:
 *  because of how passthroughs are handled we can't say anything about where they are
 */
void worker1test::statechanges_1()
{
    bool begin(false);
    bool end(false);
    off_t size(0);
    
    union {
        pRingItemHeader pH;
        std::uint8_t*     p8;
    } p;
    p.p8 = m_data;
    
    
    while(size < m_nBytes) {
        if (p.pH->s_type == BEGIN_RUN) begin = true;
        if (p.pH->s_type == END_RUN) end = true;
        
        size += p.pH->s_size;
        p.p8 += p.pH->s_size;
    }
    ASSERT(begin);
    ASSERT(end);
}
// Ensure the event data has the right stuff.

void worker1test::events_1() {
    // There should be 10000 physics events.
    
    off_t size(0);
    
    union {
        pRingItemHeader pH;
        std::uint8_t*     p8;
    } p;
    p.p8 = m_data;
    
    size_t nEvents(0);
    
    while(size < m_nBytes) {
        
        if (p.pH->s_type == PARAMETER_DATA) nEvents++;
        
        size += p.pH->s_size;
        p.p8 += p.pH->s_size;
    }
    EQ(size_t(10000), nEvents);
}
// Each event has the correct trigger number and # of parameters.

void worker1test::events_2() {
    // There should be 10000 physics events.
    
    off_t size(0);
    
    union {
        pRingItemHeader pH;
        pParameterItem   pP;
        std::uint8_t*     p8;
    } p;
    p.p8 = m_data;
    
    std::uint64_t trigger = 0;
    
    while(size < m_nBytes) {
        
        if (p.pH->s_type == PARAMETER_DATA) {
            EQ(trigger, p.pP->s_triggerCount);
            std::uint32_t expectedParams = trigger % 10 + 1;
            EQ(expectedParams, p.pP->s_parameterCount);
            trigger++;
        }
        
        size += p.pH->s_size;
        p.p8 += p.pH->s_size;
    }
}
// Parameters and contents are going to be correct:

void worker1test::events_3()
{
    off_t size(0);
    
    union {
        pRingItemHeader pH;
        pParameterItem   pP;
        std::uint8_t*     p8;
    } p;
    p.p8 = m_data;
    
    std::uint64_t trigger = 0;
    CTreeParameterArray array("array", 16, 0); // so we can get ids.
    
    while(size < m_nBytes) {
        
        if (p.pH->s_type == PARAMETER_DATA) {
            
            std::uint32_t expectedParams = trigger % 10 + 1;
            for (int i = 0; i < expectedParams; i++) {
                EQ(array[i].getId(), p.pP->s_parameters[i].s_number);
                EQ(double(expectedParams-1), p.pP->s_parameters[i].s_value);
            }
            trigger++;
        }
        
        size += p.pH->s_size;
        p.p8 += p.pH->s_size;
    }
}