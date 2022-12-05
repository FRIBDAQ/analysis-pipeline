lib_LTLIBRARIES = libSpecTclFramework.la

libSpecTclFramework_la_SOURCES=BufferDecoder.cpp Analyzer.cpp Event.cpp
libSpecTclFramework_la_CPPFLAGS=-I@top_srcdir@/base
libSpecTclFramework_la_LDFLAGS=@top_builddir@/base/libfribCore.la

include_HEADERS = BufferDecoder.h Analyzer.h Event.h

noinst_PROGRAMS=eventTests

eventTests_SOURCES=TestRunner.cpp eventtests.cpp
eventTests_CPPFLAGS=@CPPUNIT_CFLAGS@ -I@top_srcdir@/base
eventTests_LDFLAGS=libSpecTclFramework.la @CPPUNIT_LIBS@

TESTS=eventTests

