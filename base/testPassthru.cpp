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

/** @file:  testPassthru.cpp
 *  @brief: Check the file passthruTest created.
 */
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include "AnalysisRingItems.h"
#include "DataReader.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>

using namespace frib::analysis;
extern std::string outfile;
class passthrutest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(passthrutest);
    CPPUNIT_TEST(header_1);
    CPPUNIT_TEST(contents_1);
    CPPUNIT_TEST_SUITE_END();
    
private:
    int m_fd;
public:
    void setUp() {
        m_fd = open(outfile.c_str(), O_RDONLY);
        ASSERT(m_fd > 0);
    }
    void tearDown() {
        close(m_fd);
    }
protected:
    void header_1();
    void contents_1();
};

CPPUNIT_TEST_SUITE_REGISTRATION(passthrutest);

// First should be two headers for the parameter/variable defs:
void passthrutest::header_1()
{
    const ParameterDefinitions* record1;
    const VariableItem*         record2;
    
    CDataReader reader(m_fd, 1024*1024*8);   // Should be big enough to suck it all in.
    auto info = reader.getBlock(1024*1024*8);
    
    EQ(size_t(102), info.s_nItems);          // 100 passthrus + the docs.
    ASSERT(info.s_pData);
    
    record1 = reinterpret_cast<const ParameterDefinitions*>(info.s_pData);
    EQ(PARAMETER_DEFINITIONS, record1->s_header.s_type);
    EQ(std::uint32_t(0), record1->s_numParameters);
    EQ(sizeof(ParameterDefinitions), size_t(record1->s_header.s_size));
    EQ(sizeof(std::uint32_t), size_t(record1->s_header.s_unused));
    
    record2 = reinterpret_cast<const VariableItem*>(record1+1);
    EQ(VARIABLE_VALUES, record2->s_header.s_type);
    EQ(std::uint32_t(0), record2->s_numVars);
    EQ(sizeof(VariableItem), size_t(record2->s_header.s_size));
    EQ(sizeof(std::uint32_t), size_t(record2->s_header.s_unused));
    
}
// check that the post doc records are all correct

void passthrutest::contents_1() {
#pragma pack(push, 1)
    struct Item {
        RingItemHeader s_header;
        std::uint8_t   s_body[100];
    } ;
#pragma pack(pop)
    union {
        const RingItemHeader* pH;
        const std::uint8_t*   pB;
        const Item*           pItem;
    } p;
    
    CDataReader reader(m_fd, 1024*1024*8);   // Should be big enough to suck it all in.
    auto info = reader.getBlock(1024*1024*8);
    
    // Skip the headers:
    
    p.pB = reinterpret_cast<const std::uint8_t*>(info.s_pData);
    p.pB += p.pH->s_size;
    
    for (int item = 0; item < 100; item++) {
        p.pB += p.pH->s_size;
        
        EQ(std::uint32_t(6), p.pItem->s_header.s_type );    // Invented type:
        EQ(sizeof(std::uint32_t), size_t(p.pItem->s_header.s_unused));
        EQ(sizeof(Item), size_t(p.pItem->s_header.s_size));
        
        // payload contents:
        
        for (int i = 0; i < 100; i++) {
            EQ(std::uint8_t(i + item), p.pItem->s_body[i]);
        }
    }
}