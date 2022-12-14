\section compileandrunrawtoparams  Compiling and running the application

This section describes how to build your application and how to run it.

\subsection compilerawtoparams Compilineg

The exact recipes for compiling an FRIB raw to parameter application depend on
where the libraries are installed.  At the FRIB they will normally be installed at
`/usr/opt/fribanalysis/major.minor-edit`  where the `major.minor-edit` directory
identifies a deployed version of the library.   `major` is the majore version number,
`minor` is the minor version number and `edit` a three digit edit level (edit
levels normally are used for minor changes such as bugfixes).

For example:

   /usr/opt/fribanalysis/1.0-003
   
is an installation of major version `1`, minor version `0` edit level `003`.

In our recipes we'll assume that you have defined the following symbols
in your Makefile:

*    `BASEDIR`   - the base of the installation directory tree
*    `INCDIR`    - as `$BASEDIR/include` the headers directory.
*    `LIBDIR`    - as `$BASEDIR/lib`  the library directory.

For example:

```
BASEDIR=/usr/opt/fribanalysis/1.0-003
INCDIR=$(BASEDIR)/include
LIBDIR=$(BASEDIR)/lib
```

These definitions in your Makefile allow you to easily move to a new library version
and keep rules concise.

Makefiles support default compilation rules.  These rules, in turn, can be tailored
to support user supplied options.   The following definition help support the
default rules that turn a C++ source program into an object:

```
CPPFLAGS=-I$(INCDIR)
``

If I now have a source file called myapp.cpp My Makefile can have the rule:

```
myapp.o:  myapp.cpp
```

and Make will supply a default rule that looks something like this:

```
$(CXX) -c $(CPPFLAGS) myapp.cpp
```

Which in turn will expand to:

```
g++ -c -I$(INCDIR) myapp.cpp
```

which will properly build my file.  Naturally you may need other flags on your
`CPPFLAGS` definition depending on the contents of your files.

To link your application you will need to define link flags that:

*    Add the $(LIBDIR) to the library search path.
*    Ensure that shared libraries in $(LIBDIR) will be loaded at run time.
*    Specify the shared libraries to be searched for unresolved symbols in your
software.

Here's an example of a definition of `LDFLAGS` which provides all of these
for a program that only depends on the FRIB analysis framework libraries:

```
LDFLAGS=-L$(LIBDIR) -Wl,-rpath=$(LIBDIR) \
    -lfribCore -lSpecTclFramework -ltclPlus -lException
```
Note that if you are not using the `SpecTclWorker` or classes derived from them
to support SpecTcl event processing pipelines you may not need `-lSpecTclFramework`

Finally, let's look at a sample Makefile.  We have the following sources that
need to be compiled:

*   main.cpp  - sets up the application structure and invokes it.
*   Worker.cpp - Sets up the worker (whether it be one from scratch or the
SpecTclWorker and an event processor in the same file for simplicity).

`Worker.h` is assumed to be a header that defines the classe implemented in
`Worker.cpp`:

```
BASEDIR=/usr/opt/fribanalysis/1.0-003
INCDIR=$(BASEDIR)/include
LIBDIR=$(BASEDIR)/lib

CPPFLAGS=-I$(INCDIR)

LDFLAGS=-L$(LIBDIR) -Wl,-rpath=$(LIBDIR) \
    -lfribCore -lSpecTclFramework -ltclPlus -lException
    
all: myprogram

myprogram: main.o Worker.o
    $(CXX)  -o myprogram $^ $(LDFLAGS)
    
main.o: main.cpp Worker.h

Worker.o: Worker.cpp Worker.h


```

Note the use of default compilation rules for the C++ files to generate their
objects.  The special macro `$^` means all of the dependencies (in this case
it expands to
`main.o Worker.o`).

\subsection runningraw Running the Programs.

Since the FRIB analysis pipeline framwework builds MPI Parallel applications they
cannot be run directly.  Instead the mpirun command must be used from the directory
against which the application framework was built.

How do we know where that is?  When the analysis framework is built/installed,
it writes a filenamed `VERSION` at the top level of its installation diectory.
Here are sample contents of this file:

```

FRIB analysis framework 1.0-000
Use mpirun in /usr/opt/mpi/openmpi-4.0.1/bin
Built on genesis on Wed 14 Dec 2022 09:00:44 AM EST

```
Note that the second line of this file tells you which mpirunto use:
`/usr/opt/mpi/openmpi-4.0.1/bin/mpirun`

mpirun must be given several command parameters:

*    -np <int>   provides the total number of processes to run.  <int> must be
an integer that is at least 4  (dealer, one worker, farmer, outputter).  For numbers
larger than 4, additional worker processes will be run.
*     The program name.
*     Any additional program parameters.

For example, suppose we are running ``myprogram`` with 10 workers and our input file
is run-0000-00.evt which we want to produce the output file run-0000-00.pars and
we're using the parameter definition file paramdef.tcl  - and our application uses
the default filename getters in the dealer and outputter and gets the parameter definition
file from the fourth command word:

```
mpirun -np 13 myprogram run-0000-00.evt run-0000-00.pars paramdef.tcl
```

will do the job.  mpirun has additional parameters including those which can distribute
the application across several hosts.  Normally, if you run on a cluster, however,
this distribution will usually be  handled by the job manager in parallel clusters.