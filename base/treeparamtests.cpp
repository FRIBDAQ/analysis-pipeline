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

/** @file:  treeparamtests.cpp
 *  @brief: Test of CTreeParameter
 */
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#define private public
#include "TreeParameter.h"
#undef private


using namespace frib::analysis;

class TPTest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(TPTest);
    CPPUNIT_TEST(initial);
    CPPUNIT_TEST(next_1);
    CPPUNIT_TEST(next_2);
    CPPUNIT_TEST(next_3);
    CPPUNIT_TEST(next_4);
    
    CPPUNIT_TEST(collect_1);
    CPPUNIT_TEST(collect_2);
    
    CPPUNIT_TEST(dlimits);
    CPPUNIT_TEST(dbins);
    CPPUNIT_TEST(dunits);
    
    CPPUNIT_TEST(getEvent);
    CPPUNIT_TEST(getsb);
    
    CPPUNIT_TEST(construct_1);
    CPPUNIT_TEST(construct_2);
    CPPUNIT_TEST(construct_3);
    CPPUNIT_TEST(construct_4);
    CPPUNIT_TEST(construct_5);
    CPPUNIT_TEST(construct_6);
    CPPUNIT_TEST(construct_7);
    CPPUNIT_TEST(construct_8);
    CPPUNIT_TEST(construct_9);
    
    CPPUNIT_TEST(init_1);
    CPPUNIT_TEST(init_2);
    CPPUNIT_TEST(init_3);
    CPPUNIT_TEST(init_4);
    CPPUNIT_TEST(init_5);
    
    CPPUNIT_TEST(dup_1);
    CPPUNIT_TEST(dup_2);
    
    CPPUNIT_TEST(cvtdouble_1);
    CPPUNIT_TEST(cvtdouble_2);
    CPPUNIT_TEST(cvtdouble_3);
    
    CPPUNIT_TEST(assign_1);
    CPPUNIT_TEST(assign_2);
    CPPUNIT_TEST(assign_3);
    CPPUNIT_TEST(assign_4);
    CPPUNIT_TEST(assign_5);
    CPPUNIT_TEST(assign_6);
    CPPUNIT_TEST(assign_7);
    
    CPPUNIT_TEST(pluseq_1);
    CPPUNIT_TEST(pluseq_2);
    CPPUNIT_TEST(pluseq_3);
    
    CPPUNIT_TEST(minuseq_1);
    CPPUNIT_TEST(minuseq_2);
    CPPUNIT_TEST(minuseq_3);
    
    CPPUNIT_TEST(timeseq_1);
    CPPUNIT_TEST(timeseq_2);
    CPPUNIT_TEST(timeseq_3);

    CPPUNIT_TEST(diveq_1);
    CPPUNIT_TEST(diveq_2);
    CPPUNIT_TEST(diveq_3);
    
    CPPUNIT_TEST(postinc_1);
    CPPUNIT_TEST(postinc_2);
    CPPUNIT_TEST(postinc_3);
    
    CPPUNIT_TEST(preinc_1);
    CPPUNIT_TEST(preinc_2);
    CPPUNIT_TEST(preinc_3);
    
    CPPUNIT_TEST(postdec_1);
    CPPUNIT_TEST(postdec_2);
    CPPUNIT_TEST(postdec_3);
    
    CPPUNIT_TEST(predec_1);
    CPPUNIT_TEST(predec_2);
    CPPUNIT_TEST(predec_3);
    
    CPPUNIT_TEST(getname_1);
    CPPUNIT_TEST(getname_2);
    
    CPPUNIT_TEST(getid_1);
    CPPUNIT_TEST(getid_2);
    
    // A bit of white box testing here: getValue, setValue --
    // those are used by the arithmetic operators so they're implicitly
    // already tested.
    
    CPPUNIT_TEST(getbins_1);
    CPPUNIT_TEST(getbins_2);
    
    CPPUNIT_TEST(setbins_1);
    CPPUNIT_TEST(setbins_2);
    CPPUNIT_TEST_SUITE_END();
    
private:

public:
    void setUp() {
        
    }
    void tearDown() {
        CTreeParameter::m_parameterDictionary.clear();
        CTreeParameter::m_scoreboard.clear();
        CTreeParameter::m_generation = 1;
        CTreeParameter::m_nextId = 0;
        CTreeParameter::m_event.clear();
        CTreeParameter::m_defaultSpecification = {
            .s_low = 0,                 // Will need updating if it
            .s_high = 100,              // changes in TreeParameter.cpp
            .s_chans = 100,
            .s_units = "Chans"
        };
    }
