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

/** @file:  writertests.cpp
 *  @brief: Test CDataWriter.
 */
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <new>
#include <stdexcept>
#include <string>
#include <string.h>

#define private public
#include "DataWriter.h"
#include "TreeParameter.h"
#include "TreeParameterArray.h"
#include "TreeVariable.h"
#include "TreeVariableArray.h"
#undef private

#include "DataReader.h"
#include "AnalysisRingItems.h"


using namespace frib::analysis;
static const char* templateFilename="testXXXXXX.dat";


class writertest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(writertest);
    CPPUNIT_TEST(construct_1);
    CPPUNIT_TEST(construct_2);
    CPPUNIT_TEST(construct_3);
    CPPUNIT_TEST(construct_4);
    CPPUNIT_TEST(construct_5);
    CPPUNIT_TEST(construct_6);
    CPPUNIT_TEST(construct_7);
    
    CPPUNIT_TEST(write_1);
    CPPUNIT_TEST(write_2);
    CPPUNIT_TEST(write_3);
    
    CPPUNIT_TEST(writepars_1);
    CPPUNIT_TEST_SUITE_END();
    
private:
    int m_fd;
    std::string m_filename;
public:
    void setUp() {
        // Make a temporary file in which the data can be written.

        char ftemplate[100];
        strncpy(ftemplate, templateFilename, sizeof(ftemplate));
        m_fd = mkstemps(ftemplate, 4);     // 4 '.dat'
        if (m_fd < 0) {
            std::string failmsg = "Failed to make tempfile: ";
            failmsg += strerror(errno);
            throw std::runtime_error(failmsg);
        }
        m_filename = ftemplate;

    }
    void tearDown() {
         // CLose the tempfile and remove it...b/c/ temp files aren't atually temp

        close(m_fd);      // Might have been closed in test so don't check status
        unlink(m_filename.c_str());
        CTreeParameter::m_parameterDictionary.clear();
        CTreeVariable::m_dictionary.clear();
    }
protected:
    void construct_1();
    void construct_2();
    void construct_3();
    void construct_4();
    void construct_5();
    void construct_6();
    void construct_7();
    
    void write_1();
    void write_2();
    void write_3();
    
    void writepars(1);
private:
        void* makeCountingRingItem(
            void* pBuffer,
            std::uint32_t totalSize, std::uint8_t first, std::uint8_t step
        );
        const void* skipItems(const void* pBuffer, size_t nItems=1);
};

/**
 * skipItems
 * 
 *   Skip buffered ring item(s).
 * @param pBuffer - Buffer containing a sequence of ring items.
 * @param nItems - number of items to skip.
 * @note the caller is responsible for determining there are at least nItems
 * @return void* pointer to the ring item after skippgin nItesm in the buffer.
 *
 */
const void*
writertest::skipItems(const void* pBuffer, size_t nItems) {
    for (int i =0; i < nItems; i++) {
        union {
            const std::uint8_t* p8;
            const RingItemHeader* ph;
        } p;
        p.p8 = reinterpret_cast<const std::uint8_t*>(pBuffer);
        p.p8 += p.ph->s_size;
        pBuffer = p.p8;
    }
    return pBuffer;
}
/**
 * makeCountingRingItem
 *    Create a counting ring item.
 *  @param pBuffer - user buffer must bet at least totalSize bytes of storage.
 *  @param totalSize - Total number of bytes in the ring item to create.
 *  @param first    - Value of first byte of body.
 *  @param step     - Next item step added to prior.
 *  @return void*    - pBuffer
 *
 *  @note the item type will be TEST_DATA, of course.
 */
void*
writertest::makeCountingRingItem(
        void* pBuffer,
        std::uint32_t totalSize, std::uint8_t first, std::uint8_t step
) {
    ASSERT(totalSize >= sizeof(RingItemHeader));   // At least an empty item.
    pRingItemHeader pHeader = reinterpret_cast<pRingItemHeader>(pBuffer);
    pHeader->s_size = totalSize;
    pHeader->s_type = TEST_DATA;
    pHeader->s_unused = sizeof(std::uint32_t);
    
    pHeader++;
    std::uint8_t* p = reinterpret_cast<std::uint8_t*>(pHeader);
    totalSize -= sizeof(RingItemHeader);                // Remaining bytes:
    
    for (int i =0; i < totalSize; i++) {
        *p++ = first;
        first += step;
    }
    return pBuffer;
}


CPPUNIT_TEST_SUITE_REGISTRATION(writertest);

// Construct from filename
void writertest::construct_1()
{
    CPPUNIT_ASSERT_NO_THROW(CDataWriter w(m_filename.c_str()));
}
// construct from fd

void writertest::construct_2() {
    CPPUNIT_ASSERT_NO_THROW(CDataWriter w(m_fd));
}
// Be sure the preamble is written -- in this case minimal items.

