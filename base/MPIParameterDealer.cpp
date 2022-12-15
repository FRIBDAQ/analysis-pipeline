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

/** @file:  MPIParameterDealer.cpp
 *  @brief: Implement CMPIParameterDealer
 */

#include "MPIParameterDealer.h"
#include "AbstractApplication.h"
#include "AnalysisRingItems.h"
#include "DataReader.h"
#include <stdexcept>
#include <cstdint>
#include <vector>
#include <string.h>

static unsigned DEFAULT_BLOCKSIZE=16*1024*1024;

namespace frib {
    namespace analysis {
        /**
         * constructor
         *   @param argc, argv - command line words.
         *   @param pApp       - pointer to the application object.
         *   @note - this may be called prior to MPI_Init.
         *   
         */
        CMPIParameterDealer::CMPIParameterDealer(
            int argc, char** argv, AbstractApplication* pApp
        )  : m_argc(argc), m_argv(argv), m_pApp(pApp),
        m_pReader(nullptr), m_nBlockSize(0), m_nEndsLeft(0)
        {}
        /**
         * destructor
         *    Kill off the reader.
         */
        CMPIParameterDealer::~CMPIParameterDealer() {
            delete m_pReader;
        }
        
        /**
         * operator()
         *   The entry point to the code.
         */
        void
        CMPIParameterDealer::operator()() {
            m_nBlockSize = getBlockSize(m_argc, m_argv);
            auto name = getInputFile(m_argc, m_argv);
            m_pReader = new CDataReader(name, m_nBlockSize);
            m_nEndsLeft = m_pApp->numWorkers();
            
            auto info = m_pReader->getBlock(m_nBlockSize);
            if (info.s_nbytes == 0) {
                
                m_pApp->sendEofs();
                return;
            }
            // Gulp in the intial read and be sure it gets enough to send
            // the definitions.
            
            const std::uint8_t* p = reinterpret_cast<const std::uint8_t*>(info.s_pData);
            std::size_t     nItems = info.s_nItems;
            if (nItems < 2) {
                throw std::logic_error(
                    "Initial read could not fit parameter and variable descriptions"
                );
            }
            
            p += sendDefinitions(p);
            nItems -= 2;
            
            // Send the remainder of the data and then EOFS to everyone.
    
            sendData(nItems, p);
            m_pApp->sendEofs();
        }
        ////////////////////////////////////////////////////////////////////
        // Private methods
        
