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

/** @file:  testouttests.cpp
 *  @brief: Tests for the success of the testOutput parallel program.
 */
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#define private public
#include "TreeParameter.h"
#undef private
#include "TreeVariable.h"
#include "AnalysisRingItems.h"
#include "DataReader.h"

#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

using namespace frib::analysis;

extern std::string testFile;

class outputtest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(outputtest);
    CPPUNIT_TEST(countItems1);
    
    CPPUNIT_TEST(front_1);
    CPPUNIT_TEST(front_2);
    CPPUNIT_TEST(front_3);
    CPPUNIT_TEST_SUITE_END();
    
private:
    int m_fd;               // File open on the test output file.
    CDataReader* m_pReader;
public:
    void setUp() {
        m_fd = open(testFile.c_str(), O_RDONLY);
        if (m_fd < 0) {
            const char* reason = strerror(errno);
            std::string msg = "Test file open failed: ";
            msg += reason;
            CPPUNIT_FAIL(msg);
        }
        m_pReader = new CDataReader(m_fd, 10*1024*1024);   // Sufficient to read all.
    }
    void tearDown() {
        close(m_fd);
        delete m_pReader;
    }
protected:
    void countItems1();
    void front_1();
    void front_2();
    void front_3();
private:
    void reconstructParameters(const ParameterDefinitions* pParams);
    const void* skipItems(const void* pData, size_t nItems = 1);
};

CPPUNIT_TEST_SUITE_REGISTRATION(outputtest);


// skip nItems ring items from pData
// Assumption header size is the first 32 bits of the item.
const void*
outputtest::skipItems(const void* pData, size_t nItems) {
    union {
        const std::uint8_t* p8;
        const std::uint32_t* p32;
    } p;
    p.p32 = reinterpret_cast<const std::uint32_t*>(pData);
    for (int i =0; i < nItems; i++) {
        p.p8 += *p.p32;
    }
    return p.p8;
}


// create parameters from the definitions item -- by name only.

void
outputtest::reconstructParameters(const ParameterDefinitions* pParams) {
    const ParameterDefinition* p = pParams->s_parameters;
    for (int i = 0; i < pParams->s_numParameters; i++) {
        std::string name = p->s_parameterName;
        CTreeParameter param(name);
        const char* pName = p->s_parameterName;
        pName += strlen(pName) + 1;     // Next def.
        p = reinterpret_cast<const ParameterDefinition*>(pName);
    }
}

// Should have 102 items:
void outputtest::countItems1()
{
    auto r = m_pReader->getBlock(10*1024*1024);
    m_pReader->done();
    EQ(std::size_t(102), r.s_nItems);
}
// First item is a parameterdefinitions item:

void outputtest::front_1()
{
    auto r = m_pReader->getBlock(1024*1024);
    ASSERT(r.s_pData);     // Not an eof.
    const ParameterDefinitions* p =
        reinterpret_cast<const ParameterDefinitions*>(r.s_pData);
    
    EQ(PARAMETER_DEFINITIONS, p->s_header.s_type);   // verify it's the right type.
    EQ(std::uint32_t(17), p->s_numParameters); // Verify parameter count.
    
}
// Verify we can reconstruct the parameters from the definition ring item:

void outputtest::front_2() {
    auto r = m_pReader->getBlock(1024*1024);
    ASSERT(r.s_pData);     // Not an eof.
    const ParameterDefinitions* p =
        reinterpret_cast<const ParameterDefinitions*>(r.s_pData);
    reconstructParameters(p);
    
    EQ(size_t(17), CTreeParameter::getDefinitions().size());   // Right number.
    
    // here are the names we should have:
    
    const char* names[17] = { 
        "simple", "array.00", "array.01", "array.02", "array.03", "array.04"
        , "array.05", "array.06", "array.07", "array.08", "array.09", "array.10"
        , "array.11", "array.12", "array.13", "array.14", "array.15"
    };
    for (int i =0;i < 17; i++) {
        ASSERT(CTreeParameter::lookupParameter(names[i]));
    }
}
// There should be 17 variable definitions:

void outputtest::front_3() {
    auto r = m_pReader->getBlock(1024*1024);
    ASSERT(r.s_pData);     // Not an eof.
    
    // Skip the first item:
    
    const VariableItem* pItem =
        reinterpret_cast<const VariableItem*>(skipItems(r.s_pData));
    EQ(VARIABLE_VALUES, pItem->s_header.s_type);
    EQ(std::uint32_t(33), pItem->s_numVars);
}