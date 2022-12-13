\section rawtoparams Raw events to parameters

This section describes, in detail, how to write an application in the FRIB
analysis framework that unpackes raw event data and outputs the unpacked parameters.

While the program you will write depends heavily on MPI, those dependencies have
been abstracted out into the libraries that make up the framwork.  Therefore
you do not need to use the MPI compiler drivers to build your program.

In order to write an application you'll need to accomplish the tasks in this checklist:

- [ ]    Write the worker code for your application.
- [ ]    Derive a concrete application from the frib::analysis::AbstractApplication abstract base class.
- [ ]    Compile/link your program specifying the correct flags to find the
library headers and shared objects that make up the framework.
- [ ]    Use **mpirun** to run your application specifying the desired number of
processes to run (and hence the number of workers).

\subsection rawtoparamworker Writing the Raw to parameter worker.

The worker process must request work items from the dealer process.  Work items
are blocks of input data to the worker.  Once the worker has received its next
block of data, it must break it up into raw events and operate on each of those
events to fill in the tree parameters that make up its output.

When an input event has been operated on, the resulting set of tree parameters
must be forwarded to the farmer, the next stage of the process.

All of this requires intimate knowledge of the messaging protocols and data
formats for the application when, in reality, you'd really just like to
write the event decode code.  Fortunately there are a pair of worker program
templates you can use.  The first: frib::analysis::CMPIRawToParametersWorker
requires only that you fill in a method that operates on a single ring item
to fill in the tree parameters that you want to output.  This will be describe in
this section.

The second worker; frib::analysis::CSpecTclWorker is designed to make it simple
to port SpecTcl analysis pipelines into this parallel framework.   This will  be
described elsehwere ( \ref spectclworker ).

frib::analysis::CMPIRawToParametersWorker is an abstract base class with
a single pure virtual method: **unpackData**, called for each raw event,
and an optional virtual method
**initializeUserCode** which is called once before the first event is received.

The example below shows how to derive a class from CMPIRawToParametersWorker
that unpacks event ring items that contain fixed length
events with 16 words that should be unpacked sequentially into a tree parameter
array named **parameters**.

```
#include <MPIRawToParametersWorker.h>
#include <AnalysisRingItems.h>                 // 1
#include <CTreeParamteerArray.h>
#include <cstdint>

using namespace frib::analysis;               // 2
ckass AbstractApplication;

class MyWorker : public CMPIRawToParametersWorker {
private:
    CTreeParameterArray*  m_pParams;
public:
    MyWorker(AbstractApplication& app) :
        AbstractApplication(app),
        m_pParams(new CTreeParameterArray("parameters", 16, 0)) {} // 3
    ~MyWorker() {
        delete m_pParams;
    }
    void unpackData(const void* pData);
};


// unpacker:

void MyWorker::unpackData(const void* pData) {   // 4
    union {
        const RingItemHeader* pH;
        const std::uint8_t*   p8;                // 5
        const std::uint32_t*  p32;
    } p;
    p.p8 = reinterpret_cast<const std::Uint8_t*>(pData);
    size_t hdrsize = sizeof(RingItemHeader);                        // 6
    if (p.pH->s_unused > sizeof(std::uint32_t)) {
        hdrsize += p.pH->s_unused - sizeof(std::uint32_t);
    }
    p.p8 += hdrsize;
    
    CTreeParameterArray& params(*m_pParams);           // 7
    for (int i=0; i < 16; i++) {
        params[i] = p.p32[i];                          // 8
    }
    
}

```

The numbers in the list below refer to the comment nubers in the sample
code above.

1.    Includes the minmum set of headers to get this compilation unit built.
2.    Brings the entire **frib::analysis** namespace intothe current namespace.  While not necessary it makes
the specification of the frib analysis classes less verose.
3.    Constructs `m_pParams` as a 16 element array of parameters.
4.    The unpack method receives a const void pointer to storage that, in this case
is actually an event (`PHYSICS_EVENT`) ring item.
5.     This union of pointers is used to eliminate the need to keep casting the
`pData` parameter to different pointer types.   The union members are a pointer to a
ring item header (`AnalysisRingItems.h` defines a limited set of ring item definitions).
6.     This block of code computes the pointer to the event body.  The `s_unused`
field of `RingItemHeader` is the same location as the body header size field.  If
it is larger than `sizeof(std::uint32_t)`  it is the size of the body header and
includes itself.  Otherwise there is no body header (NSCLDAQ-v11) and the
event data begins
immediately after the `RingItemHeader`.   Note that this code will only supprt
NSCLDAQ-11 and later data as NSCLDAQ-10 data will have event data immediately
after the `s_type` field of `RingItemHeader`.  NSCLDAQ-8 and earlier are not
compatible with the framework as the data are in fixed sized buffers rather than
in ringitems.
7.     In order to not have to use pointer notation, we initialize a reference
to the `m_params`.
8.    This loop unpacks the data  from the event into the tree parameter array.

