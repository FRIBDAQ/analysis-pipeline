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

/** @file:  SpecTclWorker.cpp
 *  @brief: Implement the CSpecTclWorker class.
 */

#include "SpecTclWorker.h"
#include "BufferDecoder.h"
#include "Analyzer.h"
#include "Event.h"
#include "EventProcessor.h"

#include <AbstractApplication.h>
#include <AnalysisRingItems.h>
#include <TreeParameter.h>

#include <stdexcept>
#include <algorithm>
#include <sstream>
#include <cstdint>


namespace frib {
    namespace analysis {
        /**
         * constructor
         *   - initialize the index used to assign names to unnamed processors.
         *   - Create the dummy decoder, analyzer and event to pass into the pipeline
         *      elements.
         */
        CSpecTclWorker::CSpecTclWorker(AbstractApplication& app) :
            CMPIRawToParametersWorker(app),  m_unNamedIndex(0) {
            m_pDecoder  = new CBufferDecoder;
            m_pAnalyzer = new CAnalyzer;
            m_pEvent    = new CEvent;
        }
        /**
         * destructor
         *    Note that the event processor ownership is retained by the client so
         *    we leave them alone.  We do need to delete our props for the SpecTcl
         *    objects
         */
        CSpecTclWorker::~CSpecTclWorker() {
            delete m_pDecoder;
            delete m_pAnalyzer;
            delete m_pEvent;
        }
        /**
         * addProcessor
         *    Append an event processor to the end of the logical event processing
         *    pipeline.
         * @param pProcessor - Pointer to the event processor object. This storage
         *    remains owned/managed by the caller.
         * @param name - If the name is nullptr (the default) a new, unique one is generated.
         * @return std::string - name of the event processor.
         */
        std::string
        CSpecTclWorker::addProcessor(CEventProcessor* pProcessor, const char* name) {
            std::string strName;
            if (name) {
                strName = name;
            } else {
                strName = makeName();
            }
            m_pipeline.push_back(
                std::pair<std::string, CEventProcessor*>(strName, pProcessor)
            );
            return strName;
        }
        /**
         * removeEventProcessor
         *    Remove an event processor specified by a pointer to it.
         *  @param pProcessor - pointer to the one to remove.
         *  @throw std::logic_error - no such processor.
         */
        void
        CSpecTclWorker::removeEventProcessor(CEventProcessor* pProcessor) {
            auto p = std::find_if(
                m_pipeline.begin(), m_pipeline.end(),
                [pProcessor] (const std::pair<std::string, CEventProcessor*>& item) {
                    return pProcessor == item.second;
                }
            );
            removeEventProcessor(p);
            
        }
        /**
         * removeEventProcessor
         *    Removes an event processor given its name.
         *  @param name name of the event processor.
         *  @throw std::logic_error - no such processor.
         */
        void
        CSpecTclWorker::removeEventProcessor(const char* name) {
            auto p = std::find_if(
                m_pipeline.begin(), m_pipeline.end(),
                [name] (const std::pair<std::string, CEventProcessor*>& item) {
                    return item.first == name;
                }
            );
            removeEventProcessor(p);
            
        }
        /**
         * initializeUserCode
         *    Use getInputFilename to get the name of the input file and
         *    call OnInitialize and OnEventSourceOpen for all elements
         *    of the pipeline.  If any element returns kfFalse,
         *    our abort is even stronger than SpecTcl's throwing a
         *    std::runtime_error.
         *  @param argc, argv - command parameters.
         *  @param app         - application reference.
         */
        void
        CSpecTclWorker::initializeUserCode(
            int argc, char** argv, AbstractApplication& app
        ) {
            std::string fname = getInputFilename(argc, argv);
            
            // Initialize:
            
            for (auto& pr : m_pipeline) {
                if (! pr.second->OnInitialize())  {
                    std::stringstream msg;
                    msg  << pr.first << ": OnInitialize did not return a true value";
                    std::string strmsg = msg.str();
                    throw std::runtime_error(strmsg);
                }
            }
            // Now the OnEventSourceOpen:
            
            for (auto &pr : m_pipeline) {
                if (! pr.second->OnEventSourceOpen(fname)) {
                    std::stringstream msg;
                    msg  << pr.first << ": OnEventSourceOpen did not return a true value";
                    std::string strmsg = msg.str();
                    throw std::runtime_error(strmsg);
                }
            }
        }
        /**
         * unpackData
         *    Called for each ring item. Invokes the event processor pipeline
         *    element operator()'s.  On a failure, the event is reset to empty
         *    and processing continues with the next event.
         *    
         * @param pData - pointer to the ring item.
         */
        void
        CSpecTclWorker::unpackData(const void* pData) {
            void* pncData = const_cast<void*>(pData);
            m_pDecoder->setBody(pncData);
            
            // Figure out where the body of the ring item starts... That's
            // what we need to pass to the user code.
            
            union {
                pRingItemHeader pH;
                std::uint8_t*   p8;
            } p;
            p.p8 = reinterpret_cast<std::uint8_t*>(pncData);
            
            // need the size of the body header which can follow the
            // ring item header:
            // Note only 11.x and later is supported!!!
            
            std::uint32_t bhSize = p.pH->s_unused;
            if (bhSize == 0) bhSize = sizeof(std::uint32_t);  // 11.x
            
            //Point past the header:
            
            p.pH++;
            
            // Skip the body header:
            
            p.p8 += bhSize - sizeof(std::uint32_t);
            
            for (auto& pr : m_pipeline) {
                if (! (*pr.second)(p.p8, *m_pEvent, *m_pAnalyzer, *m_pDecoder)) {
                    CTreeParameter::nextEvent();     // Invalidate the whole event.
                    break;                           // run no more processors.
                }
                
            }
            
        }
        /**
         * getInputFilename
         *    Default implementation - argv[1] is the input filename.
         *    If it does not exist, std::invalid_argument is thrown.
         *  @param argc,argv - the command line.
         *  @return std::string
         *  @throw std::invalid_argument - see above.
         */
        std::string
        CSpecTclWorker::getInputFilename(int argc, char** argv) {
            if (argc >= 2) {
                return argv[1];
            } else {
                throw std::invalid_argument("Insufficient command line parameters");
            }
        }
        
        ////////////////////////////////////////////////////////////////////
        // Private utils.
        
        /**
         *  makeName
         *     Create a new name for an event processor
         * @retrn std::string
         */
        std::string
        CSpecTclWorker::makeName() {
            std::stringstream strname;
            strname << "_Unamed_." << m_unNamedIndex++;
            std::string result = strname.str();
            return result;
        }
        
        /**
         * removeEventProcessor
         *  @param p - iterator into thye pipeline defining what to remove.
         *  @throw std::logic_error if p = m_pipeline.end().
         */
        void
        CSpecTclWorker::removeEventProcessor(
            std::vector<std::pair<std::string, CEventProcessor*>>::iterator p
        ) {
            if (p != m_pipeline.end()) {
                m_pipeline.erase(p);
            } else {
                throw std::logic_error("No such event processor");
            }
        }
        
    }
    
}