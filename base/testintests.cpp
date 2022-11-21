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

/** @file:  
 *  @brief: 
 */
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <memory>
#include <cstdint>


#include "AnalysisRingItems.h"

using namespace frib::analysis;
extern std::string testFilename;

static const std::uint32_t BEGIN_RUN = 1;
static const std::uint32_t END_RUN   = 2;
static const std::uint32_t PHYSICS_EVENT= 30;

class inputtest  : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(inputtest);
    CPPUNIT_TEST(header_1);
    CPPUNIT_TEST(header_2);
    
    CPPUNIT_TEST(contents_1);
    CPPUNIT_TEST_SUITE_END();
    
private:
    int m_fd;
public:
    void setUp() {
        m_fd = open(testFilename.c_str(), O_RDONLY);
    }
    void tearDown() {
        close(m_fd);
    }
protected:
    void header_1();
    void header_2();
    
    void contents_1();
};

CPPUNIT_TEST_SUITE_REGISTRATION(inputtest);
// First should be a header:
void inputtest::header_1()
{
    FRIB_MPI_Message_Header header;
    auto nRead = read(m_fd, &header, sizeof(header));
    EQ(sizeof(header), size_t(nRead));
    ASSERT(header.s_nBytes > 0);
    ASSERT(!header.s_end);
}
// 1. should be able to read the bytes specified in the header.
// 2. There's only one work item so that should also hit the EOF

void inputtest::header_2() {
    FRIB_MPI_Message_Header header;
    auto nRead = read(m_fd, &header, sizeof(header));
 
    std::unique_ptr<char> block(new char[header.s_nBytes]);
    nRead = read(m_fd, block.get(), header.s_nBytes);
    EQ(header.s_nBytes , unsigned(nRead));    // Slurped in the whole thing.
    
    // Reading even one more byte gives an End:
    
    std::uint8_t byte;
    nRead = read(m_fd, &byte, sizeof(byte));
    EQ(ssize_t(0), nRead);
}
// First item in contents is a BEGIN_RUN stub record:

void inputtest::contents_1() {
    FRIB_MPI_Message_Header header;
    auto nRead = read(m_fd, &header, sizeof(header));
 
    std::unique_ptr<char> block(new char[header.s_nBytes]);
    nRead = read(m_fd, block.get(), header.s_nBytes);
    
    RingItemHeader* pHeader = reinterpret_cast<RingItemHeader*>(block.get());
    EQ(std::uint32_t(sizeof(RingItemHeader)), pHeader->s_size);
    EQ(BEGIN_RUN, pHeader->s_type);
    EQ(std::uint32_t(sizeof(std::uint32_t)), pHeader->s_unused);
}