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
class aTestSuite : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(aTestSuite);
    CPPUNIT_TEST(header_1);
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
};

CPPUNIT_TEST_SUITE_REGISTRATION(aTestSuite);

// First should be two headers for the parameter/variable defs:
void aTestSuite::header_1()
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