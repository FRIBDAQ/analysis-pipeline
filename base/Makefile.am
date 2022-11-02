lib_LTLIBRARIES = libfribCore.la

libfribCore_la_SOURCES = TreeParameter.cpp TreeParameterArray.cpp
include_HEADERS=TreeParameter.h TreeParameterArray.h

noinst_PROGRAMS=treeparamtests

treeparamtests_SOURCES=TestRunner.cpp Asserts.h treeparamtests.cpp \
	treeparamarraytests.cpp
treeparamtests_CPPFLAGS=@CPPUNIT_CFLAGS@
treeparamtests_LDFLAGS= @CPPUNIT_LIBS@
treeparamtests_LDADD=libfribCore.la

TESTS=treeparamtests
