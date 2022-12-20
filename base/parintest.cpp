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

/** @file:  parintests.cpp
 *  @brief: parameter input tests.
 */
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <string>
#include <AnalysisRingItems.h>
#include <DataReader.h>

extern std::string outputFile;    // From the ouputter
extern std::string workerFile;    // From the worker.
extern int         numberEvents;

using namespace frib::analysis;

const std::uint32_t BEGIN_RUN = 1;
const std::uint32_t END_RUN  = 2;

class parintest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(parintest);
    CPPUNIT_TEST(outfile_1);
    CPPUNIT_TEST(outfile_2);
    
    CPPUNIT_TEST(wfile_1);
    CPPUNIT_TEST_SUITE_END();
    // Set up and teardown -- well opening both output files
    // is a bit of overkill.  But opening them with a common
    // Data reader is not so:
private:
    CDataReader* m_pReader;
public:
    
    
    
    void setUp() {
        m_pReader = nullptr;
    }
    void tearDown() {
        delete m_pReader;
    }
protected:
    void outfile_1();
    void outfile_2();
    
    void wfile_1();
};

CPPUNIT_TEST_SUITE_REGISTRATION(parintest);


// Output file should have exactly two items:
void parintest::outfile_1()
{
    m_pReader = new CDataReader(outputFile.c_str(), 1024*1024);   // should be big enough.
    auto info = m_pReader->getBlock(1024*1024);
    
    EQ(size_t(2), info.s_nItems);
    ASSERT(info.s_pData);
    ASSERT(info.s_nbytes != 0);
}
// should be a minimal begin and end item.

void parintest::outfile_2()
{
    m_pReader = new CDataReader(outputFile.c_str(), 1024*1024);
    
    auto info = m_pReader->getBlock(1024*1024);      // ALready established there are items.
    
    const RingItemHeader* pItem = reinterpret_cast<const RingItemHeader*>(info.s_pData);
    EQ(std::uint32_t(sizeof(RingItemHeader)), pItem->s_size);
    EQ(BEGIN_RUN, pItem->s_type);
    EQ(std::uint32_t(sizeof(std::uint32_t)), pItem->s_unused);
    
    pItem++;
    EQ(std::uint32_t(sizeof(RingItemHeader)), pItem->s_size);
    EQ(END_RUN, pItem->s_type);
    EQ(std::uint32_t(sizeof(std::uint32_t)), pItem->s_unused);
}
// The worker file should have numberEvents+2 items
// +2 from the documentation items.

void wfile_1() {
    m_pReader = new CDataReader(workerFile.c_str(), 8*1024*1024);  // big enough I hope.
    
    auto info = m_pReader->getBlock(8192*8192);
    
    EQ(numberEvents+2, int(info.s_nItems));
}
