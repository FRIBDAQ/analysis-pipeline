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

/** @file:  treevariabletests.cpp
 *  @brief: Tests of CTreeVariable
 */
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#define private public
#include "TreeVariable.h"
#undef private

using namespace frib::analysis;

class TVTest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(TVTest);
    CPPUNIT_TEST(crdef_1);
    CPPUNIT_TEST(crdef_2);
    CPPUNIT_TEST(crdef_3);
    
    CPPUNIT_TEST(lookupdef_1);
    CPPUNIT_TEST(lookupdef_2);
    
    CPPUNIT_TEST(names_1);
    CPPUNIT_TEST(names_2);
    
    CPPUNIT_TEST(getdef_1);
    CPPUNIT_TEST(getdef_2);
    
    CPPUNIT_TEST(iter_1);
    CPPUNIT_TEST(iter_2);
    
    CPPUNIT_TEST(size_1);
    CPPUNIT_TEST(size_2);
    
    CPPUNIT_TEST(construct_1);
    CPPUNIT_TEST(construct_2);
    CPPUNIT_TEST(construct_3);    // Note construction indirectly tests
    CPPUNIT_TEST(construct_4);    // CTreeVariable::Initialize so we don't need
    CPPUNIT_TEST(construct_5);    // an additional test for it.
    CPPUNIT_TEST(construct_6);
    
    // conversions and operators:
    
    CPPUNIT_TEST(double_1);
    CPPUNIT_TEST(double_2);
    
    CPPUNIT_TEST(assign_1);
    CPPUNIT_TEST(assign_2);
    CPPUNIT_TEST(assign_3);
    CPPUNIT_TEST(assign_4);
    
    CPPUNIT_TEST(pluseq_1);
    CPPUNIT_TEST(pluseq_2);
    
    CPPUNIT_TEST(minuseq_1);
    CPPUNIT_TEST(minuseq_2);
    
    CPPUNIT_TEST(timeseq_1);
    CPPUNIT_TEST(timeseq_2);
    
    CPPUNIT_TEST(diveq_1);
    CPPUNIT_TEST(diveq_2);
    
    CPPUNIT_TEST(postinc_1);
    CPPUNIT_TEST(postinc_2);
    CPPUNIT_TEST(preinc_1);
    CPPUNIT_TEST(preinc_2);
    
    CPPUNIT_TEST(postdec_1);
    CPPUNIT_TEST(postdec_2);
    CPPUNIT_TEST(predec_1);
    CPPUNIT_TEST(predec_2);
    
    CPPUNIT_TEST(name_1);
    CPPUNIT_TEST(name_2);
    
    CPPUNIT_TEST(value_1);
    CPPUNIT_TEST(value_2);
    CPPUNIT_TEST(value_3);
    CPPUNIT_TEST(value_4);
    
    CPPUNIT_TEST(unit_1);
    CPPUNIT_TEST(unit_2);
    CPPUNIT_TEST(unit_3);
    CPPUNIT_TEST(unit_4);

    CPPUNIT_TEST(changed_1);
    CPPUNIT_TEST(changed_2);
    CPPUNIT_TEST_SUITE_END();
    
private:

public:
    void setUp() {
        
    }
    void tearDown() {
        CTreeVariable::m_dictionary.clear();        
    }
protected:
    void crdef_1();
    void crdef_2();
    void crdef_3();
    
    void lookupdef_1();
    void lookupdef_2();
    
    void names_1();
    void names_2();
    
    void getdef_1();
    void getdef_2();
    
    void iter_1();
    void iter_2();
    
    void size_1();
    void size_2();
    
    void construct_1();
    void construct_2();
    void construct_3();
    void construct_4();
    void construct_5();
    void construct_6();
    
    void double_1();
    void double_2();
    
    void assign_1();
    void assign_2();
    void assign_3();
    void assign_4();
    
    void pluseq_1();
    void pluseq_2();
    
    void minuseq_1();
    void minuseq_2();
    
    void timeseq_1();
    void timeseq_2();
    
    void diveq_1();
    void diveq_2();
    
    void postinc_1();
    void postinc_2();
    void preinc_1();
    void preinc_2();
    
    void postdec_1();
    void postdec_2();
    void predec_1();
    void predec_2();
    
    void name_1();
    void name_2();
    
    void value_1();
    void value_2();
    void value_3();
    void value_4();
    
    void unit_1();
    void unit_2();
    void unit_3();
    void unit_4();
    
    void changed_1();
    void changed_2();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TVTest);