protected:
    void initial();
    
    void next_1();
    void next_2();
    void next_3();
    void next_4();
    
    void collect_1();
    void collect_2();
    
    void dlimits();
    void dbins();
    void dunits();
    
    void getEvent();
    void getsb();
    
    void construct_1();
    void construct_2();
    void construct_3();
    void construct_4();
    void construct_5();
    void construct_6();
    void construct_7();
    void construct_8();
    void construct_9();
    
    // Initialize is only tested in cases where the original is  unbound
    // as constructors checked the other cases.
    
    void init_1();
    void init_2();
    void init_3();
    void init_4();
    void init_5();
    
    // Dup - test that duplicate names point to the same definition block:
    
    void dup_1();
    void dup_2();
    
    void cvtdouble_1();
    void cvtdouble_2();
    void cvtdouble_3();
    
    void assign_1();
    void assign_2();
    void assign_3();
    void assign_4();
    void assign_5();
    void assign_6();
    void assign_7();
    
    void pluseq_1();
    void pluseq_2();
    void pluseq_3();
    
    void minuseq_1();
    void minuseq_2();
    void minuseq_3();
    
    void timeseq_1();
    void timeseq_2();
    void timeseq_3();
    
    void diveq_1();
    void diveq_2();
    void diveq_3();
    
    void postinc_1();
    void postinc_2();
    void postinc_3();
    
    void preinc_1();
    void preinc_2();
    void preinc_3();
    
    void postdec_1();
    void postdec_2();
    void postdec_3();
    
    void predec_1();
    void predec_2();
    void predec_3();
    
    void getname_1();
    void getname_2();
    
    void getid_1();
    void getid_2();
    
    void getbins_1();
    void getbins_2();
    
    void setbins_1();
    void setbins_2();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TPTest);

// initial state of static stuff.
void TPTest::initial()
{
    EQ(std::uint64_t(1), CTreeParameter::m_generation);
    ASSERT(CTreeParameter::m_parameterDictionary.empty());
    EQ(unsigned(1), CTreeParameter::m_nextId);
    ASSERT(CTreeParameter::m_event.empty());
    ASSERT(CTreeParameter::m_scoreboard.empty());
    
    // these need to be updated if the initial default values
    // change.
    
    EQ(double(0), CTreeParameter::m_defaultSpecification.s_low);
    EQ(double(100), CTreeParameter::m_defaultSpecification.s_high);
    EQ(unsigned(100), CTreeParameter::m_defaultSpecification.s_chans);
    EQ(std::string("Chans"), CTreeParameter::m_defaultSpecification.s_units);
}
// nextEvent increments  the generation:

void TPTest::next_1() {
    CTreeParameter::nextEvent();
    EQ(std::uint64_t(2), CTreeParameter::m_generation);
    
}
// next event does not affect empties the parameter dictionary.

void TPTest::next_2() {
    CTreeParameter::m_parameterDictionary[std::string("test")] = CTreeParameter::m_defaultSpecification;
    CTreeParameter::nextEvent();
    
    EQ(size_t(1), CTreeParameter::m_parameterDictionary.size());
}
// next event does not modify m_nextId

void TPTest::next_3() {
    auto old = CTreeParameter::m_nextId;
    CTreeParameter::nextEvent();
    
    EQ(old, CTreeParameter::m_nextId);
}
// next event clears the scoreboard and does not touch the event:

void TPTest::next_4() {
    CTreeParameter::m_scoreboard.push_back(1);
    CTreeParameter::m_scoreboard.push_back(2);
    CTreeParameter::m_event.push_back(1234.5);
    CTreeParameter::m_event.push_back(3.1416);
    
    CTreeParameter::nextEvent();
    EQ(size_t(0), CTreeParameter::m_scoreboard.size());
    EQ(size_t(2), CTreeParameter::m_event.size());
}
// nothing to collect gives empty vector:
void TPTest::collect_1() {
    EQ(size_t(0), CTreeParameter::collectEvent().size());
}
// Put some stuff in event and a sparse scoreboard we get the right struff out.

