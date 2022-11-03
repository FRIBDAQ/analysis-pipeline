lib_LTLIBRARIES = libfribCore.la

libfribCore_la_SOURCES = TreeParameter.cpp TreeParameterArray.cpp \
	TreeVariable.cpp TreeVariableArray.cpp
include_HEADERS=TreeParameter.h TreeParameterArray.h TreeVariable.h \
	TreeVariableArray.h

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
