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
#include "CDataReader.h"
#include <stdexcept>
#include <cstdint>
#include <vector>
#include <string.h>

static unsigned DEFAULT_BLOCKSIZE=16*1024*1024;

nameespace frib {
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
            m_nBlockSize = getBlockSize(argc, argv);
            auto name = getInputFile(argc, argv);
            m_pReader = new CDataReader(name, m_nBlockSize);
            m_nEndsLeft = m_pApp->numWorkers();
            
            auto info = m_pReader->getBlock(m_nBlockSize);
            if (info.s_nBytes == 0) {
                
                sendEofs();
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
            
            p += sendDefinitions();
            nItems -= 2;
            
            // Send the remainder of the data and then EOFS to everyone.
    
            sendData(nItems, p);
            sendEofs();
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
        CMPIParameterDealer::getInputFile(int argc, char** argv) {
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
        CMPIParameterDealer::getBlockSize(int argc, char** argv) {
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
        CMPIParamterDealer::sendDefinitions(const void* pData) {
            size_t result(0);
            std::uint8_t* p = reinterpret_cast<const std::uint8_t*>(pData);
            
            result = sendParameterDefs(p);
            p.p += result
            
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
                    const ParamterDefinition* pDef;
                    std::uint8_t*             p8;
                } p;
                p.pDef = pDefs->s_parameters;
                
                for (int i =0; i < pDefs->s_numParameters) {
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
                std::vector<MPI_VariableDef> defs;
                defs.reserve(pItem->s_numVars);
                
                const Variable* pVar = pItem->s_variables;
                for (int i =0; i < pItem->s_numVars; i++) {
                    FRIB_MPI_VariableDef def;
                    def.s_value = pVar->s_value;
                    strncpy(
                        def.s_variableUnits, pVar->s_variableUnits,
                        MAX_UNITS_LENGTH
                    );
                    strncpy(def.s_name, pVar->s_varibableName, MAX_IDENT);
                    
                    defs.push_back(def);
                }
                sendAll(
                    defs.data(), m_pApp->variableDefType,
                    mItem->s_numVars, MPI_VARIABLES_TAG
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
                    sendWorkItem(pData, pItem->s_header.s_size);
                } else {
                    sendPassthroug(pItem->s_header.s_size, pData);
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
    }
}