void writertest::construct_3() {
    {
        CDataWriter w(m_filename.c_str());
    }                                 // Closed file/flush etc.
    CDataReader r(m_fd, 8192*10);     // nice big buffer.
    
    auto rr = r.getBlock(8192*10);     // Slurp it all in.
    
    EQ(size_t(2), rr.s_nItems);       // Params and variables.
    
    /// Params:
    
    const ParameterDefinitions* pParams =
        reinterpret_cast<const ParameterDefinitions*>(rr.s_pData);
    ASSERT(pParams);
    
    EQ(sizeof(ParameterDefinitions), size_t(pParams->s_header.s_size));
    EQ(PARAMETER_DEFINITIONS, pParams->s_header.s_type);
    EQ(std::uint32_t(0), pParams->s_numParameters);
    
    /// Variables:
    
    const std::uint8_t* p = reinterpret_cast<const uint8_t*>(pParams);
    p += pParams->s_header.s_size;
    const VariableItem* pVars = reinterpret_cast<const VariableItem*>(p);
    EQ(sizeof(VariableItem), size_t(pVars->s_header.s_size));
    EQ(VARIABLE_VALUES, pVars->s_header.s_type);
    EQ(std::uint32_t(0), pVars->s_numVars);
    
}
// there's a tree parameter:

void writertest::construct_4() {
    CTreeParameter param("a", "mm");
    
    {
        CDataWriter w(m_filename.c_str());
    }
    // There should be an entry in the parameter descriptor:
    
    CDataReader r(m_fd, 8192*10);     // nice big buffer.
    
    auto rr = r.getBlock(8192*10);     // Slurp it all in.
    
    EQ(size_t(2), rr.s_nItems);       // Params and variables.
    
    /// Params:
    
    const ParameterDefinitions* pParams =
        reinterpret_cast<const ParameterDefinitions*>(rr.s_pData);
    ASSERT(pParams);
    
    
    EQ(PARAMETER_DEFINITIONS, pParams->s_header.s_type);
    EQ(std::uint32_t(1), pParams->s_numParameters);
    auto defs = CTreeParameter::getDefinitions();
    EQ(defs[0].second.s_parameterNumber, pParams->s_parameters[0].s_parameterNumber);
    EQ(0, strcmp(defs[0].first.c_str(), pParams->s_parameters[0].s_parameterName));
    
    // I'm not sure why but if optimization is -O2 the following fails to
    // put name to be "a" but instead leaves it as an empty string:
    //std::string name = std::string(pParams->s_parameters[0].s_parameterName);
    //EQ(defs[0].first, name);
    
    
    
}
// A few tree parameter defs - using an array:

void writertest::construct_5() {
        CTreeParameterArray a("a", "mm", 16, 0);
        {
            CDataWriter w(m_filename.c_str());
        }                                    // CLosed.
        CDataReader reader(m_fd, 8192*10);
        auto r = reader.getBlock(8192*10);   // Slurp it all in.
        
        EQ(size_t(2), r.s_nItems);  // got boht.
        
        const ParameterDefinitions* pParams =
            reinterpret_cast<const ParameterDefinitions*>(r.s_pData);
        EQ(PARAMETER_DEFINITIONS, pParams->s_header.s_type);
        EQ(std::uint32_t(16), pParams->s_numParameters);
        
        const ParameterDefinition* p = pParams->s_parameters;
        union {
            const std::uint8_t* p8;
            const ParameterDefinition* pv;
        } pp;
        pp.pv = p;
        for (int i = 0; i <16; i++) {
            
            EQ(a[i].getId(), pp.pv->s_parameterNumber);
            EQ(0, strcmp(a[i].getName().c_str(), pp.pv->s_parameterName));
            pp.p8 += sizeof(ParameterDefinition) + strlen(pp.pv->s_parameterName) + 1;
        }
        
}
// Single tree variable:
void writertest::construct_6() {
    CTreeVariable a("a", "mm");
    a = 3.1416;                          // Give it a value.
    
    {
        CDataWriter w(m_filename.c_str());
    }                                    // CLosed.
    CDataReader reader(m_fd, 8192*10);
    auto r = reader.getBlock(8192*10);   // Slurp it all in.
    
    EQ(size_t(2), r.s_nItems);  // got both
    
    // Skip the first one:
    
    union {
        const std::uint8_t* p8;
        const RingItemHeader* ph;
    } pItem;
    pItem.ph = reinterpret_cast<const RingItemHeader*>(r.s_pData);
    pItem.p8 += pItem.ph->s_size;
    
    //  Item should be a variable def item with 1 variable:
    
    EQ(VARIABLE_VALUES, pItem.ph->s_type);
    union {
        const std::uint8_t* p8;
        const VariableItem* pv;
    } pv;
    pv.p8 = pItem.p8;
    EQ(std::uint32_t(1), pv.pv->s_numVars);
    
    
    union {
        const std::uint8_t* p8;
        const Variable*     pv;
    } p;
    p.pv = pv.pv->s_variables;
    
    EQ(double(a), p.pv->s_value);
    EQ(0, strcmp(a.getUnit().c_str(), p.pv->s_variableUnits));
    EQ(0, strcmp(a.getName().c_str(), p.pv->s_variableName));
    
}
// multiple tree variables:

void writertest::construct_7()
{
    CTreeVariableArray a("a", 1.2345, "mm", 16, 0);
    {
        CDataWriter w(m_filename.c_str());
    }                                    // CLosed.
    CDataReader reader(m_fd, 8192*10);
    auto r = reader.getBlock(8192*10);   // Slurp it all in.
    
    EQ(size_t(2), r.s_nItems);  // got both
    
    // Skip the first one:
    
    union {
        const std::uint8_t* p8;
        const RingItemHeader* ph;
    } pItem;
    pItem.ph = reinterpret_cast<const RingItemHeader*>(r.s_pData);
    pItem.p8 += pItem.ph->s_size;
    
    //  Item should be a variable def item with 1 variable:
    
    EQ(VARIABLE_VALUES, pItem.ph->s_type);
    union {
        const std::uint8_t* p8;
        const VariableItem* pv;
    } pv;
    pv.p8 = pItem.p8;
    EQ(std::uint32_t(16), pv.pv->s_numVars);
    
    union {
        const std::uint8_t* p8;
        const Variable*     pv;
    } p;
    p.pv = pv.pv->s_variables;
    
    for (int i =0; i < 16; i++) {
        EQ(double(a[i]), p.pv->s_value);
        EQ(0, strcmp(a[i].getUnit().c_str(), p.pv->s_variableUnits));
        EQ(0, strcmp(a[i].getName().c_str(), p.pv->s_variableName));
        p.p8 += sizeof(Variable) + strlen(p.pv->s_variableName) +1;
    }
}
// Write an empty ring item:

void writertest::write_1()
{
    RingItemHeader item;
    void* pItem = makeCountingRingItem(&item, sizeof(item), 0, 0);
    {
        CDataWriter w(m_filename.c_str());
        w.writeItem(pItem);
    }
    // The file should have three items - empty parameter defs, empty var values
    // and the empty ring item in item.
    
    CDataReader reader(m_fd, 8192*10);
    auto r = reader.getBlock(8192*10);
    EQ(size_t(3), r.s_nItems);
    
    const void* pReadItem = skipItems(r.s_pData, 2); // s.b. item.
    EQ(0, memcmp(pItem, pReadItem, sizeof(RingItemHeader)));
    
}
// Write a ring item with some contents:

void writertest::write_2()
{
    std::uint32_t item[8192];
    void* pItem = makeCountingRingItem(item, 100, 1, 1);
    {
        CDataWriter w(m_filename.c_str());
        w.writeItem(pItem);
    }
    // The file should have three items - empty parameter defs, empty var values
    // and the empty ring item in item.
    
    CDataReader reader(m_fd, 8192*10);
    auto r = reader.getBlock(8192*10);
    EQ(size_t(3), r.s_nItems);
    
    const void* pReadItem = skipItems(r.s_pData, 2); // s.b. item.
    
    // Ensure the size is right:
    
    const RingItemHeader* pHeader = reinterpret_cast<const RingItemHeader*>(pReadItem);
    EQ(size_t(100), size_t(pHeader->s_size));
    EQ(TEST_DATA, pHeader->s_type);
    EQ(sizeof(std::uint32_t), size_t(pHeader->s_unused));
    
    EQ(0, memcmp(item, pReadItem, 100));
    
}
// write a couple non-empty ring items
void writertest::write_3() {
    std::uint32_t item1[200];
    std::uint32_t item2[100];
    void* pItem1 = makeCountingRingItem(item1, 200, 1, 1);
    void* pItem2 = makeCountingRingItem(item2, 100, 1, 2);
    
    {
        CDataWriter w(m_filename.c_str());
        w.writeItem(item1);
        w.writeItem(item2);
    }
    // The file should have three items - empty parameter defs, empty var values
    // and the empty ring item in item.
    
    CDataReader reader(m_fd, 8192*10);
    auto r = reader.getBlock(8192*10);
    EQ(size_t(4), r.s_nItems);
    
    const void* pReadItem = skipItems(r.s_pData, 2); // s.b. item.
    const RingItemHeader* pHeader = reinterpret_cast<const RingItemHeader*>(pReadItem);
    EQ(size_t(200), size_t(pHeader->s_size));
    EQ(TEST_DATA, pHeader->s_type);
    EQ(sizeof(std::uint32_t), size_t(pHeader->s_unused));
    EQ(0, memcmp(item1, pReadItem, 200));
    
    pReadItem = skipItems(pReadItem);
    pHeader = reinterpret_cast<const RingItemHeader*>(pReadItem);
    EQ(size_t(100), size_t(pHeader->s_size));
    EQ(TEST_DATA, pHeader->s_type);
    EQ(sizeof(std::uint32_t), size_t(pHeader->s_unused));
    EQ(0, memcmp(item2, pReadItem, 100));
}