// Create definition returns a goo def:

void TVTest::crdef_1()
{
    auto p = CTreeVariable::createDefinition("test", 1.234, "furlongs");
    
    EQ(double(1.234), p->s_value);
    EQ(std::string("furlongs"), p->s_units);
    ASSERT(!p->s_definitionChanged);
    ASSERT(!p->s_valueChanged);
}
// Create definition puts it in the dict:

void TVTest::crdef_2() {
    auto pDef = CTreeVariable::createDefinition("test", 1.234, "furlongs");
    auto p   = CTreeVariable::m_dictionary.find("test");
    ASSERT(p != CTreeVariable::m_dictionary.end());
    EQ(pDef, &(p->second));
}
// Creating an existing def gives a logic error:

void TVTest::crdef_3() {
    auto pDef = CTreeVariable::createDefinition("test", 1.234, "furlongs");
    CPPUNIT_ASSERT_THROW(
        CTreeVariable::createDefinition("test", 1.234, "furlongs"),
        std::logic_error
    );
}
// NOthing to see here:

void TVTest::lookupdef_1() {
    auto pDef = CTreeVariable::lookupDefinition("Not found");
    ASSERT(!pDef);
}
// Found
void TVTest::lookupdef_2() {
    auto pDef = CTreeVariable::createDefinition("test", 1.234,"furlong");
    auto another = CTreeVariable::createDefinition("junk", 3.1416, "radians");
    
    EQ(pDef, CTreeVariable::lookupDefinition("test"));
}

// empty names:
void TVTest::names_1() {
    auto v = CTreeVariable::getNames();
    ASSERT(v.empty());
}
// Some names:
void TVTest::names_2() {
    CTreeVariable::createDefinition("test4", 1.0, "");
    CTreeVariable::createDefinition("test1", 1.0, "");
    CTreeVariable::createDefinition("test3", 1.0, "");
    CTreeVariable::createDefinition("test2", 1.0, "");
    
    // We rely on the fact that tree iteration is lexically ordered:
    
    auto v = CTreeVariable::getNames();
    EQ(size_t(4), v.size());
    EQ(std::string("test1"), v[0]);
    EQ(std::string("test2"), v[1]);
    EQ(std::string("test3"), v[2]);
    EQ(std::string("test4"), v[3]);    
}
// nothing :
//
void TVTest::getdef_1() {
    auto v = CTreeVariable::getDefinitions();
    ASSERT(v.empty());
    
}
// something
void TVTest::getdef_2() {
    CTreeVariable::createDefinition("test4", 4.0, "");
    CTreeVariable::createDefinition("test1", 1.0, "");
    CTreeVariable::createDefinition("test3", 3.0, "");
    CTreeVariable::createDefinition("test2", 2.0, "");
    
    auto v = CTreeVariable::getDefinitions();
    EQ(size_t(4), v.size());
    EQ(std::string("test1"), v[0].first);
    EQ(double(1.0), v[0].second->s_value);
    
    EQ(std::string("test2"), v[1].first);
    EQ(double(2.0), v[1].second->s_value);
    
    EQ(std::string("test3"), v[2].first);
    EQ(double(3.0), v[2].second->s_value);
    
    EQ(std::string("test4"), v[3].first);
    EQ(double(4.0), v[3].second->s_value);
}

// iterate on empty:

void TVTest::iter_1()
{
    ASSERT(CTreeVariable::begin() == CTreeVariable::end());
}
// Iterate on something:

void TVTest::iter_2() {
    CTreeVariable::createDefinition("test4", 4.0, "");
    CTreeVariable::createDefinition("test1", 1.0, "");
    CTreeVariable::createDefinition("test3", 3.0, "");
    CTreeVariable::createDefinition("test2", 2.0, "");
    
    // Iteration order is lexical:
    
    std::vector<std::string> names ={"test1", "test2", "test3", "test4"};
    std::vector<double> values = {1.0, 2.0, 3.0, 4.0};
    unsigned i =0;
    for (auto p = CTreeVariable::begin(); p != CTreeVariable::end(); p++ ) {
        EQ(names[i], p->first);
        EQ(values[i], p->second.s_value);
        i++;
    }
}
// empty:
void TVTest::size_1() {
    EQ(size_t(0), CTreeVariable::size());
}
// Has 4:
void TVTest::size_2() {
    CTreeVariable::createDefinition("test4", 4.0, "");
    CTreeVariable::createDefinition("test1", 1.0, "");
    CTreeVariable::createDefinition("test3", 3.0, "");
    CTreeVariable::createDefinition("test2", 2.0, "");
    
    EQ(size_t(4), CTreeVariable::size());
}
// Default constructor.