void TPTest::collect_2() {
    std::vector<double> eventData = {1.0, 2.1, 3.2, 5.3, 7.5, 13.7}; // See the pattern?
    std::vector<unsigned> sbdata = {2, 3, 5};                        // A prime example.
    CTreeParameter::m_event.insert(CTreeParameter::m_event.begin(), eventData.begin(), eventData.end());
    CTreeParameter::m_scoreboard.insert(CTreeParameter::m_scoreboard.begin(), sbdata.begin(), sbdata.end());
    
    auto result = CTreeParameter::collectEvent();
    
    EQ(sbdata.size(), result.size());
    for (int i =0;i < result.size(); i++) {
        EQ(result[i].first, sbdata[i]);
        EQ(result[i].second, eventData[sbdata[i]]);
    }
}

// default limits

void TPTest::dlimits() {
    CTreeParameter::setDefaultLimits(-1.0, 1.0);
    
    EQ(double(-1.0), CTreeParameter::m_defaultSpecification.s_low);
    EQ(double(1.0), CTreeParameter::m_defaultSpecification.s_high);
}
// default bins:
void TPTest::dbins() {
    CTreeParameter::setDefaultBins(1024);
    EQ(unsigned(1024), CTreeParameter::m_defaultSpecification.s_chans);
}
void TPTest::dunits() {
    CTreeParameter::setDefaultUnits("furlong/fortnight");
    EQ(std::string("furlong/fortnight"), CTreeParameter::m_defaultSpecification.s_units);
}
// Get reference to event:

void TPTest::getEvent() {
    for (int i =0; i < 100; i++) {
        CTreeParameter::m_event.push_back(i);
    }
    auto& e = CTreeParameter::getEvent();
    EQ(size_t(100), e.size());
    
    for (int i =0;i < 100; i++) {
        EQ(double(i), e.at(i));
    }
}
// Get scoreboard

void TPTest::getsb() {
    std::vector<unsigned> sbdata = {2, 3, 5};                        // A prime example.
    CTreeParameter::m_scoreboard.insert(CTreeParameter::m_scoreboard.begin(), sbdata.begin(), sbdata.end());
    
    auto& s = CTreeParameter::getScoreboard();
    EQ(sbdata.size(), s.size());
    for (int i =0; i < s.size(); i++) {
        EQ(sbdata[i], s.at(i));
    }
}
// default constructor is unbound:

void TPTest::construct_1()
{
    CTreeParameter param;
    EQ(std::string(""), param.m_name);
    EQ(CTreeParameter::pSharedData(0), param.m_pDefinition);
}
// construct with name gets bound with default metadata:
void TPTest::construct_2() {
    CTreeParameter param("Test");
    EQ(std::string("Test"), param.m_name);
    ASSERT(param.m_pDefinition);
    
    EQ(unsigned(1), param.m_pDefinition->s_parameterNumber);
    EQ(CTreeParameter::m_defaultSpecification.s_low, param.m_pDefinition->s_low);
    EQ(CTreeParameter::m_defaultSpecification.s_high, param.m_pDefinition->s_high);
    EQ(CTreeParameter::m_defaultSpecification.s_chans, param.m_pDefinition->s_chans);
    EQ(CTreeParameter::m_defaultSpecification.s_units, param.m_pDefinition->s_units);
    EQ(false, param.m_pDefinition->s_changed);
    EQ(CTreeParameter::m_generation-1, param.m_pDefinition->s_generation);
    
}
// construct with name and units:
void TPTest::construct_3() {
    CTreeParameter param("Test", "mm");
    EQ(std::string("Test"), param.m_name);
    ASSERT(param.m_pDefinition);
    
    EQ(unsigned(1), param.m_pDefinition->s_parameterNumber);
    EQ(CTreeParameter::m_defaultSpecification.s_low, param.m_pDefinition->s_low);
    EQ(CTreeParameter::m_defaultSpecification.s_high, param.m_pDefinition->s_high);
    EQ(CTreeParameter::m_defaultSpecification.s_chans, param.m_pDefinition->s_chans);
    EQ(std::string("mm"), param.m_pDefinition->s_units);
    EQ(false, param.m_pDefinition->s_changed);
    EQ(CTreeParameter::m_generation-1, param.m_pDefinition->s_generation);
}
// Construct with low, high units.
void TPTest::construct_4() {
    CTreeParameter param ("Test", -1.0, 1.0, "mm");
    EQ(std::string("Test"), param.m_name);
    ASSERT(param.m_pDefinition);
    
    EQ(unsigned(1), param.m_pDefinition->s_parameterNumber);
    EQ(double(-1.0), param.m_pDefinition->s_low);
    EQ(double(1.0), param.m_pDefinition->s_high);
    EQ(CTreeParameter::m_defaultSpecification.s_chans, param.m_pDefinition->s_chans);
    EQ(std::string("mm"), param.m_pDefinition->s_units);
    EQ(false, param.m_pDefinition->s_changed);
    EQ(CTreeParameter::m_generation-1, param.m_pDefinition->s_generation);
}
// construct with low, high channels, units.
void TPTest::construct_5() {
    CTreeParameter param ("Test", 1024, -1.0, 1.0, "mm");
    
    EQ(std::string("Test"), param.m_name);
    ASSERT(param.m_pDefinition);
    
    EQ(unsigned(1), param.m_pDefinition->s_parameterNumber);
    EQ(double(-1.0), param.m_pDefinition->s_low);
    EQ(double(1.0), param.m_pDefinition->s_high);
    EQ(unsigned(1024), param.m_pDefinition->s_chans);
    EQ(std::string("mm"), param.m_pDefinition->s_units);
    EQ(false, param.m_pDefinition->s_changed);
    EQ(CTreeParameter::m_generation-1, param.m_pDefinition->s_generation);
}
// construct with reslution
void TPTest::construct_6() {
    CTreeParameter param("Test", 10);
    
    EQ(std::string("Test"), param.m_name);
    ASSERT(param.m_pDefinition);
    
    EQ(unsigned(1), param.m_pDefinition->s_parameterNumber);
    EQ(double(0.0), param.m_pDefinition->s_low);
    EQ(double(1024.0), param.m_pDefinition->s_high);
    EQ(unsigned(1024), param.m_pDefinition->s_chans);
    EQ(CTreeParameter::m_defaultSpecification.s_units, param.m_pDefinition->s_units);
    EQ(false, param.m_pDefinition->s_changed);
    EQ(CTreeParameter::m_generation-1, param.m_pDefinition->s_generation);   
}
// old style resolution or width not supported:

void TPTest::construct_7() {
    CPPUNIT_ASSERT_THROW(CTreeParameter param(
        "Test", 12, 0.0, 1024.0, "mm", true),
        std::logic_error
    );
                         
}
// construct from template:
void TPTest::construct_8() {
    CTreeParameter original ("Test", 1024, -1.0, 1.0, "mm");
    CTreeParameter copy("Test-copy", original);
    
    EQ(std::string("Test-copy"), copy.m_name);
    ASSERT(copy.m_pDefinition);
    EQ(unsigned(2), copy.m_pDefinition->s_parameterNumber);
    EQ(original.m_pDefinition->s_low, copy.m_pDefinition->s_low);
    EQ(original.m_pDefinition->s_high, copy.m_pDefinition->s_high);
    EQ(original.m_pDefinition->s_chans, copy.m_pDefinition->s_chans);
    EQ(original.m_pDefinition->s_units, copy.m_pDefinition->s_units);
    EQ(false, copy.m_pDefinition->s_changed);
    EQ(CTreeParameter::m_generation-1, copy.m_pDefinition->s_generation);
}
// copy construction
void TPTest::construct_9() {
    CTreeParameter original ("Test", 1024, -1.0, 1.0, "mm");
    CTreeParameter copy(original);
    EQ(original.m_name, copy.m_name);
    EQ(original.m_pDefinition, copy.m_pDefinition);
    
    CTreeParameter unbound;
    CTreeParameter cunbound(unbound);
    ASSERT(!cunbound.m_pDefinition);
    
}
// init with name and resolution only:

void TPTest::init_1() {
    CTreeParameter param;                 // Unbound:
    param.Initialize("test", 12);         // Binds it.
    
    EQ(std::string("test"), param.m_name);
    ASSERT(param.m_pDefinition);
    EQ(double(0.0), param.m_pDefinition->s_low);
    EQ(double(4096),param.m_pDefinition->s_high );
    EQ(unsigned(4096), param.m_pDefinition->s_chans);
    EQ(CTreeParameter::m_defaultSpecification.s_units, param.m_pDefinition->s_units);
    
}
// deprecated init with ighorwidth.

