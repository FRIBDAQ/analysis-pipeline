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
extern unsigned NUM_DATAITEMS;

static const std::uint32_t BEGIN_RUN = 1;
static const std::uint32_t END_RUN   = 2;
static const std::uint32_t PHYSICS_EVENT= 30;

class inputtest  : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(inputtest);
    CPPUNIT_TEST(header_1);
    CPPUNIT_TEST(header_2);
    
    CPPUNIT_TEST(contents_1);
    CPPUNIT_TEST(contents_2);
    CPPUNIT_TEST(contents_3);
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
    void contents_2();
    void contents_3();
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
    
    pRingItemHeader pHeader = reinterpret_cast<pRingItemHeader>(block.get());
    EQ(std::uint32_t(sizeof(RingItemHeader)), pHeader->s_size);
    EQ(BEGIN_RUN, pHeader->s_type);
    EQ(std::uint32_t(sizeof(std::uint32_t)), pHeader->s_unused);
}
// The block finishes with an end of run stub:

void inputtest::contents_2() {
    FRIB_MPI_Message_Header header;
    auto nRead = read(m_fd, &header, sizeof(header));
 
    std::unique_ptr<char> block(new char[header.s_nBytes]);
    nRead = read(m_fd, block.get(), header.s_nBytes);
    char* pend = block.get() + header.s_nBytes;
    pRingItemHeader pHeader =
        reinterpret_cast<pRingItemHeader>(pend - sizeof(RingItemHeader));
    EQ(std::uint32_t(sizeof(RingItemHeader)), pHeader->s_size);
    EQ(END_RUN, pHeader->s_type);
    EQ(std::uint32_t(sizeof(std::uint32_t)), pHeader->s_unused);
}        
// There are NUM_DATAITEMS in between the begin andend:

void inputtest::contents_3() {
    FRIB_MPI_Message_Header header;
    auto nRead = read(m_fd, &header, sizeof(header));
 
    std::unique_ptr<char> block(new char[header.s_nBytes]);
    nRead = read(m_fd, block.get(), header.s_nBytes);
    
    // multishaped pointer to reduce the number of casts:
    union {
        const char* p8;
        const RingItemHeader* phdr;
        const std::uint32_t*  p32;
    } p;
    p.p8 = block.get();
    p.p8 += sizeof(RingItemHeader);      // we know there's a BEGIN from contents_1.
    
    for (int i =0; i < NUM_DATAITEMS; i++) {
        // check the header -- 300 uint32_t's in the body.
        EQ(300*sizeof(uint32_t) + sizeof(RingItemHeader), size_t(p.phdr->s_size));
        EQ(PHYSICS_EVENT, p.phdr->s_type);
        EQ(std::uint32_t(sizeof(std::uint32_t)), p.phdr->s_unused);
        
        // data items are a counting pattern:
        
        p.phdr++;                           // body.
        for (std::uint32_t  i = 0; i < 300; i++) {
            EQ(i, *p.p32);
            p.p32++;
        }
        
    }
    // Where I am now + sizeof header should get me to the end of the block.
    
    p.phdr++;                          // Should be header.s_nBytes past the original
    
    EQ(ptrdiff_t(header.s_nBytes), (p.p8 - block.get()));
    
    
}