# analysis-pipeline
FRIB Data Analysis Pipeline

Provides parallel frameworks for

*  Transforming raw event data into parameter files that can be processed by both SpecTcl and Rustogramer.
*  Transforming parameter files into other parameter files.

Support is provided for importing SpecTcl event processing pipelines into the framework as well
as the Treeparameter/Treevariable classes originally concieved of by Daniel Bazin.

### Building the framwork

```
./configure --prefix=where-to-install CXX=some-mpi-c++compiler
make all install check
```


Note that some-mpi-c++compiler must be the full path to an MPI C++ compiler Example:

```
./configure --prefix=/usr/opt/frib-analysis/1.0-001 CXX=/usr/opt/mpi/openmpi-4.0.1/bin/mpicxx
make all install check
```