void TPTest::init_2() {
    CTreeParameter param;
    CPPUNIT_ASSERT_THROW(
        param.Initialize("test", 12, -1.0, 1.0, "junk", true),
        std::logic_error
    );
}
// init with just name:

void TPTest::init_3() {
    CTreeParameter param;
    param.Initialize("test");
    
    EQ(std::string("test"), param.m_name);
    ASSERT(param.m_pDefinition);
    EQ(CTreeParameter::m_defaultSpecification.s_low, param.m_pDefinition->s_low);
    EQ(CTreeParameter::m_defaultSpecification.s_high, param.m_pDefinition->s_high);
    EQ(CTreeParameter::m_defaultSpecification.s_chans, param.m_pDefinition->s_chans);
    EQ(CTreeParameter::m_defaultSpecification.s_units, param.m_pDefinition->s_units);
}
// Init with name and units.

void TPTest::init_4() {
    CTreeParameter param;
    param.Initialize("test", "mm/sec");
    
    EQ(std::string("test"), param.m_name);
    ASSERT(param.m_pDefinition);
    EQ(CTreeParameter::m_defaultSpecification.s_low, param.m_pDefinition->s_low);
    EQ(CTreeParameter::m_defaultSpecification.s_high, param.m_pDefinition->s_high);
    EQ(CTreeParameter::m_defaultSpecification.s_chans, param.m_pDefinition->s_chans);
    EQ(std::string("mm/sec"), param.m_pDefinition->s_units);
}
// Full init:

void TPTest::init_5() {
    CTreeParameter param;
    param.Initialize("test", 100, -1.0, 1.0, "mm/sec");
    
    EQ(std::string("test"), param.m_name);
    ASSERT(param.m_pDefinition);
    EQ(double(-1.0), param.m_pDefinition->s_low);
    EQ(double(1.0), param.m_pDefinition->s_high);
    EQ(unsigned(100), param.m_pDefinition->s_chans);
    EQ(std::string("mm/sec"), param.m_pDefinition->s_units);
}
// Dup construction with name:

void TPTest::dup_1() {
    CTreeParameter param("test");
    CTreeParameter clone("test", "mm/sec");           // overrides existing units.
    
    EQ(param.m_pDefinition, clone.m_pDefinition);
    EQ(std::string("mm/sec"), clone.m_pDefinition->s_units);
}
// dup construction full.

void TPTest::dup_2() {
    CTreeParameter param("test");
    CTreeParameter clone("test", 100, -1.0, 1.0,  "mm/sec");
    
    EQ(param.m_pDefinition, clone.m_pDefinition);
    EQ(double(-1.0), param.m_pDefinition->s_low);
    EQ(double(1.0), param.m_pDefinition->s_high);
    EQ(unsigned(100), param.m_pDefinition->s_chans);
    EQ(std::string("mm/sec"), param.m_pDefinition->s_units);
}
// We can convert a bound valid value for the tree parameter to double:

void TPTest::cvtdouble_1() {
    CTreeParameter p("test");
    
    // Make this valid with a known value artificially:
    
    p.m_pDefinition->s_generation = CTreeParameter::m_generation;
    CTreeParameter::m_event.at(p.m_pDefinition->s_parameterNumber) = 1.2345;
    
    EQ(double(1.2345), double(p));
}
// invalid value get throws std::range_error:

void TPTest::cvtdouble_2()
{
    CTreeParameter p("test");    // bound/invalid
    
    CPPUNIT_ASSERT_THROW(
        double v = p,
        std::range_error
    );
}
// unbound throws logic_error:

void TPTest::cvtdouble_3() {
    CTreeParameter p;
    CPPUNIT_ASSERT_THROW(
        double v = p,
        std::logic_error
    );
}
// Simple assignment makes valid if bound.

void TPTest::assign_1() {
    CTreeParameter p("test");
    CPPUNIT_ASSERT_NO_THROW(p = 1.234);
    
    // the value can be gotten and matches:
    
    EQ(double(1.234), double(p));
    
    // Validity book keeping and dope vector done:
    
    EQ(CTreeParameter::m_generation, p.m_pDefinition->s_generation);
    EQ(size_t(1), CTreeParameter::m_scoreboard.size());
    EQ(p.m_pDefinition->s_parameterNumber, CTreeParameter::m_scoreboard[0]);
    
    // Assigning again does not change the generation or scoreboard;
    
    p = 3.1416;
    EQ(double(3.1416), double(p));
    EQ(CTreeParameter::m_generation, p.m_pDefinition->s_generation);
    EQ(size_t(1), CTreeParameter::m_scoreboard.size());
    EQ(p.m_pDefinition->s_parameterNumber, CTreeParameter::m_scoreboard[0]);
  
}
// Assignments can be chained

