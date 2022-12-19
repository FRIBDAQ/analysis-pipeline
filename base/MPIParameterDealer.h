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

/** @file: MPIParmaeterDealer.h
 *  @brief: A dealer that deals from parameter data.
 */
#ifndef MPIPARAMETERDEALER_H
#define MPIPARAMETERDEALER_H
#include <stddef.h>
#include <DataReader.h>
#include <mpi.h>

namespace frib {
    namespace analysis {
        class AbstractApplication;

        
        /**
         * @class CMPIParameterDealer
         *
         * This class deals parameter data to workers.  It is intended for use
         * when analysis requires extracting other parameters from a set of
         * existing parameters.
         *
         * The input file is assumed to be a parameter data file.
         * This means that the first two items in the file are
         * a parameter definition record and a variable definition record.
         *
         * These are sent to all workers in a push send prior to
         * reading the rest of the data.  THis allows workers to build up a
         * correspondence table between the parameter ids they have internally
         * and those in the file.
         *
         * Variables can be handled as determined by the worker.  They may
         * be discarded or override variable values in the definition file
         * at the discretion of the actual worker.  The worker may even have
         * a mechanism to determine which variables can be overridden and which cannot.
         * The dealer does not care - it just ships out the items.  Note that the
         * outputter, howver is going to assume the defintition file is the last
         * word on it as it does not know anything about what the workers are doing.
         *
         * Once the parameter and variable items are sent; send on request begins.
         * Each worker sends a request for data which is satisfied either by
         * a new block of parameter ring items or an end indicator.
         *
         * Ring items that are not parameter ring items are passed directy to the
         * outputter in a push so that they can be directly written to file.
         *
         * Once and end file indication has been gotten on the input file, further
         * requests are answered with an end indication and, when all workers have
         * gotten that we exit.
         */
        class CMPIParameterDealer {
        private:
            int    m_argc;
            char** m_argv;
            AbstractApplication* m_pApp;
            CDataReader* m_pReader;
            unsigned     m_nBlockSize;
            unsigned     m_nEndsLeft;
            
        public:
            CMPIParameterDealer(int argc, char** argv, AbstractApplication* pApp);
            virtual ~CMPIParameterDealer();
        private:
            CMPIParameterDealer(const CMPIParameterDealer& rhs);
            CMPIParameterDealer& operator=(const CMPIParameterDealer& rhs);
            int operator==(const CMPIParameterDealer& rhs);
            int operator!=(const CMPIParameterDealer& rhs);
        public:
            void operator()();
        private:
            virtual const char* getInputFile(int  argc, char** argv) const;
            virtual unsigned getBlockSize(int argc, char** argv) const;
            
            size_t sendDefinitions(const void* pData);
            size_t sendParameterDefs(const void* pData);
            size_t sendVariableValues(const void* pData);
            void sendData(size_t nItems, const void* pData);
            void sendWorkItem(const void* pData);
            void sendPassthrough(const void* pData);
            
            void sendAll(
                const void* pData, MPI_Datatype type, size_t numItems, int tag
            );
            void sendEofs();
            void sendEof();
        };
    }
}


#endif