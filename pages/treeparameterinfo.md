\section tree Parameters and Variables.

Users of the framework will find concepts that are familiar to them from SpecTcl.
This is intentional.  Specifically, how parameters are represented and
how variables that can steer the computation are represented are very much like
Tree parameters and Tree variables in SpecTcl.

Furthermore they are implemented by compatible classes so that porting SpecTcl
code is simplified.

\subsection treeparams  Parameter Representation

Parameters in the framework are represented by objects that mimic double precision
values.  These objects know if they have been assigned to in a given event and,
if they are used in the left hand side of a computation before having been used
as the target of an assignment they will throw an exception.

Individual parameters are represented by a class named
frib::analysis::CTreeParameter.  Fixed
sized arrays arrays of CTreeParameter objects can be created using a
frib::analysis::CTreeParameterArray
object.  Really each element of this array like object is a CTreeParameter object.

The framework supports the ability to:

*    Ensure that all stages of the processing framework know about a common
set of tree parameters.
*    In the event that capability is used, the output stage will document
the treeparameters and there correspondence to parameters in the output
file.  It is strongly recommended you do this as this facilitates reconstruction
of the original event in subsequent stages of the pipeline.

Tree parameters have several bits of metadata associated with them.  The most
important of these is the tree parameter name.  If two CTreeParameter objects
are instantiated with the same name, they refer to the same underlying parameter.
For example:

```
using namespace frib::analysis
{
   {
        CTreeParameter a("something);
        ....
        a = 3.1416;
   }
   {
        CTreeParameter b("something");   // Same as a.
        std::cerr << b << std::endl;     // Output 3.1416.
   }
}

```

In this example, even though the variable *a* has gone out of scope, the variable
*b* will still refer to its underlying parameter and be able to pull out the
value that was previously assigned.

Thus, when using tree parameters it is not necessary to have global blocks of
treeparameters that are shared by all parts of your program.  Declare the tree
Parameters you need in the parts of the program that need them.  This information
isolation/hiding, in general, makes for program structures that are more resilient.

In addition to the name of a tree parameter; each tree parameter has additional metadata
that are provided for documentation purposes.

*    *units*  The units of measure of the parameter.
*    *low*    When histogramming the raw parameter, the suggested low limit of the
parameter's axis.
*    *high*   When histogramming the raw parameter, the suggested high limit of the
parameter's axis.
*    *bins* Suggested number of bins to allocate to the parameter range between
*low* and *high*

\subsection treevars  Tree Variables

Often computations need to be parameterized.  For example, when taking a value
that represents energy you may want to apply a calibration function to it to map
it into a real energy value as a new parameter.  You can hard code the calibration
function with all its parameters and re-code it each time you need to change its
parameterization, which is error prone, or you can use *steering variables* to
parameterize the function and arrange for the values of those variables to be
set externally.

As with SpecTcl, Tree Variables provide a mechanism to define and use a steering
variable and, in the analysis framework, define the value of steering variables
externally.

A steering variable is represente internally by an instance of
frib::analysis::CTreeVariable.  This class is implemented to be compatible
with the SpecTcl CTreeVariable class.  Instances of the class can, in most cases,
be used as instances of double in your code.  Consider:

```
using namespace frib::analysis;
{
    CTreeParameter raw("raw");
    CTreeParameter e("energy");
    CTreeVariable slope("slope");
    CTreeVariable offset("offset");
    
    if (raw.isValid()) {
        e = raw*slope + offset;
    }    
}
```
This example shows how to use steering variables to compute an energy from
a raw parameter value using a linear calibration function.

Note that, as with CTreeParameter, CTreeVariable is named, and several instances
with the same name represent the same underlying data, so again, in most cases,
there's no need for elaborate global structures.   Simply declare the tree variable
you need near the code where you need it.

\subsection treereader Distributing tree definitions.

The framework normally is given a definition file which provides both tree parameter
and tree variable definitions.  The definition file is read in using a 
class derived from frib::analysis::CParameterReader that is passed to the
application's operator() when the application is started.

Since MPI uses a single program multiple data model of execution, the reader runs
in all processes of the parallel application defining a common set of parameters and
variables throughout the application.

You are free to implement your own CParameterReader class to use whatever format
you want for your definition file, or you can use the
frib::analysis::CTCLParameterReader class to use the Tcl scripting language to
definen your parameters and variables.

Here's an example of how to do that:

```
#include <AbstractApplication.h>
#include <TCLParameterReader.h>
#include <string>

using namespace frib::analysis;

// Concrete subclass of CAbstractApplication.
class MyApp : public CAbstractApplication {
...
};

int main(int argc, char** argv) {
    std::string defFile = getDefFile(argc, argv);
    MyApp app;
    CTCLParameterReader pReader(defFile.c_str());
    app(pReader);
}
```

For more about how to build your application class, see
\ref appclass Application class.