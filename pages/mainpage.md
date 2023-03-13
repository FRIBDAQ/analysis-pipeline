\mainpage FRIB Analysis Framework

This documentation describes a framework supplied at the FRIB to help you
write code to analyze data from experments performed here.  The framework
supports parallel exectuion using the worker/farmer pattern.  This allows you
to write scalable, parallel analysis code whle only needing to focus on the
segments of code that are relvevant to your problem.

Furthermore a SpecTcl jacket allows you to easily port SpecTcl event processors
into this framework to trivially gain scalable speedup.

### Analysis pipeline

Many experiments go through the same set of initial analysis steps:

*   Raw events are processed to produce a set of parameters.  This processing
can be a simple decoding of the data or involve more complex operations such as
digital signal processing on digitizer waveforms saved in each event.

*   Decoded parameters may be further processed to produce physically meaningful
values.  For example ADC values can be turned ito energies by applying some
parameterized calibration function to the raw value.  Opening angles might be
computed between hits in two parts of a detector system etc.

*  Parameter data is binned into histograms.  This may involve applying complex
conditions across several parameters to determine if a histogram should be incremented
from its underlying parameters in any event.  This is the sort of thing that
SpecTcl does well.

*   Information is extracted from histoigrams and used as input when writing
papers about the experiment.

Clearly any of these single steps or sets of steps may be iterated over until
satisfactory/correct results are achieved.

The FRIB analysis framework provides frameworkds for each of the first three
steps.

### Processing framework.

The first two steps in the pipeline can be extraoridinarily processing intensive,
depending on the actual computations required.   Therefore a parallel processing
framework is provided implemented on top of the Message Passing Interface (MPI).
This standard for parallel computing is supported both on single systems and by
most, if not all, job management systems in compute clusters.   Thus scaling
of your computation is limited only by event re-ordering, I/O bandwidth or
communications overheads.

The parallel framework is organized as:

*  An input process, which in general the user does not have to write,
*  A parallel set of workers that operate independently on sets of events (either raw or
decoded parameters depending on the analysis stage).  The user will generally need
to plug in some event processing code here.
*  A re-sorter that re-orders the data that comes out of the workers to restore
the original event order.  The user, in general does not have to write this.
*  An output stage that writes the reordered data to some data sink.  In general,
the user does not need to write this.

There is an implicit assumption in this structure that each event can be processed
indpendnetly of all other events.   For most experiments this is true.  Some
categories of experiments *do* have a processing stage for which this is not true;
decay experiments which must match up implantation events with subsequent decay events.

These experiments can still take advantage of parallelism in two ways:

*  By separating the event decoding and waveform processing from the matching problem,
the compute intensive waveform processing can be done in parallel.
*  By restricting the matching code to a single worker process (users have complete
control over the number of workers), that stage still exhibits a pipeline parallelism
between the input, compute, and output stages (re-ordering is trivial).

### Where to go for more information:

*  \ref treeparams Internal parameter representation  
*  \ref treevars    Steering variables.
*  \ref appclass 
*  \ref treereader Parameter Definition reader
*  \ref rawtoparams
    *    \ref rawtoparamworker 
    *    \ref rawtoparamapp
    *    \ref rawtoparammain
    *    \ref paramfileformat 
    *    \ref spectclworker 
    *    \ref compileandrunrawtoparams
*   \ref paramstoparams
    *    \ref parametermapping 
    *    \ref paramparamworker 
    *    \ref paramparamapp The parameter to parameter application.
    *    \ref paramparamcompileandrun compiling and running parameter to  parameter stages of the pipeline.
    
