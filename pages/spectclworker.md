\section spectclworker  SpecTcl compatibility software

Many experimental groups have a great deal of effort invested in SpecTcl
event processing piplines that, essentially, do the same thing as the raw event
to parameter stage of the analysis pipeline.   One disadavantage of using
these pipelines within SpecTcl is the computational time each analysis run
requires to repeat the decoding of raw events into parameters.  Furthermore,
SpecTcl event processors don't provide support for parallel execution.

It therefore makes sense to provide software which supports porting SpecTcl
event processing pipelines into the first stage of the analysis pipeline.
You can imagine (and we plan to implement) SpecTcl support to read parameter data
files as the first stage of its own analysis pipeline, making SpecTcl the
spectrum producing stage of the analysis pipeline.

This section describes the support available to import SpecTcl event processors
and pipelines of event processors into the analysis pipelines.

There are some retrictions, however:

*    Only physics events will be processed by these pipelines.  The other ring item
types are ignored by the framework and essentially passed without interpretation.
Processing e.g. state change items, would require ensuring these items get sent
to all worker processes.
*    The SpecTcl API is, obviously not available to event processors in the analysis
framework because it is not SpecTcl and, therefore does not have most of the objects
manipulated by that API.
*    The BufferDecoder and Analyzer APIs, while implemented will not, in general
provide any useful results as, once more, these are not really germane to the
analysis framework.
*    If more than one worker is run, event processors cannot make use of information
from prior events to process the current event.  This is because there's no
guarantee that an event processor will receive both the independent event and the
dependent event.  They might get processed in different workers, and workers are
completely contained processes with separated address spaces.
*    The event processors should unpack to tree paramters not to the ``CEvent`` object.
This is also the currently recommended way for SpecTcl event processors to operate.

If you want to maintain a common source code base between the event processors
you use in SpecTcl and those you use with the SpecTcl compatability code, you'll
probably need to use #ifdefs to select the appropriate code for the appropriate
environment.  Thee examples in this section assume that you define the
compilation preprocessor symbol
`FRIB_ANALYSIS` if you are using the analysis framework and that otherwise the
code is intended to be compiled for and run in SpecTcl.

Makefiles you use will need to be very careful to specify analysis framework
header and library directories earlier in the search paths than SpecTcl ones
if you need both.

\subsection eventprocessor A SpecTcl Event Processor ported.

In this section, we'll extract the `CFixedEventUnpacker` example from the
SpecTcl skeleton file and port it into the analysis framework.  We'll do this
in a way that allows us to use it in SpecTcl later on if we choose.
First we'll need a header e.g. `FixedUnpacker.h` which will look like this:

```
#ifndef FIXEDUNPACKER_H           // 1
#define FIXEDUNPACKER_H

#ifdef FRIB_ANALYSIS
#include <SpecTclTypes.h>
#else                            // 2
#include <daqdatatypes.h>
#endif

#include <EventProcessor.h>

#ifdef FRIB_ANALYSIS
namespace frib {
    namespace analysis {
#endif
    class CEvent;
    class CAnalyzer;            // 3
    class CBufferDecoder;
    class CTreeParameterArray;
#ifdef FRIB_ANALYSIS
    }
}
#endif

#ifdef FRIB_ANALYSIS
using namespace frib::analysis; // 4
#endif

class CFixedEventUnpacker : public  CEventProcessor
{
private:
  CTreeParameterArray& m_raw;
public:
  CFixedEventUnpacker();
  virtual ~CFixedUnpacker();
  virtual Bool_t operator()(const Address_t pEvent,
                CEvent&         rEvent,
                CAnalyzer&      rAnalyzer,
                CBufferDecoder& rDecoder);
};


#endif
```
Notes:

1.    If you are not familiar with this construct, it is called an *include guard*
This #ifdef, which encloses the entire header  ensures that multiple includes
of the same header will not cause compilation errors because only the first
inclusion will actually provide any code since that will define the `FRIXEDUNPACKER_H`
symbol.
2.    SpecTcl uses type aliases such as `Int_t`, or `Address_t` extensively to
allow the underlying actual typse to be centrally changed if needed.  In SpecTcl,
these types are defined in the header `daqdatatypes.h` while the framework
defines a subset of these types in `SpecTclTypes.h`  This block includes the appropriate
header, depending on the compilation envionment.
3.    The event processor requires at least a forward definition of several types.
In the SpecTcl environment, these actual types are defined in the global namespace
while, in the analysis framework in keeping with its naming conventions, they are
defined in the `frib::analysis` namespace.   This block of code forward defines either
`CEvent`, `CAnalyzer` and `CBufferDeder` in the SpecTcl compilation environment or,
in the analysis framework compilation environment: 
`frib::analysis::CEvent`, `frib::analysis::CAnalyzer` and `frib::analysis::CBufferDeder`
4.    Finally the `frib::analysis` namespace is usd to resolve names in the analysis
framework environment so we don't need to use fully qualified names.

Now let's look at the implementation of the `CFixedEventUnpacker` class which we
might put in `FixedUnpacker.cpp`

