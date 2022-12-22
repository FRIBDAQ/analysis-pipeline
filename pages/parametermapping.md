\sec parametermapping Input to Internal Parameter Mapping.

Application parameters are created by instantiating tree parameters or
tree parameter arrays. For example:

```
#include <TreeParameter.h>
#include <TreeParameterArray.h>

using namespace frib::analysis;

...

   CTreeParameter aparam("scalar");
   CTreeParameterArray vec("array", 16);

...
```

Is a code fragment that creates a single scalar parameter referred to by the
variable `aparam` and named `scalar` and an array of parameters
referred to by the variable `vec` named `array`.

When a parameter is defined (and CTreeParameterArray is just a collection of
parameters) it is assigned a numeric identifier.  When parameter files are written,
each event is coded as a sequence of parameter id/value pairs.

Within the application if two parameters have the same name, the tree parameter
module will ensure that they map to the same id and, therefore, point to the
same storage.  Across applications, however, there is no assurance that parameters
with the same name will be assigned the same id.

Each parameter file, however begins with an frib::analysis::ParameterDefinitions
ring item.  That item provides the name/id correspondence used by the
application that wrote the file.  It is produced using the parameter definitions
that are created by the frib::analysis::CParameterReader object used by the
application.

The parameter to parameter framework provides this information to the worker
objects allowing them to create a map between parameter ids it will receive
in events and parameters defined within the application.  This mapping is
expressed as a flat vector of file parameter id to CTreeParameter* objects and
therefore has a lookup time of O(1).

So as long as you define parameters in your application with the same names
as those in the file, the framework will know how to transparently load the application
parameters from each event's parameters.  This is done transparently to user
written code.

\subsection paramfiles Using parameter files.

Each application has a parameter file reader.   The parameter file reader
defines a set of tree parameters and tree variables.  Users of
frib::analysis::AbstractApplication must derive a parameter reader from
frib::analysis::CParameterReader and pass that to the function call operator
(`operator()`) of the application.  The parameter reader is used in all
ranks to define a set of parameters.  It is also, unless you take special steps,
the  only mechanism the outputter has to know which parameters are defined and,
therefore how to write its parameter definition ring item.

If you like, you can makee use of the frib::analysis::CTCLParameterReader which
allows you to use Tcl formatted output files that are descrbed in
\ref paramfileformat

Since these are interpreted in an extended Tcl interpreter you can use e.g.
the `source` command to stack the parameter definitions of previous stages of
the analysis pipeline up.  For example, suppose the raw parameter unpacker used

```
#raw.tcl - raw parameter definitions

treeparameterarray params 0 4095 4096 unitless 16 0

```

To defined raw parameters from some ADC module.  You've unpacked these but now
want to apply a linear calibration to produce e.g. energies.  Your parameter to
parameter definition file could look like:

```
# calibrate.tcl - parameters and steering variables

source raw.tcl;             # The input parameters

treeparameterarray energies 0 1000 100 KeV 16 0

treevariablearrary slopes 0.123 KeV/lsb 16 0
treevariablearray  offsets 500 KeV 16 0

set fd [open clalibration.dat r]
for {set i 0} {$i < 16} {incr i} {
   set line [gets $fd]
   scan $line "%f %f" slope offset
   treevariable slopes.[format %02d i] $slope Kev/lsb
   treevariable offsets.[formst %02d i] $offset KeV
}

```

Note the trick used to read the actual calibration values from a file. It
makes use of the fact that e.g. the array `slopes` is really a set values
named `slopes.00`, `slopes.01` ... `slopes.15` and that, like treeparameters,
if two tree variables are defined with the same name they point to the same
underlying storage (value and units).

This script allows the outputter to know the tree parameters of both the
raw unpacker and the energies this stage will create as well as the
calibration steering parameters used to transform the raw parameters into energies.

The outputter will write a frib::analysis::ParamterDefinitions item to the
followed by a frib::analysis::VariableItem ring item to the output file allowing
the next stage to understand how to map parameters and documenting the steering
parameters used to create the events in the parameter file.



The bottom lines from all of this;

*    If the worker defines parameters with the same name as those in the
inputfile, those parameters will be loaded for each event in which they appear.
*    If the parameter reader defines parameters and variables, they will be picked up
by the outputter and used to construct a frib::analysis::ParameterDefinitions and
frib::analysis::VariableItem  ring item which will be written to the ouptput file
prior to any other ring items.