void TPTest::assign_2() {
    CTreeParameter p("test");
    
    double result = p = 3.1416;
    EQ(double(3.1416), result);
}
// only bound parameters can be assigned to:

void TPTest::assign_3() {
    CTreeParameter p;    // not bound.
    CPPUNIT_ASSERT_THROW(
        p = 3.1416,
        std::logic_error
    );
}
// Successful assignment from another 'valid' tree parameter:

void TPTest::assign_4() {
    CTreeParameter p1("test");  // bound
    p1 = 3.1416;                // valid
    CTreeParameter p2("other");          // bound
    p2 = p1;                    // valid.
    
    EQ(CTreeParameter::m_generation, p2.m_pDefinition->s_generation);  //check validity.
    EQ(double(3.1416), double(p2));   //get double representation.
    
    // Ensure the validity book keeping was done in the scoreboard:
    
    EQ(size_t(2), CTreeParameter::m_scoreboard.size());
    EQ(p2.m_pDefinition->s_parameterNumber, CTreeParameter::m_scoreboard.at(1));
    
    
}
// assign from another tree parameter - rhs tp must be valid:

void TPTest::assign_5() {
    CTreeParameter p1("lhs");
    CTreeParameter p2("rhs");
    
    CPPUNIT_ASSERT_THROW(p1 = p2, std::range_error);
}
// assign from another tree parameter lhs must be bound:
void TPTest::assign_6() {
    CTreeParameter rhs("rhs");
    rhs = 3.1416;
    CTreeParameter lhs;              // not bound.
    CPPUNIT_ASSERT_THROW(lhs = rhs, std::logic_error);
}

// assign from another treeparameter rhs must be bound:

void TPTest::assign_7() {
    CTreeParameter rhs;
    CTreeParameter lhs("lhs");
    CPPUNIT_ASSERT_THROW(lhs = rhs, std::logic_error);
}
// += to bound/valid lhs

void TPTest::pluseq_1() {
    CTreeParameter lhs("lhs");
    lhs = 1.0;
    CPPUNIT_ASSERT_NO_THROW(lhs += 2.0);
    EQ(double(3.0) , double(lhs));
}
// += to bound invalid lhs:

void TPTest::pluseq_2() {
    CTreeParameter lhs("lhs");
    CPPUNIT_ASSERT_THROW(lhs += 1.0, std::range_error);
}
// += to unbound:

void TPTest::pluseq_3() {
    CTreeParameter lhs;
    
    CPPUNIT_ASSERT_THROW(lhs += 10.0, std::logic_error);
}

// -= to bound/valid lhs works.

void TPTest::minuseq_1() {
    CTreeParameter lhs("lhs");
    lhs = 10.0;
    CPPUNIT_ASSERT_NO_THROW(lhs -= 5.0);
    EQ(double(5.0), double(lhs));
}
// -= to bound invalid throws range error.

void TPTest::minuseq_2() {
    CTreeParameter lhs("lhs");
    CPPUNIT_ASSERT_THROW(lhs -= 5.0, std::range_error);
}
// -= to unbound throws logic_error.
void TPTest::minuseq_3() {
    CTreeParameter lhs;
    CPPUNIT_ASSERT_THROW(lhs -= 5.0, std::logic_error);
}
// the usual 3 tests for *=:

void TPTest::timeseq_1() {
    CTreeParameter lhs("lhs");
    lhs = 2.0;
    CPPUNIT_ASSERT_NO_THROW(lhs *= 2);
    EQ(double(4.0), double(lhs));
}
void TPTest::timeseq_2() {
    CTreeParameter lhs("lhs");
    CPPUNIT_ASSERT_THROW(lhs*=2, std::range_error);
}
void TPTest::timeseq_3() {
    CTreeParameter lhs;
    CPPUNIT_ASSERT_THROW(lhs *= 2, std::logic_error);
}
// the usual tests for /=

