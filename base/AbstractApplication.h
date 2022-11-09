/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Giordano Cerriza
	     FRIB
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** @file:  AbstractApplication.h
 *  @brief: Represents an application. 
 */
#ifndef ABSTRACTAPPLICATION_H
#define ABSTRACTAPPLICATION_H
#include <mpi.h>
namespace frib {
    namespace analysis {
        class CParameterReader;
        /**
         * @class AbstractApplication
         *    This class is a strategy pattern for the dealer/worker/farmer/outputter
         *    parallel execution pattern implemented in MPI.
         *    If an application has MPI rank n it will allocate them as follows:
         *    -   0  - Dealer
         *    -   1  - Farmer
         *    -   2  - Outputter
         *    -   3-n - Workers.
         *
         *   The dealer, rank 0 is connected to the data source.  Workers send requests
         *   to it for work items the dealer respondes either with data or an
         *   end message indicating there is no more data.  When the dealer
         *   has sent ends to all of the workers it will call MPI_Finalize to
         *   do our part to exit the application.
         *
         *   The Farmer gets messags from the workers and re-orders them into
         *   the original order. When the dealer sends a work item it will actually
         *   be sent as a pair of messages.  The first message will contain a
         *   work item number and a payload size while the second message will
         *   contain a size of the following message (sent as an array of chars).
         *   The Farmer orders the work items by item number and sends them to the
         *   Outputtter. End messages from the workers are counted and when
         *   the farmer has gotten an end message from all workers it will
         *   send an end message to the outputter call MPI_Finalize to do its part
         *   to exit.
         *
         *   The outputter gets messages from the farmer that are either results
         *   or end marks.  The outputter is connected to the data sink.
         *   It will write the messages from the farmer to the data sink. When it
         *   receives an end messages, it will call MPI_Finalize ot do its part to
         *   exit.
         *
         *   Workers are responsible for actually doing the application specific
         *   operations on the data.  They request and receive blocks of data
         *   to operate on from the Dealer.  They transform those blocks into
         *   output data which they then send to the Farmer for re-ordering.
         *   If the dealer sends them a worker and end message, it will
         *   send and end message to the farmer and call MPI_Finalize to do
         *   its part to exit.
         *
         *    Each process type is implemented as a pure virtual method.
         *    The function call operator():
         *     -   Calls MPI_INIT
         *     -   Uses the ParameterReader it was passed to read in the parameter configuration.
         *     -   If rank 0 figures out the extent of the program and if it is
         *         sufficient to run at least one worker.
         *     -   Depending on the rank, invokes the appropriate strategy
         *         methods to be the appropriate role.
         *     -   When the strategy method exists, invokes MPI_Finalize and
         *         returns to the caller who, presumably, exits.
         *
         *  @note the operator() is also virtual to allow that logic to be
         *  overridden.
         *
         *  A typical use of this class woud be to:
         *  \verbatim
         *
         * using namespace frib::analysis;
         * int main(argc, char**argv) {
         *  // define a MyApplication deriveds, concrete class class from
         *  //   AbstractApplication
         *
         *   CTCLParameterReader configReader("configFile.tcl");
         *   MyApplication app(argc, argv);
         *   app(configReader);
         *
         *   // When we get here our role has called MPI_Finalize
         *
         *    exit(EXIT_SUCCESS);
         *  }
         *  
         *  \endverbatim
         *   
         */
        class AbstractApplication {
        private:
            // Keep the program arguments so that we can access them from the
            // methods.
            
            int m_argc;
            char** m_argv;
        private:
            MPI_Datatype  m_messageHeaderType;
            MPI_Datatype  m_requestDataType;
        public:
            AbstractApplication(int argc, char** argv);
            virtual ~AbstractApplication();
        private:
            AbstractApplication(const AbstractApplication& rhs);
            AbstractApplication& operator=(const AbstractApplication& rhs);
            int operator==(const AbstractApplication& rhs);
            int operator!=(const AbstractApplication& rhs);
            
        public:
            // Application entry point.
            
            virtual void operator()(CParameterReader& paramReader);
            
            // Roles in the program (Strategy methods).
            
            virtual void dealer(int argc, char** argv) = 0;  // Rank 0
            virtual void farmer(int argc, char** argv) = 0;  // Rank 1
            virtual void outputter(int argc, char** argv) = 0; // Rank 2
            virtual void worker(int argc, char** argv) = 0;  // Rank 3-n.
            
            // Get message header data type
            
            MPI_Datatype& messageHeaderType();
            MPI_Datatype& requestDataType();
            
            // Services for derived classes that might override operator():
        protected:            
            int getArgc() const;
            char** getArgv();
        private:            
            void makeDataTypes();
        };
        
         
    }
}

#endif