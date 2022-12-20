lib_LTLIBRARIES = libfribCore.la

libfribCore_la_SOURCES = TreeParameter.cpp TreeParameterArray.cpp \
	TreeVariable.cpp TreeVariableArray.cpp TCLParameterReader.cpp \
	AbstractApplication.cpp DataReader.cpp DataWriter.cpp \
	MPIParameterOutput.cpp MPIRawReader.cpp TriggerSorter.cpp \
	MPITriggerSorter.cpp MPIParameterFarmer.cpp \
	MPIRawToParametersWorker.cpp MPIParameterDealer.cpp \
	MPIParametersToParametersWorker.cpp
include_HEADERS=TreeParameter.h TreeParameterArray.h TreeVariable.h \
	TreeVariableArray.h \
	ParameterReader.h  AnalysisRingItems.h \
	TCLParameterReader.h AbstractApplication.h DataReader.h \
	DataWriter.h MPIParameterOutput.h MPIRawReader.h \
	TriggerSorter.h MPITriggerSorter.h MPIParameterFarmer.h \
	MPIRawToParametersWorker.h MPIParameterDealer.h \
	MPIParametersToParametersWorker.h

libfribCore_la_CPPFLAGS=@TCL86_CFLAGS@ @TCLPLUS_CFLAGS@ -std=c++11
libfribCore_la_LDFLAGS=@TCL86_LIBS@ @TCLPLUS_LIBS@ 

noinst_PROGRAMS=treeparamtests treevartests configtests iotests \
	testOutput testInput sorttests testSort \
	passthruTest testWorker1 testParinput

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

iotests_SOURCES=TestRunner.cpp Asserts.h readertests.cpp writertests.cpp
iotests_CPPFLAGS=@CPPUNIT_CFLAGS@ @TCLPLUS_CFLAGS@ @TCL86_CFLAGS@
iotests_LDFLAGS=@CPPUNIT_LIBS@ @TCLPLUS_LIBS@ @TCL86_LIBS@
iotests_LDADD=libfribCore.la

sorttests_SOURCES=TestRunner.cpp Asserts.h sorttests.cpp
sorttests_CPPFLAGS=@CPPUNIT_CFLAGS@ @TCLPLUS_CFLAGS@ @TCL86_CFLAGS@
sorttests_LDFLAGS=@CPPUNIT_LIBS@ @TCLPLUS_LIBS@ @TCL86_LIBS@
sorttests_LDADD=libfribCore.la


testOutput_SOURCES=testOutput.cpp testouttests.cpp
testOutput_CPPFLAGS=@CPPUNIT_CFLAGS@ @TCLPLUS_CFLAGS@ @TCL86_CFLAGS@
testOutput_LDFLAGS=@CPPUNIT_LIBS@ @TCLPLUS_LIBS@ @TCL86_LIBS@
testOutput_LDADD=libfribCore.la

testInput_SOURCES=testInput.cpp testintests.cpp
testInput_CPPFLAGS=@CPPUNIT_CFLAGS@ @TCLPLUS_CFLAGS@ @TCL86_CFLAGS@
testInput_LDFLAGS=@CPPUNIT_LIBS@ @TCLPLUS_LIBS@ @TCL86_LIBS@
testInput_LDADD=libfribCore.la


testSort_SOURCES=sortTest.cpp testsorttests.cpp
testSort_CPPFLAGS=@CPPUNIT_CFLAGS@ @TCLPLUS_CFLAGS@ @TCL86_CFLAGS@
testSort_LDFLAGS=@CPPUNIT_LIBS@ @TCLPLUS_LIBS@ @TCL86_LIBS@
testSort_LDADD=libfribCore.la

passthruTest_SOURCES=passthruTest.cpp testPassthru.cpp
passthruTest_CPPFLAGS=@CPPUNIT_CFLAGS@ @TCLPLUS_CFLAGS@ @TCL86_CFLAGS@
passthruTest_LDFLAGS=@CPPUNIT_LIBS@ @TCLPLUS_LIBS@ @TCL86_LIBS@
passthruTest_LDADD=libfribCore.la

testWorker1_SOURCES=testWorker1.cpp worker1Tests.cpp
testWorker1_CPPFLAGS=@CPPUNIT_CFLAGS@ @TCLPLUS_CFLAGS@ @TCL86_CFLAGS@
testWorker1_LDFLAGS=@CPPUNIT_LIBS@ @TCLPLUS_LIBS@ @TCL86_LIBS@
testWorker1_LDADD=libfribCore.la


testParinput_SOURCES=testParinput.cpp parintest.cpp parinworkertests.cpp
testParinput_CPPFLAGS=@CPPUNIT_CFLAGS@ @TCLPLUS_CFLAGS@ @TCL86_CFLAGS@
testParinput_LDFLAGS=@CPPUNIT_LIBS@ @TCLPLUS_LIBS@ @TCL86_LIBS@
testParinput_LDADD=libfribCore.la


TESTS=treeparamtests treevartests configtests iotests sorttests

PARTESTS: install testOutput
	mpirun -np 4 testOutput /dev/null test.dat
	mpirun -np 4 testInput raw.evt 
	mpirun -np 5 testSort  null sorted.evt
	mpirun -np 4 passthruTest null passed.evt
	mpirun -np 4 testWorker1 in.evt out.evt
	mpirun -np 4 testParinput in.par out.par defs.par
