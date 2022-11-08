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

/** @file:  tclconfigtests.cpp
 *  @brief: Test the TCLParameterReader
 */
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "TCLParameterReader.h"
#include <stdlib.h>
#include <string>
#include <stdexcept>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sstream>
#include <iomanip>

#define private public
#include "TreeParameter.h"
#include "TreeVariable.h"
#undef private
using namespace frib::analysis;

// script template:

const char* scriptNameTemplate="/tmp/test_scriptXXXXXX.tcl";

class TclConfigtest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(TclConfigtest);
    CPPUNIT_TEST(empty);
    CPPUNIT_TEST(treeparam_1);
    CPPUNIT_TEST(treeparam_2);
    CPPUNIT_TEST(treeparam_3);
    
    CPPUNIT_TEST(treeparamarray_1);
    CPPUNIT_TEST_SUITE_END();
protected:
    void empty();
    void treeparam_1();
    void treeparam_2();
    void treeparam_3();
    
    void treeparamarray_1();
    
private:
    std::string m_filename;
    int         m_fd;    
public:
    void setUp() {
        // Create a temporary file that will be the script:
        
        char scriptName[100];
        strncpy(scriptName, scriptNameTemplate, sizeof(scriptName));
        m_fd = mkstemps(scriptName, 4);     // .tcl - four chars.
        if (m_fd == -1) {
            throw std::runtime_error(strerror(errno));
        }
        m_filename = scriptName;
    }
    void tearDown() {
        CTreeParameter::m_parameterDictionary.clear();
        CTreeVariable::m_dictionary.clear();
        
        unlink(m_filename.c_str());
    }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TclConfigtest);

// Empty file parses ok and leaves empty dictionaries:

void TclConfigtest::empty()
{
    close(m_fd);
    CTCLParameterReader reader(m_filename.c_str());
    CPPUNIT_ASSERT_NO_THROW(reader.read());
    
    ASSERT(CTreeParameter::m_parameterDictionary.empty());
    ASSERT(CTreeVariable::m_dictionary.empty());
}
// file with single treeparameter command:
void TclConfigtest::treeparam_1() {
    const char* script =
        "treeparameter test 0 1024 512 none\n";
    write(m_fd, script, strlen(script));
    close(m_fd);
    
    // This is not factored so that if it fails we know which test failed.
    
    CTCLParameterReader reader(m_filename.c_str());
    CPPUNIT_ASSERT_NO_THROW(reader.read());
    
    auto defs = CTreeParameter::getDefinitions();
    EQ(size_t(1), defs.size());
    EQ(std::string("test"), defs[0].first);
    CTreeParameter::SharedData& d(defs[0].second);
    EQ(0.0, d.s_low);
    EQ(1024.0, d.s_high);
    EQ(unsigned(512), d.s_chans);
    EQ(std::string("none"), d.s_units);
    
}
// file with error in treeparam command:

void TclConfigtest::treeparam_2() {
    const char* script =
        "treeparameter test 0 1024 512 none extra\n";
    write(m_fd, script, strlen(script));
    close(m_fd);
    
    CTCLParameterReader reader(m_filename.c_str());
    CPPUNIT_ASSERT_THROW(reader.read(), std::runtime_error);
    
}
// multiple defs:

void TclConfigtest::treeparam_3() {
    const char* script =
        "treeparameter a 0 1024 512 none\n\
        treeparameter b -1.5 1.5 1024 mm\n";
    write(m_fd, script, strlen(script));
    close(m_fd);
    
    CTCLParameterReader reader(m_filename.c_str());
    CPPUNIT_ASSERT_NO_THROW(reader.read());
    
    auto defs = CTreeParameter::getDefinitions();
    EQ(size_t(2), defs.size());
    
    // SHould be in lexical order since it comes from iterating over a map:
    {
        auto p = defs[0];
        auto d(p.second);     // Copy construction
        EQ(std::string("a"), p.first);
        EQ(0.0, d.s_low);
        EQ(1024.0, d.s_high);
        EQ(unsigned(512), d.s_chans);
        EQ(std::string("none"), d.s_units);
    }
    {
        auto p = defs[1];
        auto d(p.second);     // Copy construction
        EQ(std::string("b"), p.first);
        EQ(-1.5, d.s_low);
        EQ(1.5,  d.s_high);
        EQ(unsigned(1024), d.s_chans);
        EQ(std::string("mm"), d.s_units);
    }
    
}
// single treeeparameterarray definiition

void TclConfigtest::treeparamarray_1()
{
    const char* script =
        "treeparameterarray test -1.0 1.0 100 mm 16 0\n";
    write(m_fd, script, strlen(script));
    close(m_fd);
    
    CTCLParameterReader reader(m_filename.c_str());
    CPPUNIT_ASSERT_NO_THROW(reader.read());
    
    auto defs = CTreeParameter::getDefinitions();
    EQ(size_t(16), defs.size());     // 16 element 'array'
    for (int i =0; i < defs.size(); i++) {
        auto def = defs[i];
        auto info = def.second;
        std::stringstream namestream;
        namestream <<"test." << std::setw(2) << std::setfill('0') << i;
        std::string name = namestream.str();
        EQ(name, def.first);
        EQ(-1.0, info.s_low);
        EQ(1.0, info.s_high);
        EQ(unsigned(100), info.s_chans);
        EQ(std::string("mm"), info.s_units);
    }
}