\section paramstoparms Computing additional parameters

Once the raw data are unpacked into an initial set of parameters (see
\ref rawtoparams), it is not unusual to need to compute additional parameters
at a later time.  You can imagine a pipeline of computation that stepwise
(and iteratively if necessary) transforms the raw parameters to physically meaningful
parameters from the point of view of the experiment and science being extracted
from it.

It would be wasteful to have to modify the raw to parameters stage of processing
and decode the raw parameters at every step of this pipline.  Therefore,
the analysis pipeline supports stages which take, as input, a file of parameter data
and, produce as output, another file of parameter data.

The underlying principles of these stages of the analysis pipeline are very
similar to those of raw to parameter stages of the pipeline.  However:

*    You must use an input stage (dealer) that understands parameter files.
This dealer knows how to send information around to the rest of the application
that allow the parameters in the input file to be mapped to internal/output file
parameters. See \parametermapping
*    You must derive your worker from an  frib::analysis::CMPIParametersToParametersWorker
*    You *must* use tree parameters to represent your parameters internally.  You
may still use tree variables to steer your computation.

The following documentation sections provide more information about how to write
compile and run an application that maps input parameters to output parameters:

*    \ref parametermapping Describes how parameters in the input files are mapped
into tree parameters that are defined in the program.  This section stresses, as well,
the importance of using parameter definition files in addition to any
frib::analysis::CTreeParameter and frib::analysis::CTreeParameterArray objects
you might create.
*    \ref paramparamworker Describes how to build a worker object to perform the
computations that make your application work.
*    \ref paramparamapp Describes how to derive from frib::analysis::AbstractApplication
to string together the components that make up the application.
*    \ref paramparamcompileandrun Describes how to compile and run your program.

Note that since SpecTcl is written to primarily take raw events and convert them
into parameters, there is no SpecTcl compatibility layer for this stage of the
pipeline as the raw events are not available to this application;  Only the
parameters extxracted or computed by previous stages of the analysis pipeline.