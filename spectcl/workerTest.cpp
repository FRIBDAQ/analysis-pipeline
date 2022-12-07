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

/** @file:  workerTest.cpp
 *  @brief: Unit tests for CSpecTclWorker
 */
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "EventProcessor.h"
#define private public
#include "SpecTclWorker.h"
#include "TreeParameter.h"
#undef private


using namespace frib::analysis;

// 'Fancy' event processor - open to simplify for tests.
struct MyEp : public CEventProcessor {

    unsigned       m_callCount;
    unsigned       m_increment;
    bool           m_initialized;
    bool           m_connected;
    std::string    m_source;
    CTreeParameter m_param;
    
    MyEp(const char* name, unsigned increment);
    ~MyEp() {}
    
    Bool_t operator()(const Address_t pEvent,
                            CEvent& rEvent,
                            CAnalyzer& rAnalyzer,
                            CBufferDecoder& rDecoder);
    Bool_t OnEventSourceOpen(std::string name);
    Bool_t OnInitialize();

};

// Implement MyEp:

MyEp::MyEp(const char* name, unsigned increment) :
    m_callCount(0), m_increment(increment), m_initialized(false),
    m_connected(false), m_param(std::string(name)) {}

Bool_t MyEp::operator()(
    const Address_t pEvent,
    CEvent& rEvent,
    CAnalyzer& rAnalyzer,
    CBufferDecoder& rDecoder
) {
    m_param = m_callCount * m_increment;
    m_callCount++;
    
    return kfTRUE;
}

Bool_t MyEp::OnEventSourceOpen(std::string name) {
    m_connected = true;
    m_source = name;
    return kfTRUE;
}

Bool_t MyEp::OnInitialize() {
    m_initialized = true;
    return kfTRUE;
}

// We also need a mock for AbstractApplication.
// this is done this way because AbstractApplication is too entwined with MPI.

class frib::analysis::AbstractApplication {
public:
   AbstractApplication(int argc, char** argv) {}
};

// Test Suite:

class spworkertest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(spworkertest);
    CPPUNIT_TEST(test_1);
    CPPUNIT_TEST_SUITE_END();
    
private:
    AbstractApplication*          m_pApp;
    CSpecTclWorker* m_pWorker;
public:
    void setUp() {
        m_pApp = new frib::analysis::AbstractApplication(0, nullptr);
        m_pWorker = new CSpecTclWorker(*m_pApp);
                                                             // make an actual app.
        
        // Clear the tree parameter defs etc.:
        
        CTreeParameter::m_parameterDictionary.clear();
        CTreeParameter::m_event.clear();
        CTreeParameter::m_scoreboard.clear();
    }
    void tearDown() {
        delete m_pWorker;
        delete m_pApp;
    }
protected:
    void test_1();
};

CPPUNIT_TEST_SUITE_REGISTRATION(spworkertest);

void spworkertest::test_1()
{
}