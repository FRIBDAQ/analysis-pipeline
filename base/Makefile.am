lib_LTLIBRARIES = libfribCore.la

libfribCore_la_SOURCES = TreeParameter.cpp TreeParameterArray.cpp \
	TreeVariable.cpp TreeVariableArray.cpp TCLParameterReader.cpp \
	AbstractApplication.cpp DataReader.cpp DataWriter.cpp
include_HEADERS=TreeParameter.h TreeParameterArray.h TreeVariable.h \
	TreeVariableArray.h \
	ParameterReader.h ParameterOutputter.h AnalysisRingItems.h \
	TCLParameterReader.h AbstractApplication.h DataReader.h \
	DataWriter.h

libfribCore_la_CPPFLAGS=@TCL86_CFLAGS@ @TCLPLUS_CFLAGS@
libfribCore_la_LDFLAGS=@TCL86_LIBS@ @TCLPLUS_LIBS@

noinst_PROGRAMS=treeparamtests treevartests configtests readertests

treeparamtests_SOURCES=TestRunner.cpp Asserts.h treeparamtests.cpp \
	treeparamarraytests.cpp 
treeparamtests_CPPFLAGS=@CPPUNIT_CFLAGS@
treeparamtests_LDFLAGS= @CPPUNIT_LIBS@
treeparamtests_LDADD=libfribCore.la

treevartests_SOURCES=TestRunner.cpp Asserts.h treevariabletests.cpp \
	treevararraytests.cpp
treevartests_CPPFLAGS=@CPPUNIT_CFLAGS@
treevartests_LDFLAGS=@CPPUNIT_LIBS@
treevartests_LDADD=libfribCore.la

configtests_SOURCES=TestRunner.cpp Asserts.h tclconfigtests.cpp
configtests_CPPFLAGS=@CPPUNIT_CFLAGS@ @TCLPLUS_CFLAGS@ @TCL86_CFLAGS@
configtests_LDFLAGS=@CPPUNIT_LIBS@ @TCLPLUS_LIBS@ @TCL86_LIBS@
configtests_LDADD=libfribCore.la

readertests_SOURCES=TestRunner.cpp Asserts.h readertests.cpp
readertests_CPPFLAGS=@CPPUNIT_CFLAGS@ @TCLPLUS_CFLAGS@ @TCL86_CFLAGS@
readertests_LDFLAGS=@CPPUNIT_LIBS@ @TCLPLUS_LIBS@ @TCL86_LIBS@
readertests_LDADD=libfribCore.la


TESTS=treeparamtests treevartests configtests readertests
