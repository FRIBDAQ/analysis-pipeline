\section appclass The Application Class

The MPI uses a single program model of computing.  All processes in the parallel
application run the same program.  Clearly, however, at some point each instance
of that program must execute code that fulfils that instance's role within the
application.

Recall that the computing model the framework presents is one of a source of
data that fans out to an arbitray number of workers.  Each worker processes the
input data producing a set of parameters which are forwarded to a sorter. As the
sorter has fully reordered data available, it passes those data on to an outputter.

In the framwork, these roles are referred to as:

*    **Dealer** which reads input data and deals it out to:
*    **Workers** which operate on the data producing parameters that are sent to the
*    **Farmer** which sorts those data back into their original order passing them on to the
*    **Outputter** which outputs the data to e.g. file.

The [strategy pattern](https://en.wikipedia.org/wiki/Strategy_pattern)] provides
a natural match to this structure.  In the strategy pattern, a class instance
provide the main flow of control (in this case initialization, role selection and
finalization), but defers the actual execution of the role to a virtual method.

The frib::analysis::AbstractApplication class provides the strategy pattern.
It's operator() initializes MPI, computes several custom data types corresponding
to the messages that will be sent around the application and, using the process
*rank*, selects a pure virtual method to run the role for the process.

A typical use of the AbstractApplication is to derive a concrete class and
then instantiate that class from the main and call the instance's operator().
operator() then initializes MPI, computes our custom data types and, using the
process's world rank selecs the correct method.  To the user this might look like:

```
#include <AbstractApplication.h>
#include <TCLParameterReader.h>

using namespace frib::analyiss;

class MyApp : public AbstractApplication {
public:
    Application(int argc, char** argv) : AbstractApplication(argc, argv);
    
    virtual void dealer(int argc, char** argv, AbstractApplication* pApp) {...}
    virtual void farmer(int argc, char** argv, AbstractApplication* pApp) {...}
    virtual void outputter(int argc, char** argv, AbstractApplication* pApp) {...}
    virtual void worker(int argc, char** argv, AbstractApplication* pApp) {...}
};
```

The following methods must be implemented by the MyApp:

*    **dealer** must read in data and distribute it to workers.  This implementation
is normally simply selecting an existing dealer class, instantiating and running it.
*    **farmer** must receive data pushed to it by the workers and re-order it into
the original event order.  This is normally implemented by instantiating an existing
farmer class and running it.
*    **outputter** must receive data pushed to it by the farmer and ouput it to some
data sink.  This is normally done by instantiating an existing outputter and
running it.
*    **worker** must request and receive blocks of data from the dealer.   These
blocks must then be processed into parameters which are then pushed to the farmer.
This is normally implemented by extending an existing class so that all you need
to do is process the data gotten from the dealer.  The existing class would then
take care of requesting work units, receiving them and handing you items from that
unit to operate, as well as sending the results to the farmer and also handling end
conditions.

Hopefully you can see that creating an application is not usually much more
than choosing the dealer, farmer and ouputter and implementing the code you'd have
to implement in a serial program to process your data.  The pure virtual
methods of AbstractApplication are only as coarse as they are to support a common
flexible framework as well as special applications.




