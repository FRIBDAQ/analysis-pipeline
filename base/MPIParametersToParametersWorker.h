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

/** @file:  MPIParametersToParametersWorker.h
 *  @brief: Worker framework for pipeline stage that takes params -> params.
 */
#ifndef MPIPARAMETERSTOPARAMETERSWORKER_h
#define MPIPARAMETERSTOPARAMETERSWORKER_h
#include <vector>
#include <map>
#include <cstdint>

namespace frib {
    namespace analysis {
        class AbstractApplication;
        class CTreeParameter;
        
        struct _FRIB_MPI_Parameter_Value;
        typedef _FRIB_MPI_Parameter_Value
            FRIB_MPI_Parameter_Value,
            *pFRIB_MPI_Parameter_Value;
            
        /**
         * @class CMPIParametersToParameersWorker
         *     This the worker framework for workers that transform parameter
         *     input files to parameter output files.  It is an abstract base
         *     class.  The application must, at a minimum, implement the
         *     process method to process parameter data.
         *
         *     - The worker reads the parameter definition file (well the App does that).
         *     - The worker accepts a parameter definition record from the
         *       dealer and uses that to construct a mapping between parameter
         *       ids in the file and tree parameters in the application.
         *     - The worker accepts the variable definition records and
         *       creates a map of name -> {unit/value} pairs.  This map
         *       is accessible from derived classes which can take or leave it
         *       or anything in the middle.  The logic behind this is that
         *       the point of the application may be to use different variable values.
         *       Note that utility methods available to derived classes can
         *       provide the data from the message data.  Note, however that it is
         *       the file data that goes into the output file.
         *    -  The worker than requests and gets parameter data.  Using the
         *       mappings previously constructed, tree parameters are loaded with
         *       the data in each event.
         *    -  process (the user method) is then called and the user must do
         *       the application specific computations that result in output
         *       parameers
         *    -  On return from process, the parameters are marshalled from the
         *       tree parameters and sent to the farmer.
         *    -  When data are exhausted an end record is pushed to the farmer.
         *  
         */
        class CMPIParametersToParametersWorker  {
        public:
            typedef std::pair<std::string, double>   VariableInfo;   //units/value
        private:
            std::map<std::string, VariableInfo>   m_variableMap;
            std::vector<CTreeParameter*>          m_parameterMap;
            
            int                   m_argc;
            char**                m_argv;
            AbstractApplication*  m_pApp;
        public:
            CMPIParametersToParametersWorker(
                int argc, char** argv, AbstractApplication* m_pApp
            );
            virtual ~CMPIParametersToParametersWorker();
            
            virtual void operator()();
            virtual void process() = 0;
        protected:            
            VariableInfo* getVariable(const char* pVarName);
            void loadVariable(const char* pVarName);
            std::vector<std::string> getVariableNames();
        private:
            bool getEvent(
                std::uint64_t& trigger,
                std::vector<FRIB_MPI_Parameter_Value>& params);
            void loadTreeParameters(
                const std::vector<FRIB_MPI_Parameter_Value>& params
            );
        };
    }
}

#endif