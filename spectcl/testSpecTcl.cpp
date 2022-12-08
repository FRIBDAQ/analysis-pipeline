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

/** @file:  testSpecTcl.cpp
 *  @brief: Test contents of file made by spectclTest program.
 */
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <AnalysisRingItems.h>
#define private public
#include <TreeParameter.h>
#undef public
#include <TreeParameterArray.h>
#include <DataReader.h>

#include <vector>
#include <cstdint>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>

using namespace frib::analysis;

extern std::string testFile;

const std::uint32_t BEGIN_RUN = 1;
const std::uint32_t END_RUN   = 2;


class spectest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(spectest);
    CPPUNIT_TEST(header_1);
    CPPUNIT_TEST(data_1);
    CPPUNIT_TEST_SUITE_END();
    
private:
    CTreeParameter*  m_pSum;
    CTreeParameterArray* m_pArray;
    std::vector<CTreeParameter*> m_fileToInternalMapping;
    
    int m_fd;
    
public:
    void setUp() {
        // Note this order makes the ids different than in the file...
        // this is intentional.
        
        
        m_pSum = new CTreeParameter("sum");
        m_pArray = new CTreeParameterArray("array", 16, 0);
        
        
        m_fd = open(testFile.c_str(), O_RDONLY);
        ASSERT(m_fd >= 0);
        
    }
    void tearDown() {
        delete m_pSum;
        delete m_pArray;
        
        CTreeParameter::m_parameterDictionary.clear();
        CTreeParameter::m_scoreboard.clear();
        CTreeParameter::m_event.clear();
        
        for (auto p: m_fileToInternalMapping) {
            delete p;
        }
        m_fileToInternalMapping.clear();
        
        close(m_fd);
    }
    
protected:
    void header_1();
    void data_1();
private:
    void makeMapping(const ParameterDefinitions* pDefs);
    void addMapping(const ParameterDefinition* pDef);
    void dumpMappings();
    void mapData(const ParameterItem* pData);
};

// Use the mappings to map an item.

void
spectest::mapData(const ParameterItem* pData) {
    union {
        const ParameterValue* pV;
        const std::uint8_t*   p8;
    } p;
    p.pV = pData->s_parameters;
    
    for (int i =0; i < pData->s_parameterCount; i++) {
        unsigned n = p.pV->s_number;
        if (m_fileToInternalMapping[n]) {
            *m_fileToInternalMapping[n] = p.pV->s_value;
        }
        
        p.p8 += sizeof(ParameterValue);
    }
}

// Add a mapping between a file parameter and an internal parameter.

void
spectest::addMapping(const ParameterDefinition* pDef) {
    // First be sure the mapping array is big enough:
    
    if (pDef->s_parameterNumber >= m_fileToInternalMapping.size()) {
        m_fileToInternalMapping.resize(pDef->s_parameterNumber + 1, nullptr);
    }
    // Making an new parameter from the name will either map it to an existing one
    // or create a new one:
    
    std::string name = pDef->s_parameterName;
    m_fileToInternalMapping[pDef->s_parameterNumber] = new CTreeParameter(name);
    
}
// Given the parameter definition ring item make all mappings:

void
spectest::makeMapping(const ParameterDefinitions* pDefs) {
    union {
        const ParameterDefinition* pD;
        const std::uint8_t*        p8;
    } p;
    p.pD = pDefs->s_parameters;

    for (int i =0; i < pDefs->s_numParameters; i++) {
        addMapping(p.pD);
        size_t nBytes= sizeof(ParameterDefinition) + strlen(p.pD->s_parameterName) + 1;
        p.p8 += nBytes;
    }
}

// diagnostics.

void
spectest::dumpMappings() {
    unsigned i  = 0;
    for (auto p : m_fileToInternalMapping) {
        std::cerr << i << ": ";
        if (p) {
            std::cerr << p->getName() << " " << p->getId();
        } else {
            std::cerr << "(null)";
        }
        std::cerr << std::endl;
        i++;
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(spectest);

// First record should be parameter defs, second variable defs.
// third begin run.
// for fun build the parameter mapping from the parameter defs  item.
void spectest::header_1()
{
    CDataReader r(m_fd, 1000);
    auto info = r.getBlock(1000);
    ASSERT(info.s_nItems >= 3);    // Want all header recrods.
    
    union {
        const std::uint8_t*  p8;
        const RingItemHeader* pH;
    } p;
    p.p8 = reinterpret_cast<const std::uint8_t*>(info.s_pData);
    
    // First item must be a parameter def record:
    
    EQ(PARAMETER_DEFINITIONS, p.pH->s_type);
    
    makeMapping(reinterpret_cast<const ParameterDefinitions*>(p.pH));
    
    // Second item must be a VARIABLE_VALUES
    
    p.p8 += p.pH->s_size;
    EQ(VARIABLE_VALUES, p.pH->s_type);
    
    // Then a minimal begin run:
    
    p.p8 += p.pH->s_size;
    EQ(BEGIN_RUN, p.pH->s_type);   
    
    
}
// after establishing the mapping we should be able to read the  data into
// this test just requires that the number of parameter data items is correct.


void spectest::data_1() {
    CDataReader r(m_fd, 1000);
    auto info = r.getBlock(1000);
    ASSERT(info.s_nItems >= 3);    // Want all header recrods.
    
    union {
        const std::uint8_t*  p8;
        const RingItemHeader* pH;
    } p;
    p.p8 = reinterpret_cast<const std::uint8_t*>(info.s_pData);
    
    // First item must be a parameter def record:
    
    EQ(PARAMETER_DEFINITIONS, p.pH->s_type);
    
    makeMapping(reinterpret_cast<const ParameterDefinitions*>(p.pH));
    
    // Second item must be a VARIABLE_VALUES
    
    p.p8 += p.pH->s_size;
    EQ(VARIABLE_VALUES, p.pH->s_type);
    
    // Then a minimal begin run:
    
    p.p8 += p.pH->s_size;
    EQ(BEGIN_RUN, p.pH->s_type);   
    p.p8 += p.pH->s_size;
    // Now for all the data items:
    
    size_t nRemaining = info.s_nItems - 3;
    int nPhy = 0;
    while (info.s_nItems) {

        if (p.pH->s_type == PARAMETER_DATA) {
            mapData(reinterpret_cast<const ParameterItem*>(p.pH));
            nPhy++;
        } else {
            EQ(END_RUN, p.pH->s_type);   // only other thing is an end run.
        }
        
        p.p8 += p.pH->s_size;
        nRemaining--;

        if(nRemaining == 0) {
            r.done();
            info = r.getBlock(1000);     // next block -- if there is one.
            nRemaining = info.s_nItems;
            p.p8 = reinterpret_cast<const std::uint8_t*>(info.s_pData);
        }
    }
    
    EQ(10000, nPhy);
}