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
    CPPUNIT_TEST(construct_1);
    CPPUNIT_TEST(add_1);
    CPPUNIT_TEST(add_2);
    CPPUNIT_TEST(add_3);
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
    void construct_1();
    void add_1();
    void add_2();
    void add_3();
};

CPPUNIT_TEST_SUITE_REGISTRATION(spworkertest);

// Initial data correct on construction:
void spworkertest::construct_1()
{
    ASSERT(m_pWorker->m_pipeline.empty());
    EQ(unsigned(0), m_pWorker->m_unNamedIndex);
    ASSERT(m_pWorker->m_pDecoder);
    ASSERT(m_pWorker->m_pAnalyzer);
    ASSERT(m_pWorker->m_pEvent);
}
// Add named event processor.
void spworkertest::add_1() {
    MyEp ep("param", 1);
    std::string name = m_pWorker->addProcessor(&ep, "name");
    EQ(std::string("name"), name);
    EQ(size_t(1), m_pWorker->m_pipeline.size());
    auto info = m_pWorker->m_pipeline[0];
    EQ(std::string("name"), info.first);
    EQ(reinterpret_cast<CEventProcessor*>(&ep), info.second);
}
// add unnmamed:

void spworkertest::add_2() {
    MyEp ep("param", 1);
    auto name = m_pWorker->addProcessor(&ep);
    EQ(std::string("_Unamed_.0"), name);
}
// multiple unamed are unique:

void spworkertest::add_3() {
    MyEp ep1("param1", 1);
    MyEp ep2("param2", 2);
    
    auto name1 = m_pWorker->addProcessor(&ep1);
    auto name2 = m_pWorker->addProcessor(&ep2);
    
    EQ(std::string("_Unamed_.0"), name1);
    EQ(std::string("_Unamed_.1"), name2);
}