lib_LTLIBRARIES = libfribCore.la

libfribCore_la_SOURCES = TreeParameter.cpp TreeParameterArray.cpp \
	TreeVariable.cpp TreeVariableArray.cpp
include_HEADERS=TreeParameter.h TreeParameterArray.h TreeVariable.h \
	TreeVariableArray.h \
	ParameterReader.h ParameterOutputter.h AnalysisRingItems.h \
	TCLParameterReader.h

libfribCore_la_CFLAGS=@TCL86_CFLAGS@
libfribCore_la_LDFLAGS=@TCL86_LIBS@

noinst_PROGRAMS=treeparamtests treevartests

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

TESTS=treeparamtests treevartests
