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
using namespace frib::analysis;
#undef private
#include "DataReader.h"
#include "AnalysisRingItems.h"
#include "TreeParameter.h"
#include "TreeVariable.h"

static const char* templateFilename="testXXXXXX.dat";


class writertest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(writertest);
    CPPUNIT_TEST(construct_1);
    CPPUNIT_TEST(construct_2);
    CPPUNIT_TEST(construct_3);
    CPPUNIT_TEST(construct_4);
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

    }
protected:
    void construct_1();
    void construct_2();
    void construct_3();
    void construct_4();
};

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