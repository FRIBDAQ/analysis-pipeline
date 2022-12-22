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

/** @file:  worker2tests.cpp
 *  @brief: Test the output from testWorker2.
 */
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "AnalysisRingItems.h"
#include "DataReader.h"
#include "TreeParameter.h"
#include "TreeVariable.h"
#include <set>
#include <map>
#include <string.h>

using namespace frib::analysis;

extern std::string outputFile;
extern unsigned numEvents;

const std::uint32_t BEGIN_RUN = 1;
const std::uint32_t END_RUN   = 2;

class worker2test : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(worker2test);
    CPPUNIT_TEST(count_1);
    CPPUNIT_TEST(params_1);
    CPPUNIT_TEST(params_2);
    CPPUNIT_TEST(params_3);
    
    CPPUNIT_TEST(vars_1);
    CPPUNIT_TEST(vars_2);
    CPPUNIT_TEST(vars_3);
    
    CPPUNIT_TEST(state_1);
    CPPUNIT_TEST(state_2);
    
    CPPUNIT_TEST(data_1);
    CPPUNIT_TEST_SUITE_END();
    
private:
    CDataReader *m_pReader;
public:
    void setUp() {
        m_pReader = new CDataReader(outputFile.c_str(), 32768); // require multigulp.
    }
    void tearDown() {
        delete m_pReader;        
    }
protected:
    void count_1();
    void params_1();
    void params_2();
    void params_3();
    
    void vars_1();
    void vars_2();
    void vars_3();
    
    void state_1();
    void state_2();
    
    void data_1();
private:
    const void* nextItem(CDataReader::Result& info);            // info is modified.
};

//Assuming that s_pData points to an item points to the next ring item.
// info is modified to reflect the new position- if necessary, another
// chunk of data is fetched from the file:
const void*
worker2test::nextItem(CDataReader::Result& info) {
    info.s_nItems--;               // one less item.
    if (info.s_nItems) {           // there's at least one more:
        union {
            const RingItemHeader* ph;
            const std::uint8_t*   p8;
        } p;
        p.p8 = reinterpret_cast<const std::uint8_t*>(info.s_pData);
        ASSERT(info.s_nbytes >= p.ph->s_size);    // There must be that many bytes at least.
        info.s_nbytes -= p.ph->s_size;
        p.p8 += p.ph->s_size;
        info.s_pData = p.p8;
        return p.p8;
        
    } else {                       // must read:
        m_pReader->done();
        info = m_pReader->getBlock(32768);
        return info.s_pData;         // If done this will naturally be null.
    }
 }



CPPUNIT_TEST_SUITE_REGISTRATION(worker2test);
// there should be num events + 4 items in the file
//  (paramter defs, variable defs, begin, end are the extra four).
void worker2test::count_1()
{
    unsigned itemCount(0);
    auto info = m_pReader->getBlock(32768);
    while(info.s_nItems) {
        itemCount += info.s_nItems;
        m_pReader->done();
        info = m_pReader->getBlock(32768);
    }
    EQ(numEvents+4, itemCount);
}
// First item is a parameter def block:
// and there are 19 definitions.
void worker2test::params_1() {
    auto info = m_pReader->getBlock(32768);
    
    const ParameterDefinitions* pDefs =
        reinterpret_cast<const ParameterDefinitions*>(info.s_pData);
    EQ(PARAMETER_DEFINITIONS, pDefs->s_header.s_type);
    EQ(std::uint32_t(19), pDefs->s_numParameters);
}
// All the parameters should be defined - we've already checked the count.

void worker2test::params_2() {
    auto info = m_pReader->getBlock(32768);
    
    const ParameterDefinitions* pDefs =
        reinterpret_cast<const ParameterDefinitions*>(info.s_pData);
    auto defs = CTreeParameter::getDefinitions();
    EQ(size_t(pDefs->s_numParameters), defs.size());
    
    // Throw the names of the defs up into a set:
    // As we see a def we'll locate it and remove it from the set os any dups are
    // also caught.
    
    std::set<std::string> names;
    for (auto& def : defs) {
        names.insert(def.first);
    }
    // Now process each of the definitions in ParameterDefinitions
    // It must be in the se, then it's delete, and, when done the
    // set should be empty:
 
    const ParameterDefinition *p = pDefs->s_parameters;
    for (int i = 0; i < pDefs->s_numParameters; i++) {
        std::string aname = p->s_parameterName;
        auto iter = names.find(aname);
        ASSERT(iter != names.end());    // Can't use EQ since << for iterators does not work.
        names.erase(iter);
        
        // Next definition:
        
        const std::uint8_t* p8 = reinterpret_cast<const std::uint8_t*>(p);
        p8 += sizeof(ParameterDefinition) + aname.size() + 1;
        p = reinterpret_cast<const ParameterDefinition*>(p8);
    }
    ASSERT(names.empty());
    
}
// We established there are the right params and number of them.
// Let's make sure the ids match too:

void worker2test::params_3() {
    auto info = m_pReader->getBlock(32768);
    
    const ParameterDefinitions* pDefs =
        reinterpret_cast<const ParameterDefinitions*>(info.s_pData);
    auto defs = CTreeParameter::getDefinitions();
    EQ(size_t(pDefs->s_numParameters), defs.size());
    
    // Throw the names and definitions up into a map:
    // As we see a def we'll locate it and remove it from the set os any dups are
    // also caught.

    std::map<std::string, CTreeParameter::SharedData> map;
    for (auto& def : defs) {
        map[def.first] = def.second;
    }
    //
    const ParameterDefinition *p = pDefs->s_parameters;
    for (int i =0; i < pDefs->s_numParameters; i++) {
        std::string aname = p->s_parameterName;
        auto iter = map.find(aname);            // we now this succeeds from params_2.
        
        EQ(p->s_parameterNumber, std::uint32_t(iter->second.s_parameterNumber));
        
        
        // Next definition:
        
        const std::uint8_t* p8 = reinterpret_cast<const std::uint8_t*>(p);
        p8 += sizeof(ParameterDefinition) + aname.size() + 1;
        p = reinterpret_cast<const ParameterDefinition*>(p8);
    }
}

// Ensure we have the right number of vars:

void worker2test::vars_1() {
    auto info = m_pReader->getBlock(32768);
    const VariableItem* pItem = reinterpret_cast<const VariableItem*>(nextItem(info));
    ASSERT(pItem);                 // There _must_ be a variables item.
    EQ(VARIABLE_VALUES, pItem->s_header.s_type);
    EQ(std::uint32_t(17), pItem->s_numVars);
}
// Ensure all var names are there with no dups.

void worker2test::vars_2() {
    auto info = m_pReader->getBlock(32768);
    const VariableItem* pItem = reinterpret_cast<const VariableItem*>(nextItem(info));
    auto defs = CTreeVariable::getDefinitions();
    
    // Throw the names up into a set:
    
    std::set<std::string> varnames;
    for(auto& d : defs) {
        varnames.insert(d.first);
    }
    // iterate over vars in the def ensure each name can be found and after
    // deleting the found ones, the set is empty.
    
    const Variable* pV = pItem->s_variables;
    for (int i =0; i < pItem->s_numVars; i++) {
        std::string name = pV->s_variableName;
        auto iter = varnames.find(name);
        ASSERT(iter != varnames.end());
        varnames.erase(iter);
        
        const std::uint8_t* p8 = reinterpret_cast<const std::uint8_t*>(pV);
        p8 += sizeof(Variable) + name.size() + 1;
        pV = reinterpret_cast<const Variable*>(p8);
    }
    
    ASSERT(varnames.empty());
}
// make sure the defs/values are reflected properly:

void worker2test::vars_3() {
    auto info = m_pReader->getBlock(32768);
    const VariableItem* pItem = reinterpret_cast<const VariableItem*>(nextItem(info));
    auto defs = CTreeVariable::getDefinitions();
    
    // Throw the defs up into a map indexed by name:
    
    std::map<std::string, const CTreeVariable::Definition*> defmap;
    for (auto& item : defs) {
        defmap[item.first] = item.second;
    }
    // Now iterate over the parameters from file and check:
    
    const Variable* pV = pItem->s_variables;
    for (int i = 0; i < pItem->s_numVars; i++) {
        
        
        std::string name = pV->s_variableName;
        std::string units= pV->s_variableUnits;
        double      v    = pV->s_value;
        
        auto iter = defmap.find(name);              // From vars2 we know this works.
        EQ(units, iter->second->s_units);
        EQ(v, iter->second->s_value);
        
        const std::uint8_t* p8 = reinterpret_cast<const std::uint8_t*>(pV);
        p8 += sizeof(Variable) + name.size() + 1;
        pV = reinterpret_cast<const Variable*>(p8);
    }
}
// Because of when stuff is sent the begin should be right after the
// vars -- no assurance the end is at the end of the file, however.

void worker2test::state_1() {
    auto info = m_pReader->getBlock(32768);
    nextItem(info);
    const RingItemHeader* pItem =
        reinterpret_cast<const RingItemHeader*>(nextItem(info));
    // Minimal begin run _Must_ follow the def records.
    EQ(BEGIN_RUN, pItem->s_type);
    EQ(sizeof(RingItemHeader), size_t(pItem->s_size));
    EQ(sizeof(std::uint32_t), size_t(pItem->s_unused));
}
// Somewhere before the end there is a minimal end run as well - only one:

void worker2test::state_2() {
    auto info = m_pReader->getBlock(32768);
    const RingItemHeader* pItem =
        reinterpret_cast<const RingItemHeader*>(info.s_pData);
    unsigned count(0);
    
    while (pItem) {
        if (pItem->s_type == END_RUN) {
            count++;
            EQ(sizeof(RingItemHeader), size_t(pItem->s_size));
            EQ(sizeof(std::uint32_t), size_t(pItem->s_unused));
        }
        pItem = reinterpret_cast<const RingItemHeader*>(nextItem(info));
    }
    EQ(unsigned(1), count);
}
// should be numEvents items of type PARAMETER_DATA

void worker2test::data_1() {
    auto info = m_pReader->getBlock(32768);
    const RingItemHeader* pItem =
        reinterpret_cast<const RingItemHeader*>(info.s_pData);
    unsigned count(0);
    
    while (pItem) {
        if (pItem->s_type == PARAMETER_DATA) count++;
        pItem = reinterpret_cast<const RingItemHeader*>(nextItem(info));
    }
    EQ(numEvents, count);
}