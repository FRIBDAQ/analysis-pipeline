\section paramparamworker The Parameter to Parmaeter Worker.

The analysis library defines an abstract base class named:
frib::analysis::CMPIParametersToParametersWorker  It operates as follows:

*    It receives the parameter definitions item pushed to all workers by the dealer
and uses it to construct a lookup table indexed by ids in that item whose entries
point to tree parameters with corresonding names in the application.
*    It receives the variable value item pushed to all workers by the dealer and
saves that information so that if the application needs it can retrieve it and, optionally,
load it into tree variables local to the application.  The framework cannot know
if the user code wants to use those variables or if it has overriden values for
those variables.
*    It requests and receives events from the dealer and loads the local parameters
using the mapping table it constructed.   Any parameter indices which do not have
a mapping are lost.   This could be an error or it could be the application wanting
to reduce the set of parameters.
*    Once parameters for an event are loaded, the pure virtual method
`process` is called and, in a concrete class, this invokes user code to perform
the needed computations to produce any output parameters.
*    On the return from `process` the event is marshaled from the treeparameters
and the event is sent to the farmer.

As with the raw event to parameters stage of the pipeline, the farmer re-sorts
events by the order in which they originally occurred before passing them on to
the outputter which creates and output parameter file.

\subsection paramparamworkerexample Sample Worker.

Let's carry on with the example in \ref paramfiles.  Recall that this example
was applying a linear calibration function to input parameters producing energy
parameters as output.   In order to make our worker we need to

*    Create a new class derived from frib::analysis::CMPIParametersToParametersWorker
*    The constructor of that class must define the tree variables and tree parameters
the application needs.
*    The `process` method *must* be written and must apply the linear transform
to compute the output parameters.

Here's a sample:

```
#include <MPIParametersToParametersWorker.h>
#include <TreeParameter.h>           // 1
#include <TreeParameterArray.h>
#include <TreeVariableArray.h>

namespace frib {
    namespace analysis {
        class AbstractApplication;  // 2
    }
}


using namespace frib::analysis;


class MyWorker : public CMPIParametersToParametersWorker {
    CTreeParameterArray raw;
    CTreeParameterArray calibrated;
    CTreeVariableArray  slopes;                 // 3
    CTreeVariableArray  offsets;
public:
    MyWorker(int argc, char** argv, AbstractApplication* pApp) :
    AbstractApplication(argc, argv, pApp),   // 4
    raw("raw, 16),
    calibrated("energy", 16),     // 5      
    slopes("slopes", 16),
    offsets("offsets", 16)
    {
        
    }
    virtual void process() {
        for (int i =0; i < 16; i++) {
            if (raw[i].isValid()) {               // 6
                calibrated[i] = raw[i]*slopes[i] + offsets[i];
            }
        }
    }
    
}

```

