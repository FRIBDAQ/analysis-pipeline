lib_LTLIBRARIES = libSpecTclFramework.la

libSpecTclFramework_la_SOURCES=BufferDecoder.cpp Analyzer.cpp Event.cpp \
	EventProcessor.cpp SpecTclWorker.cpp
libSpecTclFramework_la_CPPFLAGS=-I@top_srcdir@/base
libSpecTclFramework_la_LDFLAGS=@top_builddir@/base/libfribCore.la

include_HEADERS = BufferDecoder.h Analyzer.h Event.h EventProcessor.h \
	SpecTclWorker.h

noinst_PROGRAMS=eventTests workerTests spectclTest

eventTests_SOURCES=TestRunner.cpp eventtests.cpp
eventTests_CPPFLAGS=@CPPUNIT_CFLAGS@ -I@top_srcdir@/base
eventTests_LDFLAGS=@CPPUNIT_LIBS@
eventTests_LDADD=libSpecTclFramework.la \
        @top_builddir@/base/libfribCore.la


workerTests_SOURCES=TestRunner.cpp workerTest.cpp
workerTests_CPPFLAGS=@CPPUNIT_CFLAGS@ -I@top_srcdir@/base
workerTests_LDFLAGS= @CPPUNIT_LIBS@
workerTests_LDADD=libSpecTclFramework.la \
        @top_builddir@/base/libfribCore.la


spectclTest_SOURCES=spectclTest.cpp testSpecTcl.cpp
spectclTest_CPPFLAGS=@CPPUNIT_CFLAGS@ -I@top_srcdir@/base
spectclTest_LDFLAGS= @CPPUNIT_LIBS@
spectclTest_LDADD=libSpecTclFramework.la \
        @top_builddir@/base/libfribCore.la



TESTS=eventTests workerTests

PARTESTS: spectclTest
	mpirun -np 4 spectclTest dummy specout.pars
