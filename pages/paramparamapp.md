\section paramparamapp  Writing the Parameter to Parameter application.

This section describes the requirements for writing a concrete application
class for a parameter to parameter pipeline stage.  A simple example is
also given.

The application object is used to set up the proceses that make up the ranks of the
MPI application.  It must be derived from the
frib::analysis::AbstractApplication abstract base class.

This subclass must:

*    Establish the standard parameter file reader as the dealer process.
*    Establish the standard trigger sorter as the farmer process.
*    Establish the parameter outputter as the outputter.
*    Establish *your* worker as the worker process.
*    Additionally, you must supply a main which:
    *    Creates a parameter reader to define how the  tree parameter and tree variable
definitions are gotten.
    *    Creates an instance of your concrete application
    *    Transfers control to the application's operator() method to start the
MPI application.

\subsection paramparamconcreteapp The Concrete Subclass of frib::analysis::AbstractApplication

In this example we will use the sample worker we showed in \ref paramparamworkerexample
We'll assume the header for this worker is  in `MyWorker.h`

```

#include <AbstractApplication.h>
#include <MPIParameterDealer.h>
#include <MPITriggerSorter.h>    // 1
#include <MPIParameterFarmer.h>
#include <TCLParamteerReader.h>
#include "MyWorker.h"            // 2

using namespace frib::analysis;

class MyApp : public AbstractApplication {
public:
    MyApp(int argc, char** argv);
    virtual ~MyApp();
    
    virtual void dealer(int argc, char** argv, AbstractApplication* pApp);
    virtual void worker(int argc, char** argv, AbstractApplication* pApp); // 3
    virtual void farmer(int argc, char** argv, AbstractApplication* pApp) ;
    virtual void outputter(int argc, char** argv, AbstractApplication* pApp);
}
// Implement the dealer:

void
MyApp::dealer(int argc, char** argv, AbstractApplication* pApp) {   // 4
     CMPIParameterDealer dealer(argc, argv, pApp);
     dealer();
}
// Implement my worker:

void
MyApp::worker(int argc, char** argv, AbstractApplication* pApp) { // 5
    MyWorker worker(argc, argv, pApp);
    worker();
}

// Implement my farmer:

void
MyApp::farmer(int argc, char** argv, AbstractApplication* pApp) { // 6
    CMPIParameterFarmer farmer(argc, argvc, *pApp);
    farmer();
}
// Implement the outputter:

void
MyApp::outputter(int argc, char** argv, AbstractApplication* pApp) { // 7
    CMPIParameterOutput outputter;
    outputter(argc, argv, pApp);
}
// Now the main program-- we'll use the Tcl definition file reader.
// We'll assume that argv[3] is the name of the parameter file.
// A real application should check argc
//
int main(int argc, char** argv) {                    // 8
    CTCLParameterReader defreader(argv[3]);          // 9
    MyApp app(argc, argv);                           // 10
    app(defreader);                                  // 11
}
```
Let's go through this example.   Numbres in the list below refer to the commented
numbers in the example:

1.   Includes the definitions for the classes we're going to need when implementing
our application and program.
2.   Includes the definiton of our worker class.
3.   This section defines the class `MyWorker` which implements a concrete
class derived from frib::analysis::AbstractApplication  We must implement four
methods, one for each process role in our MPI application.  The base class, upon
starting MPI will invoke the appropriate method depending on the rank of the
process running.
4.    The dealer reads input data and sends them to the workers, how operate on them
in parallel.  Note that the dealer, by default, reads its input data from the file
that is `argv[1]`  (this is the argv[] after MPI has stripped the mpirun specific
parameters from it).   You can derive a subclass from frib::analysis::CMPIParameterDealer
and override its getInputFile method to modify this behavior.
5.    Our worker just instantiates `MyWorker` and run it by calling the
`operator()` method.
6.    The farmer just insantiates an frib::analysis::CMPIParameterFarmer object
and runs it by calling its `operator` method.
7.    The outputter implementation creates an runs a
frib::analysis::CMPIParameterOutput which outputs a parameter data file.
By default, data are written to the filenamed by
argv[2].  To modify this behavior you can derive a subclass which overrides
the getOutputFile method to provide a filenane from a different source.
8.  All programs require a `main` as their entry point.  Here we deinfe and
implement that function.
9.  We use a Tcl defintion file reader which will get its definition file from
argv[3].  In a production program:
    *   The command parameters have not yet been stripped of their mpirun
specific parameters.  That happens in MPI_Init which is run by the application's
function call operator, so you probably should, instead use `argv[arc-1]` e.g.
    *    Code should verify the appropriate minimum parameter count.
    *    Could might want to verify the file provided by the user is a readable file.
10.    Creates an instancde of our app.
11.    Runs the application providing it with the definition file reader.