void TVTest::construct_1() {
    CTreeVariable v;
    
    ASSERT(CTreeVariable::m_dictionary.empty());
    EQ(std::string(""), v.m_name);
    ASSERT(!v.m_pDefinition);
}
// name only construction
void TVTest::construct_2() {
    CTreeVariable v("test");
    EQ(size_t(1), CTreeVariable::m_dictionary.size());  // Entered in the dictionary.
    ASSERT(CTreeVariable::lookupDefinition("test"));    // With the right key.
    
    EQ(std::string("test"), v.m_name);
    ASSERT(v.m_pDefinition);
    EQ(CTreeVariable::lookupDefinition("test"), v.m_pDefinition);
    
    // Contents of the definition:
    
    EQ(double(0.0), v.m_pDefinition->s_value);
    EQ(std::string(""), v.m_pDefinition->s_units);
    ASSERT(!v.m_pDefinition->s_valueChanged);
    ASSERT(!v.m_pDefinition->s_definitionChanged);
}
// Construct with name and units:

void TVTest::construct_3()
{
    CTreeVariable v("test", "mm");
    
    EQ(size_t(1), CTreeVariable::m_dictionary.size());  // Entered in the dictionary.
    ASSERT(CTreeVariable::lookupDefinition("test"));    // With the right key.
    
    EQ(std::string("test"), v.m_name);
    ASSERT(v.m_pDefinition);
    EQ(CTreeVariable::lookupDefinition("test"), v.m_pDefinition);
    
    // Contents of the definition:
    
    EQ(double(0.0), v.m_pDefinition->s_value);
    EQ(std::string("mm"), v.m_pDefinition->s_units);
    ASSERT(!v.m_pDefinition->s_valueChanged);
    ASSERT(!v.m_pDefinition->s_definitionChanged);
}
// name units and value:
void TVTest::construct_4()
{
    CTreeVariable v("test", 3.1416, "mm");
    
    EQ(size_t(1), CTreeVariable::m_dictionary.size());  // Entered in the dictionary.
    ASSERT(CTreeVariable::lookupDefinition("test"));    // With the right key.
    
    EQ(std::string("test"), v.m_name);
    ASSERT(v.m_pDefinition);
    EQ(CTreeVariable::lookupDefinition("test"), v.m_pDefinition);
    
    // Contents of the definition:
    
    EQ(double(3.1416), v.m_pDefinition->s_value);
    EQ(std::string("mm"), v.m_pDefinition->s_units);
    ASSERT(!v.m_pDefinition->s_valueChanged);
    ASSERT(!v.m_pDefinition->s_definitionChanged);
}

// construct from a properties thing.
// Note the flags don't get carried along:
void TVTest::construct_5() {
    CTreeVariable::Definition def(3.1416, "mm");
    def.s_valueChanged = true;                 // Does not propagate.
    def.s_definitionChanged= true;             //   ""     ""
    CTreeVariable v("test", def);
    
    EQ(size_t(1), CTreeVariable::m_dictionary.size());  // Entered in the dictionary.
    ASSERT(CTreeVariable::lookupDefinition("test"));    // With the right key.
    
    EQ(std::string("test"), v.m_name);
    ASSERT(v.m_pDefinition);
    EQ(CTreeVariable::lookupDefinition("test"), v.m_pDefinition);
    
    // Contents of the definition:
    
    EQ(double(3.1416), v.m_pDefinition->s_value);
    EQ(std::string("mm"), v.m_pDefinition->s_units);
    ASSERT(!v.m_pDefinition->s_valueChanged);
    ASSERT(!v.m_pDefinition->s_definitionChanged);
}
// copy construction:

void TVTest::construct_6() {
    CTreeVariable v1("test", 3.1416, "mm");
    CTreeVariable v2(v1);
    
    EQ(v1.m_name, v2.m_name);
    EQ(v1.m_pDefinition, v2.m_pDefinition);
    EQ(size_t(1), CTreeVariable::m_dictionary.size());
}
// unbound throws.
void TVTest::double_1() {
    CTreeVariable v;
    double value;
    CPPUNIT_ASSERT_THROW(value = v, std::logic_error);
}

// bound gives value

void TVTest::double_2() {
    CTreeVariable v("test", 1.2345, "mm");
    double value;
    CPPUNIT_ASSERT_NO_THROW(value = v);
    EQ(double(1.2345), value);
}
// assign double to unbound:

void TVTest::assign_1() {
    CTreeVariable v;
    CPPUNIT_ASSERT_THROW(v = 3.14159, std::logic_error);
}
// Assign double to bound

void TVTest::assign_2() {
    CTreeVariable v("test");
    CPPUNIT_ASSERT_NO_THROW(v = 3.1416);
    EQ(double(3.1416), double(v));
    
}
// Assigne Treevariable to unbound:

void TVTest::assign_3() {
    CTreeVariable v1;
    CTreeVariable v2("test1", 3.1416, "rad");
    CPPUNIT_ASSERT_THROW(v1 = v2, std::logic_error);
    
    // The other direction is also bad:
    
    CPPUNIT_ASSERT_THROW(v2 = v1, std::logic_error);
}
// Bound assignments of various types:

void TVTest::assign_4() {
    CTreeVariable v1("test1");
    CTreeVariable v2("test2", 3.1416, "rad");
    CTreeVariable v3("test3");
    
    CPPUNIT_ASSERT_NO_THROW(v3 = v1 = v2);   // Chaining too:
    EQ(double(3.1416), double(v1));
    EQ(double(3.1416), double(v3));
    
    CPPUNIT_ASSERT_NO_THROW(v1 = v2 = 1.234);  // Chaining from double.
    EQ(double(1.234), double(v2));
    EQ(double(1.234), double(v1));
}
// unbound += fails
void TVTest::pluseq_1() {
    CTreeVariable v1;

    CPPUNIT_ASSERT_THROW(v1 += 12, std::logic_error);
}
// ok and chains.
void TVTest::pluseq_2() {
    CTreeVariable v1("v1", 1.0, "mm");
    CTreeVariable v2("v2", 2.0, "Mm");
    
    CPPUNIT_ASSERT_NO_THROW(v1 += v2 += 1.234);
    EQ(3.234, double(v2));
    EQ(4.234, double(v1));
}
// -= with unbound fails.

void TVTest::minuseq_1()
{
    CTreeVariable v;
    CPPUNIT_ASSERT_THROW(v -= 1.0, std::logic_error);
}
// with bound and chaining
void TVTest::minuseq_2() {
    CTreeVariable v1("v1", 1.0, "");
    CTreeVariable v2("v2", 2.0, "");
    CTreeVariable v3("v3", 4.0, "");
    
    CPPUNIT_ASSERT_NO_THROW(v3 -= 1.0);
    EQ(3.0, double(v3));
    
    CPPUNIT_ASSERT_NO_THROW(v3 -= v2 -= 1.5);
    EQ(0.5, double(v2));
    EQ(2.5, double(v3));
}

// *= with unbound fails:

void TVTest::timeseq_1() {
    CTreeVariable v;
    CPPUNIT_ASSERT_THROW(v *= 2.0, std::logic_error);
}
// *= with bound works - and chaining.

void TVTest::timeseq_2() {
    CTreeVariable v1("v1", 1.0, "");
    CTreeVariable v2("v2", 2.0, "");
    CTreeVariable v3("v3", 4.0, "");
    
    CPPUNIT_ASSERT_NO_THROW(v1 *= 2);
    EQ(2.0, double(v1));
    CPPUNIT_ASSERT_NO_THROW(v3 *= v2 *= 2);
    EQ(4.0, double(v2));
    EQ(16.0, double(v3));
}
// /= unbound throws.,