        /**
         * getInputFile
         *    Default way to figure out our input file (argv[1]).
         *    Can be overriden in a subclass if the user wants something
         *    different.
         *  @param argc - number of command words.
         *  @param argv - Pointers to the command words.
         *  @return const char*
         */
        const char*
        CMPIParameterDealer::getInputFile(int argc, char** argv) const {
            if (argc >= 2) {
                return argv[1];
            } else {
                throw std::invalid_argument(
                    "CMPIParameterDealer - not enough command line parameters"
                );
            }
        }
        /**
         * getBlockSize
         *    Get the size of the block that we'll read from the file:
         *  @param argc, argv - command words.
         *  @return unsigned - a hard-coded constant.  Override this method
         *  if you want somethingn different
         */
        unsigned
        CMPIParameterDealer::getBlockSize(int argc, char** argv) const {
            return DEFAULT_BLOCKSIZE;
        }
        /**
         * sendDefinitions
         *    Send the parameter and variable definitions to the workers.
         * @param pData - pointer to the ring item contaning the parameter defs.
         * @return size_t - Total size of the two ring items.
         * @note - the caller must ensure that pData points to at least two
         *  consecutive ring items.
         */
        size_t
        CMPIParameterDealer::sendDefinitions(const void* pData) {
            size_t result(0);
            const std::uint8_t* p = reinterpret_cast<const std::uint8_t*>(pData);
            
            result = sendParameterDefs(p);
            p += result;
            
            result += sendVariableValues(p);
            
            return result;
        }
        /**
         * sendParameterDefs
         *    Send parameter definitions to the workers as a push.
         *    These are sent individually rather than in a multicast
         *    because it's simpler and only needs to be done once.
         * @param pData - pointer to the parameter definition ring item.
         * @return size_t - Number of bytes in the ring item.
         * @throw std::logic_error if pData does not point at a PARAMETER_DEFINITIONS
         *   item.
         */
        size_t
        CMPIParameterDealer::sendParameterDefs(const void* pData) {
            const ParameterDefinitions* pDefs =
                reinterpret_cast<const ParameterDefinitions*>(pData);
            if (pDefs->s_header.s_type != PARAMETER_DEFINITIONS) {
                throw std::logic_error("Expected a PARAMETER_DEFINITIONS ringitem and did not get it");
            }
            
            // Send the number of defs:
            
            sendAll(&(pDefs->s_numParameters), MPI_UINT32_T, 1, MPI_PARAMDEF_TAG);
            
            // only marshall/send the defs if there are any:
            
            if (pDefs->s_numParameters) {
                std::vector<FRIB_MPI_ParameterDef> defs;
                defs.reserve(pDefs->s_numParameters);
                union {
                    const ParameterDefinition* pDef;
                    std::uint8_t*             p8;
                } p;
                p.pDef = pDefs->s_parameters;
                
                for (int i =0; i < pDefs->s_numParameters; i++) {
                    FRIB_MPI_ParameterDef item;
                    item.s_parameterId = p.pDef->s_parameterNumber;
                    strncpy(item.s_name, p.pDef->s_parameterName, MAX_IDENT);
                    defs.push_back(item);
                }
                
                sendAll(
                    defs.data(), m_pApp->parameterDefType(),
                    pDefs->s_numParameters, MPI_PARAMDEF_TAG
                );
            }
            return pDefs->s_header.s_size;
        }
        /**
         * sendVariableValues
         *    Send the variable definitions/values to all workers as a push.
         *  @param pData - ostensibly a pointer to the VARIABLE_VALUES ring item
         *     that is second in the file.
         *  @return size_t - number of bytes in the ring item.
         *  @throw std::logic_error  - if pData does not point to a VARIABLE_VALUES
         *     item.
         */
        size_t
        CMPIParameterDealer::sendVariableValues(const void* pData) {
            
            const VariableItem* pItem =
                reinterpret_cast<const VariableItem*>(pData);
            if (pItem->s_header.s_type != VARIABLE_VALUES) {
                throw std::logic_error(
                    "Expected a VARIABLE_VALUES item but got something else"
                );
            }
            
            // Send the number of variables to expect:
            
            sendAll(&(pItem->s_numVars), MPI_UINT32_T, 1, MPI_VARIABLES_TAG);
            
            // Only actuallys end variables if there are some:
            
            if (pItem->s_numVars) {
                std::vector<FRIB_MPI_VariableDef> defs;
                defs.reserve(pItem->s_numVars);
                
                const Variable* pVar = pItem->s_variables;
                for (int i =0; i < pItem->s_numVars; i++) {
                    FRIB_MPI_VariableDef def;
                    def.s_value = pVar->s_value;
                    strncpy(
                        def.s_variableUnits, pVar->s_variableUnits,
                        MAX_UNITS_LENGTH
                    );
                    strncpy(def.s_name, pVar->s_variableName, MAX_IDENT);
                    
                    defs.push_back(def);
                }
                sendAll(
                    defs.data(), m_pApp->variableDefType(),
                    pItem->s_numVars, MPI_VARIABLES_TAG
                );
            }
            
            return pItem->s_header.s_size;
        }
        /**
         * sendData
         *    Sends data on request to workers.  In this version one item is
         *    sent at a time.   Future versions we may want to marshall more
         *    than one item.  We keep reading, as needed from the input file and
         *    reuturn when a read indicates there's no more data to read.
         * @param nItems  - Number of items left  in the current block of data.
         * @param pData   - Pointer to the next item.
         * @note any non PARAMETER_DATA items are sent without interpretation
         *      to the outputter.
         */
        void
        CMPIParameterDealer::sendData(size_t nItems, const void* pData) {
            while (nItems) {
                const ParameterItem* pItem =
                    reinterpret_cast<const ParameterItem*>(pData);
                
                if (pItem->s_header.s_type == PARAMETER_DATA) {
                    sendWorkItem(pData);
                } else {
                    sendPassthrough(pData);
                }
                
                nItems--;
                if (nItems == 0) {
                    m_pReader->done();      // Release storage for re-use.
                    auto info = m_pReader->getBlock(m_nBlockSize);
                    nItems = info.s_nItems; // 0 if at EOF.
                    pData  = info.s_pData;
                    
                } else {
                    const std::uint8_t* p =
                        reinterpret_cast<const std::uint8_t*>(pData);
                    p += pItem->s_header.s_size;
                }
            }
        }
        /**
         * sendWorkItem
         *    - Marshall a work item into a message for a worker.
         *    - Accept to the next work item request from a worker and satisfy
         *      it with the parameter item we have.
         *  @param pData - pointer to what is known  to be a PARAMETER_DATA ring item
         */
        void
        CMPIParameterDealer::sendWorkItem(const void* pData) {
            const ParameterItem* pItem =
                reinterpret_cast<const ParameterItem*>(pData);
                
            // Marshall the header:
            
            FRIB_MPI_Parameter_MessageHeader header;
            header.s_triggerNumber = pItem->s_triggerCount;
            header.s_numParameters = pItem->s_parameterCount;
            header.s_end           = false;
            
            // Marshall the parameters:
            
            std::vector<FRIB_MPI_Parameter_Value> body;
            body.reserve(pItem->s_parameterCount);
            
            auto p = pItem->s_parameters;
            for (int i =0; i < pItem->s_parameterCount; i++) {
                FRIB_MPI_Parameter_Value v = {
                    .s_number = p->s_number,
                    .s_value  = p->s_value
                };
                body.push_back(v);
            }
            
            // Now we're read to respond to a request:
            
            int worker = m_pApp->getRequest();    // Send it to this rank.
            
            int status = MPI_Send(
                &header, 1, m_pApp->parameterHeaderDataType(), worker,
                MPI_HEADER_TAG,
            MPI_COMM_WORLD);
            m_pApp->throwMPIError(status, "Sending parameter header to worker");
            
            status = MPI_Send(
                body.data(),
                pItem->s_parameterCount, m_pApp->parameterValueDataType(),
                worker, MPI_DATA_TAG, MPI_COMM_WORLD
            );
        
            m_pApp->throwMPIError(status, "Sending parameter data block to worker");
        
        }
        /**
         * sendPassthrough
         *    Sends a ring item around the normal flow of work, directly to the
         *    outputter because it's not suitable for processign by workers.
         * @param pData - pointer to a ring item.
         */
        void
        CMPIParameterDealer::sendPassthrough(const void* pData) {
            const RingItemHeader* pItem =
                reinterpret_cast<const RingItemHeader*>(pData);
            
            m_pApp->forwardPassThrough(pData, pItem->s_size);
        }
        /**
         * sendAll
         *    Rather than play with making additional groups and communicators,
         *    since we only have a couple of messages to send; and only once,
         *    we just iterate over the workers to multicast the definition
         *    items to them:
         *  @param pData - pointer to the data to send.
         *  @param type  - MPI Data type of the payload
         *  @param numItesm - Number of items of *type* in pData.
         *  @param ttag  - MPI Tag to use to send the items.
         */
        void
        CMPIParameterDealer::sendAll(
            const void* pData, MPI_Datatype type, size_t numItems, int tag
        ) {
            int nextWorker = 3;    // 0 - dealer 1 - farmer  2- outputter.
            unsigned nWorkers  = m_pApp->numWorkers();
            
            for (int i =0; i < nWorkers; i++ ) {
                int status = MPI_Send(
                    pData, numItems, type, nextWorker, tag, MPI_COMM_WORLD
                );
                m_pApp->throwMPIError(status, "Failed send in CMPIParameterDealer::sendAll");
                
                nextWorker++;
            }
        }
    }    
}