Naturally most worker's will be computationally more complicated than this.


\subsection rawtoparamapp Writing the application

In order to produce a raw to parameter unpacker in the framework you need
to provide an application that

- [ ]    Has a dealer that read in the raw event file.
- [ ]    Has a farmer that reorders paramter data.
- [ ]    Has an outputter for parameter data.
- [ ]    Uses your `MyWorker` class written above in \ref rawtoparamworker

Fortunately the framework library provides code for all but the worker which we wrote.

The framework dealer:  frib::analysis::CMPIRawReader reads and deals out data
from an event file.  The outputter: frib::analysis::CMPIParameterOutput similarly
writes a parameter output file.

One concern both of these classes face is how to get the input and output files.
Both provide a virtual method; `getInputFile` in the case of CMPIRawReader and
`getOutputFile` in the case of CMPIParameterOutput that take as arguments the
`argc` and `argv` parameters defining the command line parameters passed to the
application and return a string filename path.

Note that the `mpirun` command will strip its parameters from the commandline parameter
arguments prior to these files seeing them.  So, for example:

   mpirun -np 10 myapp run-0000-00.evt run-0000-000.pars
   
Will set:

|  Argument   |    value     |
|-------------|--------------|
| argc        | 3            |
| argv[0]     | "myapp"      |
| argv[1]     | "run-0000-00.evt" |
| argv[2]     | "run-0000-00.pars" |


The default implementation of CMPIRawReader::getInputFile will return `argv[1]`
while the default implementation of CMPIParameterOutput will return `argv[2]`
If this is not suitable then you'll need to derive subclasses that replace these
default implementations.

Let's get started.  We need to derive a sublcass from frib::analysis::AbstractApplication
that implements the `dealer` `farmer` `outputter` and `worker` classes.  For this
application, we'll assume that the default file getters are just fine:

```
#include <AbstractApplication.h>
#include <MPIRawReader.h>
#include <MPIParameterOutputter.h>
#include <MPIParameterFarmer.h>
#include "MyWorker.h"           // 1

using namespace frib::analysis;   // 2

class MyApp : public AbstractApplication {  // 3
public:
    MyApp(int argc, char** argv);
    
    virtual void dealer(int argc, char** argv, AbstractApplication* pApp);
    virtual void farmer(int argc, char** argv, AbstractApplication* pApp);    // 4
    virtual void outputter(int argc, char** argv, AbstractApplication* pApp);
    virtual void worker(int argc, char** argv, AbstractApplication* pApp);
    
};

void
MyApp::dealer(int argc, char** argv, AbstractApplication* pApp) {
    CMPIRawReader dealer(argc, argv, pApp);                // 5
    dealer();                                              // 6
}

void
MyApp::farmer(int argc, char** argv, AbstractApplication* pApp) {
    CMPIParameterFarmer farmer(argc, argv, *pApp);       // 7
    farmer();                                            // 8
}

MyApp::outputter(int argc, char** argv, AbstractApplication* pApp) {
    CMPIParameterOutput outputter(argc, argv, pApp);   // 9
    outputter();                                       // 10
}

MyApp::worker(int argc, char** argv, AbstractApplication* pApp) {
    MyWorker worker(*pApp);                          // 11
    worker();                                        // 12
}


```

The numbers below refer to the commented numbers in the example code.

1.    We're assuming that the worker class we wrote in the previous section is
implemented in a separate compilation unit with a header named
`MyWorker.h`.
2.    Brings the symbols in the `frib::analysis` namespace into the default namespace.
This allows a more concise naming of library elements (e.g. `AbstractApplication`
rather than `frib::analysis::AbstractApplication`).
3.    We need to derive our own application class, in this case
`MyApp` from the `AbstractApplication` base class. This class declaration does that.
4.    In our application class definition, we will need to implement the pure
virtual methods of the base class.  These lines declare our intent to do so.
5.    Declares a raw reader object which will be our dealer.
6.    Runs the code in the raw dealer.   Note that this method, the dealer
method, will only be run by the framework int he correct MPI process (world rank #0).
7.     Declares a `CMPIParameterFarmer` as the farmer.  This class knows how to
resort the parmaeter data in to its original  order.
8.     Runs the farmer.  Note tha the framework will only call this method in
the correct (world rank #1) MPI process.
9.     Declares a `CMPIParameterOutput` object as our outputter.
10.    Runs the outputter object. Note that the outputter methdod will only be
called by the framework in the correct (world rank #2) MPI process.
11.    Creates an instance of our worker
12.    Runs our worker.  Note that this method will be called in all MPI Processes
with world rank > 2.  Thus when mpirun is used to start the program, -np must be at least 4
to ensure there's at least one worker.   The actual number of workers will be three less
than the value of the mpirun's -np option.