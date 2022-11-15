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
    CPPUNIT_TEST(front_4);
    
    CPPUNIT_TEST(data_1);
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
    void front_4();
    
    void data_1();
private:
    void reconstructParameters(const ParameterDefinitions* pParams);
    const void* skipItems(const void* pData, size_t nItems = 1);
    void reconstructVariables(const VariableItem* pVars);
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
// reconstruct tree variables from the data in the variable item
// ring item.

void
outputtest::reconstructVariables(const VariableItem* pVars) {
    const Variable* p = pVars->s_variables;
    for (int i = 0; i < pVars->s_numVars; i++) {
        std::string name = p->s_variableName;
        CTreeVariable v(name, p->s_value, p->s_variableUnits);
        const char* pName = p->s_variableName;
        pName += strlen(pName) +1;    // Next variable def/value.
        p = reinterpret_cast<const Variable*>(pName);
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
// There should be 33 variable definitions:

void outputtest::front_3() {
    auto r = m_pReader->getBlock(1024*1024);
    ASSERT(r.s_pData);     // Not an eof.
    
    // Skip the first item:
    
    const VariableItem* pItem =
        reinterpret_cast<const VariableItem*>(skipItems(r.s_pData));
    EQ(VARIABLE_VALUES, pItem->s_header.s_type);
    EQ(std::uint32_t(33), pItem->s_numVars);
}
// check names and values:

void outputtest::front_4() {
    auto r = m_pReader->getBlock(1024*1024);
    ASSERT(r.s_pData);     // Not an eof.
    
    // Skip the first item:
    
    const VariableItem* pItem =
        reinterpret_cast<const VariableItem*>(skipItems(r.s_pData));
    reconstructVariables(pItem);
    
    struct  Info {
        const char* s_name;
        const char* s_units;
        double      s_value;
    };
    const Info varInfo[33] = {
        {"simple-inc", "mm", 0.1},
        {"multipliers.00", "unitless", 0.0},
        {"multipliers.01", "unitless", 1.0},
        {"multipliers.02", "unitless", 2.0},
        {"multipliers.03", "unitless", 3.0},
        {"multipliers.04", "unitless", 4.0},
        {"multipliers.05", "unitless", 5.0},
        {"multipliers.06", "unitless", 6.0},
        {"multipliers.07", "unitless", 7.0},
        {"multipliers.08", "unitless", 8.0},
        {"multipliers.09", "unitless", 9.0},
        {"multipliers.10", "unitless", 10.0},
        {"multipliers.11", "unitless", 11.0},
        {"multipliers.12", "unitless", 12.0},
        {"multipliers.13", "unitless", 13.0},
        {"multipliers.14", "unitless", 14.0},
        {"multipliers.15", "unitless", 15.0},
        {"additions.00", "degrees", 0.0},
        {"additions.01", "degrees", 1.5},
        {"additions.02", "degrees", 3.0},
        {"additions.03", "degrees", 4.5},
        {"additions.04", "degrees", 6.0},
        {"additions.05", "degrees", 7.5},
        {"additions.06", "degrees", 9.0},
        {"additions.07", "degrees", 10.5},
        {"additions.08", "degrees", 12.0},
        {"additions.09", "degrees", 13.5},
        {"additions.10", "degrees", 15.0},
        {"additions.11", "degrees", 16.5},
        {"additions.12", "degrees", 18.0},
        {"additions.13", "degrees", 19.5},
        {"additions.14", "degrees", 21.0},
        {"additions.15", "degrees", 22.5},
        
    };
    for (int i = 0; i < pItem->s_numVars; i++) {
        auto def = CTreeVariable::lookupDefinition(varInfo[i].s_name);
        ASSERT(def);
        EQ(0, strcmp(varInfo[i].s_units, def->s_units.c_str()));
        EQ(varInfo[i].s_value, def->s_value);
    }
}
// after the first two items, we have a sequence of 100 data items
// followed by an EOF with 9 parameters per event...and sequential
// triggers numberd from 0.

void
outputtest::data_1()
{
    auto r = m_pReader->getBlock(1024*1024);
    ASSERT(r.s_pData);     // Not an eof.
    
    const ParameterItem* pItem =
        reinterpret_cast<const ParameterItem*>(skipItems(r.s_pData, 2));
    
    for (std::uint64_t i =0; i < 100; i++) {

        EQ(PARAMETER_DATA, pItem->s_header.s_type);
        EQ(i, pItem->s_triggerCount);
        EQ(std::uint32_t(9), pItem->s_parameterCount);
        
        // The following assumes parameters are sequentially assigned from
        // 0.
        
        auto pParam = pItem->s_parameters;
        EQ(std::uint32_t(1), pParam->s_number);
        EQ(-0.5, pParam->s_value);   // Simple.
        pParam++;
    
        for (int i =0; i < 8; i++) {
            EQ(std::uint32_t((2 + i*2)), pParam->s_number);
            EQ(double(i*10), pParam->s_value);
            pParam++;
        }
        
        pItem = reinterpret_cast<const ParameterItem*>(skipItems(pItem));   // next one
    }
    m_pReader->done();
    r = m_pReader->getBlock(1024*1024);
    ASSERT(!r.s_pData);                      // eof.
}