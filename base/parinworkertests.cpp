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

/** @file:  parinworkertests.cpp
 *  @brief:  Test the output file from the worker for testParinput
 */
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "AnalysisRingItems.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sstream>

using namespace frib::analysis;

extern std::string workerFile;
extern int         numberEvents;

const size_t BUFFER_SIZE = 1024*1024*32;
class parinworkertest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(parinworkertest);
    CPPUNIT_TEST(params_1);
    CPPUNIT_TEST(vars_1);
    CPPUNIT_TEST(events_1);
    CPPUNIT_TEST(events_2);
    CPPUNIT_TEST(events_3);
    CPPUNIT_TEST_SUITE_END();
    
private:
    int m_fd;
    std::uint8_t*   m_contents;
    size_t         m_nBytes;
public:
    void setUp() {
        m_contents = new std::uint8_t[BUFFER_SIZE];
        m_fd = open(workerFile.c_str(), O_RDONLY);
        ASSERT(m_fd >= 0);
        m_nBytes = read(m_fd, m_contents, BUFFER_SIZE);
        ASSERT(m_nBytes > 0);
    }
    void tearDown() {
        close(m_fd);
        delete []m_contents;
    }
protected:
    void params_1();
    void vars_1();
    void events_1();
    void events_2();
    void events_3();
private:
    const void*  skipParams();
    const void*  skipDefs();
    const void*  nextEvent(const void* pEvent);
};

CPPUNIT_TEST_SUITE_REGISTRATION(parinworkertest);

// Skip the parameter record and return a pointer to the next record:

const void* parinworkertest::skipParams()
{
    const std::uint32_t* pN = reinterpret_cast<const std::uint32_t*>(m_contents);
    std::uint32_t numItems = *pN;
    pN++;
    const FRIB_MPI_ParameterDef* pDef =
        reinterpret_cast<const FRIB_MPI_ParameterDef*>(pN);
    return pDef + numItems;
}
// Skep parameter and variable definition records.

const void* parinworkertest::skipDefs()
{
    const std::uint32_t* pN = reinterpret_cast<const std::uint32_t*>(skipParams());
    std::uint32_t numItems = *pN;
    pN++;
    const FRIB_MPI_VariableDef* pDef =
        reinterpret_cast<const FRIB_MPI_VariableDef*>(pN);
    return pDef + numItems;
}
// skip an event:
const void* parinworkertest::nextEvent(const void* pEvent) {
    const FRIB_MPI_Parameter_MessageHeader* pHeader =
            reinterpret_cast<const FRIB_MPI_Parameter_MessageHeader*>(pEvent);
    const std::uint8_t* p = reinterpret_cast<const std::uint8_t*>(pEvent);
    
    return p + sizeof(FRIB_MPI_Parameter_MessageHeader) +
        pHeader->s_numParameters * sizeof(FRIB_MPI_Parameter_Value);
}

// First item should define all of the parameters:
void parinworkertest::params_1()
{
    const std::uint32_t* pN = reinterpret_cast<const std::uint32_t*>(m_contents);
    EQ(std::uint32_t(17), *pN);
    pN++;                              // The items.
    
    const FRIB_MPI_ParameterDef* pDef =
        reinterpret_cast<const FRIB_MPI_ParameterDef*>(pN);
 
    long id = 1;
    EQ(0, strcmp("scalar", pDef->s_name));
    EQ(id, pDef->s_parameterId);
    
    // Now the 16 elements of the array:
    //
    for (int i =0; i < 16; i++) {
        pDef++;
        id++;
        
        std::stringstream sname;
        sname << "array." << i;
        std::string name = sname.str();
        EQ(name, std::string(pDef->s_name));
        EQ(id, pDef->s_parameterId);
    }
}
// Next should be the vars item:

void parinworkertest::vars_1()
{
    const std::uint32_t* pN = reinterpret_cast<const std::uint32_t*>(skipParams());
    
    EQ(std::uint32_t(17), *pN);
    pN++;
    
    const FRIB_MPI_VariableDef* pDef =
        reinterpret_cast<const FRIB_MPI_VariableDef*>(pN);
    
    // first is "slope"  then an array of offset.n
    
    EQ(std::string("slope"), std::string(pDef->s_name));
    EQ(std::string("unitless"), std::string(pDef->s_variableUnits));
    EQ(double(1.234), pDef->s_value);
    
    for (int i =0; i < 16; i++) {
        pDef++;
        std::stringstream sname;
        sname << "offset." << i;
        std::string name = sname.str();
        EQ(name, std::string(pDef->s_name));
        EQ(std::string("mm"), std::string(pDef->s_variableUnits));
        EQ(double(i), pDef->s_value);
    }
    
}
// should be numberEvents events:

void parinworkertest::events_1()
{
    const std::uint8_t* pEvent = reinterpret_cast<const std::uint8_t*>(skipDefs());
    
    size_t bytesLeft = m_nBytes;
    bytesLeft -= (pEvent - m_contents);        // Bytes of event data:
    
    int numEvents(0);
    while (bytesLeft) {
        numEvents++;
        
        const FRIB_MPI_Parameter_MessageHeader* pHeader =
            reinterpret_cast<const FRIB_MPI_Parameter_MessageHeader*>(pEvent);
        unsigned size = sizeof(FRIB_MPI_Parameter_MessageHeader) +
            pHeader->s_numParameters * sizeof(FRIB_MPI_Parameter_Value);
        pEvent += size;
        bytesLeft -= size;
    }
    EQ(numberEvents, numEvents);
    
}
// triggers count and the size of the event is a rollovery thing
//
void parinworkertest::events_2() {
    const FRIB_MPI_Parameter_MessageHeader* pHeader =
            reinterpret_cast<const FRIB_MPI_Parameter_MessageHeader*>(skipDefs());
    
    for (int i =0; i < numberEvents; i++) {
        EQ(std::uint64_t(i), pHeader->s_triggerNumber);
        EQ(std::uint32_t( i % 16 + 1), pHeader->s_numParameters);
        
        pHeader =
            reinterpret_cast<const FRIB_MPI_Parameter_MessageHeader*>(nextEvent(pHeader));
            
    }
}
void parinworkertest::events_3() {
     const FRIB_MPI_Parameter_MessageHeader* pHeader =
            reinterpret_cast<const FRIB_MPI_Parameter_MessageHeader*>(skipDefs());
    
    for (int i =0; i < numberEvents; i++) {
        const FRIB_MPI_Parameter_Value* pValue =
            reinterpret_cast<const FRIB_MPI_Parameter_Value*>(pHeader+1);
        for (int p =0; p < pHeader->s_numParameters; p++) {
            EQ(std::uint32_t(p), pValue->s_number);
            EQ(double(p*10), pValue->s_value);
            
            pValue++;
        }
        
        pHeader =
            reinterpret_cast<const FRIB_MPI_Parameter_MessageHeader*>(nextEvent(pHeader));
            
    }
}