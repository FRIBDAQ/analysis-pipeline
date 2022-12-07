lib_LTLIBRARIES = libSpecTclFramework.la

libSpecTclFramework_la_SOURCES=BufferDecoder.cpp Analyzer.cpp Event.cpp \
	EventProcessor.cpp SpecTclWorker.cpp
libSpecTclFramework_la_CPPFLAGS=-I@top_srcdir@/base
libSpecTclFramework_la_LDFLAGS=@top_builddir@/base/libfribCore.la

include_HEADERS = BufferDecoder.h Analyzer.h Event.h EventProcessor.h \
	SpecTclWorker.h

noinst_PROGRAMS=eventTests workerTests

eventTests_SOURCES=TestRunner.cpp eventtests.cpp
eventTests_CPPFLAGS=@CPPUNIT_CFLAGS@ -I@top_srcdir@/base
eventTests_LDFLAGS=@top_builddir@/spectcl/libSpecTclFramework.la \
	@top_builddir@/base/libfribCore.la \
	@CPPUNIT_LIBS@

workerTests_SOURCES=TestRunner.cpp 
workerTests_CPPFLAGS=@CPPUNIT_CFLAGS@ -I@top_srcdir@/base
workerTests_LDFLAGS=@top_builddir@/spectcl/libSpecTclFramework.la \
        @top_builddir@/base/libfribCore.la \
        @CPPUNIT_LIBS@


TESTS=eventTests workerTests