void TPTest::diveq_1() {
    CTreeParameter lhs("lhs");
    lhs = 4.0;
    CPPUNIT_ASSERT_NO_THROW(lhs /= 2);
    EQ(double(2.0), double(lhs));
}
void TPTest::diveq_2() {
    CTreeParameter lhs("lhs");
    CPPUNIT_ASSERT_THROW(lhs /= 2, std::range_error);
}
void TPTest::diveq_3() {
    CTreeParameter lhs;
    CPPUNIT_ASSERT_THROW(lhs /= 2, std::logic_error);
}
// three tests for x++;
void TPTest::postinc_1() {
    CTreeParameter p("P");
    p = 0.0;
    double d;
    CPPUNIT_ASSERT_NO_THROW(d = p++);
    EQ(double(0.0), d);
    EQ(double(1.0), double(p));
}
void TPTest::postinc_2() {
    CTreeParameter p("P");
    CPPUNIT_ASSERT_THROW(p++, std::range_error);
}
void TPTest::postinc_3() {
    CTreeParameter p;
    CPPUNIT_ASSERT_THROW(p++, std::logic_error);
}
// Three tests for ++x

void TPTest::preinc_1() {
    CTreeParameter p("P");
    p = 0.0;
    double d;
    CPPUNIT_ASSERT_NO_THROW(d = ++p);
    EQ(double(1.0), d);
    EQ(double(1.0), double(p));
}
void TPTest::preinc_2() {
    CTreeParameter p("P");
    CPPUNIT_ASSERT_THROW(++p, std::range_error);
}
void TPTest::preinc_3() {
    CTreeParameter p;
    CPPUNIT_ASSERT_THROW(++p, std::logic_error);
}
// three tests for x--:

void TPTest::postdec_1() {
    CTreeParameter p("P");
    p = 0.0;
    double d;
    CPPUNIT_ASSERT_NO_THROW(d = p--);
    EQ(double(0.0), d);
    EQ(double(-1.0), double(p));
    
}
void TPTest::postdec_2() {
    CTreeParameter p("P");
    CPPUNIT_ASSERT_THROW(p--, std::range_error);
}
void TPTest::postdec_3() {
    CTreeParameter p;
    CPPUNIT_ASSERT_THROW(p--, std::logic_error);
}
// Three tests for --p

void TPTest::predec_1() {
    CTreeParameter p("P");
    p = 0.0;
    double d;
    CPPUNIT_ASSERT_NO_THROW(d = --p);
    EQ(double(-1.0), d);
    EQ(double(-1.0), double(p));
}
void TPTest::predec_2() {
    CTreeParameter p("P");
    CPPUNIT_ASSERT_THROW(--p, std::range_error);
}
void TPTest::predec_3() {
    CTreeParameter p;
    CPPUNIT_ASSERT_THROW(--p, std::logic_error);
}
// you can get the name of bound or unbound parameters.
// unbound names are just empty is all:

void TPTest::getname_1() {
    CTreeParameter p("param");
    EQ(std::string("param"), p.getName());
    
}
void TPTest::getname_2() {
    CTreeParameter p;
    EQ(std::string(""), p.getName());
}
// getId requires a bound parameter, however:

void TPTest::getid_1() {
    CTreeParameter p("param");
    
    EQ(p.m_pDefinition->s_parameterNumber, p.getId());
}
void TPTest::getid_2() {
    CTreeParameter p;
    CPPUNIT_ASSERT_THROW(p.getId(), std::logic_error);
}
// bound and unbound test for getbins.

void TPTest::getbins_1()
{
    CTreeParameter p("Test");
    unsigned bins;
    CPPUNIT_ASSERT_NO_THROW(bins = p.getBins());
    EQ(CTreeParameter::m_defaultSpecification.s_chans, bins);
}
void TPTest::getbins_2() {
    CTreeParameter p;
    CPPUNIT_ASSERT_THROW(p.getBins(), std::logic_error);
}
// and for setbins too:

void TPTest::setbins_1() {
    CTreeParameter p("Test");
    CPPUNIT_ASSERT_NO_THROW(p.setBins(100));
    EQ(unsigned(100), p.getBins());
}
void TPTest::setbins_2() {
    CTreeParameter p;
    CPPUNIT_ASSERT_THROW(p.setBins(100), std::logic_error);
}