```
#include "FixedUnpacker.h"
#include <Event.h>
#include <Analyzer.h>               // 1
#include <BufferDecoder.h>

using namespace frib::analysis;     // 2

CFixedEventUnpacker::CFixedEventUnpacker() :
    m_raw(*(new CTreeParameterArray("even.raw", 10))) {} // 3

CFixedEventUnpacker::~CFixedEventUnpacker() {
    delete &m_raw;                                
}
Bool_t
CFixedEventUnpacker::operator()(const Address_t pEvent,
                CEvent&         rEvent,
                CAnalyzer&      rAnalyzer,
                CBufferDecoder& rDecoder)
{

  const UShort_t* p(reinterpret_cast<const UShort_t>(pEvent)); // 4
  
  UShort_t  nWords = *p++;
  Int_t     i      = 1;

  
#ifndef FRIB_ANALYSIS
  CTclAnalyzer&      rAna((CTclAnalyzer&)rAnalyzer);    // 5
  rAna.SetEventSize(nWords*sizeof(UShort_t)); // Set event size.
#endif

  nWords--;                     // The word count is self inclusive.
  int param = 0;                // No more than 10 parameters.

  while(nWords && (param < 10)) {               // Put parameters in the event
    m_raw[param] = *p++;                        // 6
    nWords--;
    param++;
  }
  return kfTRUE;                // kfFALSE would abort pipeline.
}


```

Notes:

1.    The Makefile for this application must ensure that these come from the
FRIB analysis framework headers not the SpecTcl headers.
2.    Pulls the definitions of the FRIB Analysiys code from the `frib::analysis`
namespace into the default name resolution space.  Thus e.g.
frib::analysis::CEvent can be referred to as `CEvent`.
3.    In the version of this unpacker in `MySpecTclApp.cpp`, This tree paramter array
was defined at file global scope.  For both SpecTcl and the FRIB analysis framework
this unecessary so we create the tree parameter array  here. and delete it in the
destructor.
4.    In SpecTcl, we used a translating buffer pointer but, since all NSCLDAQ
systems are little endian, and since big endian systems are increasingly hard to
come by,  we just use a plain old pointer to UShort_t.
5.    The FRIB Framework does not need to be given the size of the event so this
code is only compiled if we're not compiling it for the FRIB analysis framework.
6.    As before the loop unpacks data into the parameters.  In this case the
parameters are a tree parameter array referenced by `m_raw`.

\subsection spectclapp Setting up the Application for SpecTcl.

Much of the work described here is the same as for \ref rawtoparamworker
The only difference is that we need to use a SpecTclWorker, instantiate one or more
eventprocessors, and add them to that worker to set up the analysis pipeline.
That's done as follows:

```
#include <AbstractApplication.h>
#include <SpecTclWorker.h>
#include "FixedUnpacker.h"

...   // code above here as in \ref rawtoparamworker

void
MyApp::worker(int argc, char** argv, AbstractApplication* pApp) {
    SpecTclWorker worker(*app);     // 1
    
    CFixedEventUnpacker unpacker;   
    worker.addProcessor(unpacker, "Raw");  // 2
    worker(argc, argv);            // 3
}

```

Notes:
1.    The frib::analysis::SpecTclWorker provides support for a SpecTcl analysis
pipeline.
2.     In this line of code, we set up the analysis pipeline to only contain
the fixed event unpacker.  If we have more than one event processor, we can
instantiate them here and add them in the order we want them processed.
3.      The worker process invokes each element of the event processing pipeline
and transmits the unpacked tree parameters to the farmer.

This modified event processor *should* be usable both in SpecTcl and in
the FRIB analysis framework.

\subsubsection tailoringspectclworker Tailoring the SpecTclWorker

The frib::analysiys::SpecTclWorker can be tailored in a couple of ways.  Both
require creating, instantiating and using a derived class from SpecTclWorker:

The SpecTclWorker object invokes the event processing pipeline elements'
`OnInitialize` and `OnEventSourceOpen` virtual methods.  The latter requires
knowing the name of the input file.  This is gotten by querying the
virtual method  getInputFilename which, by default just uses `argv[1]`  you
may override this in order to use a different command line parameter e.g. as the
name of the input event file.

The event processing pipeline `OnInitialize` and `OnEventSourceOpen` in turn are
invoked from the virtual method `initializeUserCode`  overriding this can also
be used to provide initialization of e.g. global tree parameter structs.
For example, If we wanted the event.raw tree parameter array to be global data:

```
...
#include <TreeParameterArray.h>
#include <SpecTclWorker.h>
#include <AbstractApplication.h>
...
using namespace frib::analysis;    // 1

CTreeParameterArray raw;           // 2

class MyWorker : public SpecTclWorker { // 3
public:
    MyWorker(AbstractApplication& app) : SpecTclWorker(app) {} // 4
    virtual void initializeUserCode(    // 5
        int argc, char** argv, AbstractApplication& app
    );
};

void
MyWorker::initializeUserCode(
        int argc, char** argv, AbstractApplication& app
) {
    raw.Initialize("event.raw", 10);            // 6
    SpecTclWorker::initializeUserCode(argc, argv, app); // 7
}

```

Notes:
1.    As usual the names in the `frib::analysis` namespace are imported into the
default symbol resolution space so we don't need to use the fully qualified names.
2.    Tree parameters and tree parameter array support both full construction and
two phase construction.  Full construction of global data is risky due to the
C++'s rules about the order in which such construction occurs.  In two phase construction
an object is constructed minimally, as it is here and later, when safe fully initialized
with a method call (in the case of the tree parameter subsystem, with a call to one of
the `Initialize` methods).
3.    We declare `MyWorker` as a class derived from SpecTclWorker.
4.    We don't have any construction needs of our own so we just defer construction
to the `SpecTclWorker` class constructor.
5.    This line declares our desire to override the `SpecTclWorker::initializeUserCode`
method with one of our own.
6.     Our  `MyWorker::initializeUserCode` method will complete the initialization of the
global tree parameter array.
7.     This line then invokes the base class's `initializeUserCode` which invokes
the appropriate analysis pipeline methods.


Using `MyWorker` in place of `SpecTclWorker` in our application provides support
for global tree parameter data.  Clearly this can be extended to the more complicated
global tree parameter based data structures seen in code developed and supported
by some groups.

