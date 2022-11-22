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

/** @file:  MPIParameterFarmer.h
 *  @brief: Class that encapsulates a farmer for MPI unpacked parameters.
 */
#ifndef MPIPARAMETERFARMER_H
#define MPIPARAMETERFARMER_H

#include "AnalysisRingItems.h" 
namespace frib {
    namespace analysis {
        class AbstractApplication;
        /**
         * @class CMPIParameterFarmer
         *    This can be instantiated in the farmer method of thee CAbstractApplication
         *    specialization to farm parameter data items from workers and
         *    sort them before handing them off to the outputter.
         *
         *    Normal usage might be:
         * \verbatim
         *     CMPIParameterFarmer farmer(argc, argv, pApp);
         *     farmer();
         * \endverbatim
         *    The outputter rank will receive the items sent to us
         *    sorted by trigger number.
         *    Once all workers have sent us ends, we'll flush any remaining
         *    unflushed items and then send an end to the outputter.
         *    The unflushed items will be in trigger order but will probably have
         *    at least one skip (else they already would have been emitted).
         *    
         */
        class CMPIParameterFarmer {
        private:
            int m_argc;
            char** m_argv;
            AbstractApplication& m_App;
            int m_nEndsLeft;
            unsigned m_nMaxParams;
            pFRIB_MPI_Parameter_Value  m_parameterBuffer;
        public:
            CMPIParameterFarmer(int argc, char** argv, AbstractApplication& app);
            virtual ~CMPIParameterFarmer();
            
            void operator()();
        private:
            void sendEnd();
            pParameterItem getItem();
        };
    }
} 


#endif