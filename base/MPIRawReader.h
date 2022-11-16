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

/** @file:  MPIRawReader.h
 *  @brief: Read raw events from a CDataReader and distribute them to workers.
 */
#ifndef MPIRAWREADER_H
#define MPIRAWREADER_H

#include <stddef.h>


namespace frib {
    namespace analysis {
        class CDataReader;
        class AbstractApplication;
        /**
         * @class MPIRawReader
         *   This class is intended to be used as the dealer in an MPI
         *   application.  It accepts request from workers for data and
         *   satisfies them with raw data from an event file.
         *
         *   MPI RawReader knows the stucture of the AbstractApplication
         *   and can compute how many workers are present.  On EOF, this
         *   allows it to know how many ends to send in reply to data requests
         *   before exiting.
         *
         *   As a raw reader one thing the dealer must do is assign trigger
         *   numbers to the start of each set of ring items passed to the workers.
         *   this is passed as the block number of the FRIB_MPI_Message_Header
         *   sent as the first message in response to a request.
         *
         *   The comutation of these trigger numbers is overlapped with the
         *   request - that is we get a block to send, analyze the number
         *   triggers in it to update  m_nTriggersInBLock; and _then_
         *   get the next request
         */
        class CMPIRawReader {
        private:
            int m_argc;
            char** m_argv;
            AbstractApplication* m_pApp;
            CDataReader* m_pReader;
            unsigned     m_nBlockSize;
            unsigned     m_nEndsLeft;
        public:
            CMPIRawReader(int argc, char** argv, AbstractApplication* pApp);
            virtual ~CMPIRawReader();
        private:
            CMPIRawReader(const CMPIRawReader& rhs);
            CMPIRawReader& operator=(const CMPIRawReader& rhs);
            int operator==(const CMPIRawReader& rhs);
            int operator!=(const CMPIRawReader& rhs);
        public:
            void operator()();
        private:
            // These utilities are virtual so that the user can override them
            // to parse argc/argv differently than we do.
            //
            virtual const char* getInputFile(int argc, char** argv) const;
            virtual unsigned getBlockSize(int argc, char** argv) const;
            
            void sendData();
            void sendEofs();
            
            unsigned countTriggers(const void* pData, size_t numItems) const;
            void sendWorkItem(const void* pData, size_t nBytes, unsigned blockNum);
            void sendEof();
            int getRequest();
        };
    }
}



#endif