void TVTest::diveq_1() {
    CTreeVariable v;
    CPPUNIT_ASSERT_THROW(v /= 2.0, std::logic_error);
}
// ok and chaining
void TVTest::diveq_2() {
    CTreeVariable v1("v1", 2.0, "");
    CTreeVariable v2("v2", 4.0, "");
    CTreeVariable v3("v3", 16.0, "");
    
    CPPUNIT_ASSERT_NO_THROW(v1 /= 2);
    EQ(1.0, double(v1));
    
    CPPUNIT_ASSERT_NO_THROW(v3 /= v2 /= 2);
    EQ(2.0, double(v2));
    EQ(8.0, double(v3));
}
// post increment unbound throws

void TVTest::postinc_1()
{
    CTreeVariable v;
    CPPUNIT_ASSERT_THROW(v++, std::logic_error);
}
// Post increment bound.

void TVTest::postinc_2() {
    CTreeVariable v("v");     // 0.0 value.
    double pre;
    CPPUNIT_ASSERT_NO_THROW(pre = v++);
    EQ(0.0, pre);
    EQ(1.0, double(v));
}
void TVTest::preinc_1() {
    CTreeVariable v;
    CPPUNIT_ASSERT_THROW(++v, std::logic_error);
}
void TVTest::preinc_2() {
    CTreeVariable v("test");
    double post;
    CPPUNIT_ASSERT_NO_THROW(post = ++v);
    EQ(1.0, post);
    EQ(1.0, double(v));
}
// operator -- tests.

void TVTest::postdec_1() {
    CTreeVariable v;
    CPPUNIT_ASSERT_THROW(v--, std::logic_error);
}
void TVTest::postdec_2() {
    CTreeVariable v("v", 2.0, "mm");
    double pre;
    CPPUNIT_ASSERT_NO_THROW(pre = v--);
    EQ(2.0, pre);
    EQ(1.0, double(v));
}
void TVTest::predec_1() {
    CTreeVariable v;
    CPPUNIT_ASSERT_THROW(--v, std::logic_error);
}
void TVTest::predec_2() {
    CTreeVariable v("v", 2.0, "mm");
    double post;
    CPPUNIT_ASSERT_NO_THROW(post = --v);
    EQ(1.0, post);
    EQ(1.0, double(v));
}
// get name unbound is actually ok.
void TVTest::name_1()
{
    CTreeVariable v;
    std::string name;
    CPPUNIT_ASSERT_NO_THROW(name = v.getName());
    EQ(std::string(""), name);
}
void TVTest::name_2() {
    CTreeVariable v("name");
    std::string name;
    CPPUNIT_ASSERT_NO_THROW(name = v.getName());
    EQ(std::string("name"), name);
}
// value method:

void TVTest::value_1() {
    CTreeVariable v;
    CPPUNIT_ASSERT_THROW(v.getValue(), std::logic_error);
}
void TVTest::value_2() {
    CTreeVariable v("v", 1.234, "mm");
    EQ(1.234, v.getValue());
}
void TVTest::value_3() {
    CTreeVariable v;
    CPPUNIT_ASSERT_THROW(v.setValue(1.2), std::logic_error);
}
void TVTest::value_4() {
    CTreeVariable v("v");
    CPPUNIT_ASSERT_NO_THROW(v.setValue(3.14));
    EQ(3.14, v.getValue());
                            
}
// getunit

void TVTest::unit_1() {
    CTreeVariable v;
    CPPUNIT_ASSERT_THROW(v.getUnit(), std::logic_error);
}
void TVTest::unit_2() {
    CTreeVariable v("v", "mm");
    std::string u;
    CPPUNIT_ASSERT_NO_THROW(u = v.getUnit());
    EQ(std::string("mm"), u);
}

// setunit

void TVTest::unit_3() {
    CTreeVariable v;
    CPPUNIT_ASSERT_THROW(v.setUnit("mm"), std::logic_error);
}
void TVTest::unit_4() {
    CTreeVariable v("test");
    CPPUNIT_ASSERT_NO_THROW(v.setUnit("furlongs/fortnight"));
    EQ(std::string("furlongs/fortnight"), v.getUnit());
}
// definition chang testing:

void TVTest::changed_1() {
    CTreeVariable v;
    CPPUNIT_ASSERT_THROW(v.hasChanged(), std::logic_error);
}
void TVTest::changed_2() {
    CTreeVariable v("test");
    
    ASSERT(!v.hasChanged());
    v.setUnit("mm");
    ASSERT(v.hasChanged());
}