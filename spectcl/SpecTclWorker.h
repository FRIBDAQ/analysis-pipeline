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

/** @file:  SpecTclWorker
 *  @brief: Provide an analysis pipeline version of CMPIRawToParameterWorker
 */
#ifndef ANALYSIS_SPECTCLWORKER_H
#define ANALYSIS_SPECTCLWORKER_H
#include <MPIRawToParametersWorker.h>

#include <vector>
#include <string>
namespace frib {
    namespace analysis {
        class CEventProcessor;
        class CBufferDecoder;
        class CAnalyzer;
        class CEvent;
        /**
         * @class CSpecTclWorker
         *    The generic parallel processing framework supports a worker class
         *    that can have its function call operator (operator()) invoked
         *    for each ring item in a work block.   CSpecTclWorker extends
         *    this class to provide an analysis pipeline much like SpecTcl
         *    for ported EventProcessors.
         *
         *    The point is to provde the capability to import unpacking/analysis
         *    code from SpecTcl easily into this framework drastically speeding
         *    up the initial processing of event data.
         *
         *    Note NSCLDAQ 11.x and later are the only formats supported.
         */
        class CSpecTclWorker : public CMPIRawToParametersWorker {
        private:
            std::vector<std::pair<std::string, CEventProcessor*>> m_pipeline;
            unsigned m_unNamedIndex;
            
            // Needed for the event processors:
            
            CBufferDecoder*  m_pDecoder;
            CAnalyzer*       m_pAnalyzer;
            CEvent*          m_pEvent;
        public:
            CSpecTclWorker(AbstractApplication& app);
            virtual ~CSpecTclWorker();
            
            // Pipeline manipulation - I'd really guess add is the only one
            // that would be used.
            
            std::string addProcessor(CEventProcessor* pProcessor, const char* name=nullptr);
            void removeEventProcessor(CEventProcessor* pProcessor);
            void removeEventProcessor(const char* name);
            
            // Interfaces for the worker:
            
            virtual void initializeUserCode(
                int argc, char** argv, AbstractApplication& app
            );
            virtual void unpackData(const void* pData);
            
            // In case the filename is in some other pocket the
            // user can replace this.
            
            virtual std::string getInputFilename(int argc, char** argv);
        private:
            std::string makeName();
            void removeEventProcessor(
                std::vector<std::pair<std::string, CEventProcessor*>>::iterator p
            );
        };
    }
}

#endif