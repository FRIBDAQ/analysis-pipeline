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

/** @file:  MPIParametersToParametersWorker.cpp
 *  @brief: Implement the CMPIParametersToParametersWorker class
 */

#include "MPIParametersToParametersWorker.h"
#include "AnalysisRingItems.h"
#include "AbstractApplication.h"
#include "TreeParameter.h"
#include "TreeVariable.h"

#include <stdexcept>
#include <sstream>
#include <mpi.h>

namespace frib {
    namespace analysis {
        /** constructor
         *  @param argc, argv - command line parameters after MPI_Init's strip.
         *  @param pApp - pointer to the application object.
         */
        CMPIParametersToParametersWorker::CMPIParametersToParametersWorker(
            int argc, char** argv, AbstractApplication* pApp
        ) :  m_argc(argc), m_argv(argv), m_pApp(pApp)
        {}
        /**
         * destructor - The tree parameters in the tree map were dynamically
         *         created by the receipt of the parameter definition record so
         *         They must be deleted.
         */
        CMPIParametersToParametersWorker::~CMPIParametersToParametersWorker() {
            for (auto& item : m_parameterMap) {
                delete item;
            }
        }
        
        /**
         * operator()
         *    Entry  point for the worker.  The top level logic is simple:
         *    Receive the parameter definitions - those are first.
         *    Receive the variable definitions - those must be second.
         *    Recieve/process all of the events:
         */
        void CMPIParametersToParametersWorker::operator()() {
            receiveParameterDefinitions();
            receiveVariableDefinitions();
            receiveEvents();
        }
        /*---------------------------------------------------------------------
         *  protected utilities available to derived (concrete) class instances.
         */
        
        /**
         * getVariable
         *    Return the information block that describes a variable received
         *    from the dealer.
         *    
         * @param  pVarName - name of the variable we want.
         * @return CMPIParametersToParametersWorker::VariableInfo*
         * @retval nullptr - no matching variable definition.
         */
        CMPIParametersToParametersWorker::VariableInfo*
        CMPIParametersToParametersWorker::getVariable(const char* pVarName) {
            VariableInfo* result = nullptr;
            auto p = m_variableMap.find(std::string(pVarName));
            if (p != m_variableMap.end()) {
                result =  &(p->second);
            }
            return result;
        }
        /**
         * loadVariable
         *    Given a variable loads its information into the current set of
         *    tree variables.
         * @param pVarName - name of the variable.
         * @throw std::invalid_argument - if there's no matching variable.
         * @note If there's no matching *locally defined* variable, we will
         *        create a new one but this new variable will not be known
         *        to the outputter.
         */
        void
        CMPIParametersToParametersWorker::loadVariable(const char* pVarName) {
            
            auto pInfo = getVariable(pVarName);
            if (pInfo) {
                CTreeVariable newVar(
                    std::string(pVarName), pInfo->second, pInfo->first
                );
            } else {
                std::stringstream s;
                s << pVarName << " was not defined in the input data";
                std::string msg = s.str();
                throw std::invalid_argument(msg);
            }
        }
        /**
         * getVariableNames
         *    Return a vector of variable names received from the dealer.
         * @return std::vector<std::string>
         */
        std::vector<std::string>
        CMPIParametersToParametersWorker::getVariableNames() {
            std::vector<std::string> result;
            for(auto& p : m_variableMap) {
                result.push_back(p.first);
            }
            
            return result;
        }
        /*---------------------------------------------------------------------
         * Private utilities.
        
        /**
         * receiveParameterDefinitions
         *    Parameter definitions are pushed to all workers by the
         *    dealer.  We take those definitions and build the
         *    tree parameter map that maps tree parameter ids in the
         *    dealer data to any matching tree parameters already defined
         *    in the worker.
         *
         *    The actual mapping is creatd/storedin m_parameterMap which is
         *    a bunch of parameter pointers indexed by parameter id in the
         *    dealer data whose members are tree parameter pointers or nullptr
         *    if there is no id in that slot.  The method loadTreeParameters
         *    takes care of actually loading that data once we have the
         *    definitions from the dealer.
         */
        void
        CMPIParametersToParametersWorker::receiveParameterDefinitions() {
            // get the number of definition records to expect:
            
            int stat;
            MPI_Status status;
            std::uint32_t numItems;
            
            stat = MPI_Recv(
                &numItems, 1, MPI_UINT32_T,
                0, MPI_PARAMDEF_TAG, MPI_COMM_WORLD, &status
            );
            m_pApp->throwMPIError(stat, "Unable to receive count of parameter definitions");
            
            // Get the definitions:
            
            std::vector<FRIB_MPI_ParameterDef> paramDefs;
            paramDefs.resize(numItems);
            stat = MPI_Recv(
                paramDefs.data(), numItems, m_pApp->parameterDefType(),
                0, MPI_PARAMDEF_TAG, MPI_COMM_WORLD, &status
            );
            m_pApp->throwMPIError(stat,"Unable to receive parameter definitions from dealer");
                        
            loadTreeParameterMap(paramDefs);
        }
        /**
         * receiveVariableDefinitions
         *    Receive the push of variable definitions from the dealer and
         *    invoke loadVariableMap to stock m_variableMap with them.
         */
        void
        CMPIParametersToParametersWorker::receiveVariableDefinitions() {
            // get the numbver of definitions:
            
            int stat;
            MPI_Status status;
            std::uint32_t numItems;
            
            stat = MPI_Recv(
                &numItems, 1, MPI_UINT32_T,
                0, MPI_VARIABLES_TAG, MPI_COMM_WORLD, &status
            );
            m_pApp->throwMPIError(stat, "Unable to recive count of variable definitions");
            
            // Now the definitions themselves:
            
            std::vector<FRIB_MPI_VariableDef> defs;
            defs.resize(numItems);
            stat = MPI_Recv(
                defs.data(), numItems, m_pApp->variableDefType(),
                0, MPI_VARIABLES_TAG, MPI_COMM_WORLD, &status
            );
            m_pApp->throwMPIError(stat, "Unable to receive variable definitions");
            
            loadVariableMap(defs);
        }
        /**
         * receiveEvents
         *    -   Request an event from the dealer.
         *    -   Get the parameter encoded event.
         *    -   Load the event into the tree parameters.
         *    -   invoke process (user written code).
         *    -   Marshall the resulting event from the tree parameters.
         *    -   Push it to the farmer.
         *    -   Keep doing this until the dealer sends us an end item...which
         *        we also push to the farmer so it knows a single worker is done.
         */
        void
        CMPIParametersToParametersWorker::receiveEvents() {
            while(1) {
                // Request data and get the header.
                // If it's an end mark then we can end the loop.
                m_pApp->requestData(1024*1024);    // Size is actually ignored now.
                FRIB_MPI_Parameter_MessageHeader hdr;
                int stat;
                MPI_Status status;
                
                stat = MPI_Recv(
                    &hdr, 1, m_pApp->parameterHeaderDataType(),
                    0, MPI_HEADER_TAG, MPI_COMM_WORLD, &status
                );
                m_pApp->throwMPIError(stat, "Unable to get event header");
                
                if (hdr.s_end) {
                    break;
                }
                
                // Get the data into a parameter value vector, load tree params
                // and call process:?
                
                std::vector<FRIB_MPI_Parameter_Value> data;
                data.resize(hdr.s_numParameters);
                stat = MPI_Recv(
                    data.data(), hdr.s_numParameters, m_pApp->parameterValueDataType(),
                    0, MPI_DATA_TAG, MPI_COMM_WORLD, &status
                );
                m_pApp->throwMPIError(stat, "Unable to receive parameterized event data");
                
                CTreeParameter::nextEvent();
                loadTreeParameters(data);
                process();
                sendEventToFarmer(hdr.s_triggerNumber);
            
            }
            
            sendEndToFarmer();            // No more events.
        }
        /**
         * loadTreeParameterMap
         *    Given data on the parameter name/id correspondences that are in the
         *    input file, laods m_parameterMap from the data.  Note that
         *    items for which there is no pararameter are loaded with a nullptr.
         *
         *    This takes a bit of fiddling:
         *
         *    -  First find the largest parameter id.
         *    -  resize m_parameterMap to hold that id (1+ the id).
         *        std::vector is defined to fill those with default constructed items
         *        which for pointers are nulls.
         *    -  Iterate over the definitions and create a new tree parameter
         *       for each item, putting its pointer into the appropriate slot of
         *       the parameter map vector.
         *
         *    @param params - the parameter definitions.
         */
        void
        CMPIParametersToParametersWorker::loadTreeParameterMap(
            const std::vector<FRIB_MPI_ParameterDef>& params
        ) {
            int maxId = -1;
            for (const auto& def : params) {
                if (def.s_parameterId > maxId) maxId = def.s_parameterId;
            }
            // edge case if maxId == -1 there were no definitions.
            
            if (maxId != -1) {
                m_parameterMap.clear();
                m_parameterMap.resize(maxId+1);    // Filled with nulls.
                
                for (const auto& def : params) {
                    m_parameterMap[def.s_parameterId] = new CTreeParameter(def.s_name);
                }
            }
        }
        /**
         * loadVariableMap
         *    The variable map may or may not eventually steer the computation.
         *    We make it available by creating m_variableMap and by
         *    providing the getVariable and loadVariable methods.
         *
         *    This method stocks the m_variableMap from the vector of variable
         *    definitions received from the dealer.
         *    
         * @param vars - references the variable definitions received.
         */
        void
        CMPIParametersToParametersWorker::loadVariableMap(
            const std::vector<FRIB_MPI_VariableDef>& vars
        ) {
            for (const auto& def : vars) {
                VariableInfo info;
                info.first = def.s_variableUnits;
                info.second = def.s_value;
                m_variableMap[std::string(def.s_name)] = info;
            }
        }
        /**
         * loadTreeParameters
         *    Given a vector of parameter values from an event, uses
         *    the parameter ids to index the m_parameterMap, find the associated
         *    tree parameter and set it with the variable.
         *
         *    There are a couple of edge cases that both stem from the potential the
         *    generator did not define all of the tree parameters in its definition
         *    'file' that it actually used:
         *
         *    -   It's possible the id doesn't 'fit' in m_parameterMap.
         *    -   It's possible the id does fit but there's no pointer.
         *
         *    In both of those cases, the inbound paramters is considered to be
         *    discardable and is.  There is at least one use case for which this
         *    parameter trimming is useful, rather than an error...as one gets
         *    deeper into the analysis of the data and the parameters become
         *    increasingly divorced from raw and closer to physcially meaningful,
         *    one can imagine discarding the raw parameters from the data.
         *    This is an option and not required of course.
         *
         *  @param params - the parameter value vector received from the dealer.
         */
        void
        CMPIParametersToParametersWorker::loadTreeParameters(
            const std::vector<FRIB_MPI_Parameter_Value>& params
        ) {
            for (const auto param : params) {
                if(param.s_number < m_parameterMap.size() &&
                   m_parameterMap[param.s_number]
                ) {
                    *m_parameterMap[param.s_number] = param.s_value;    
                }
            }
        }
        /**
         * sendEventToFarmer
         *    Pulls the event from the tree parameter, marshalls and sends it
         *    to the farmer.
         *
         *    @param trigger the trigger number.
         */
        void
        CMPIParametersToParametersWorker::sendEventToFarmer(std::uint64_t trigger) {
            auto rawEvent = CTreeParameter::collectEvent();
            
            // Build the header from what we know:
            
            FRIB_MPI_Parameter_MessageHeader hdr;
            hdr.s_triggerNumber = trigger;
            hdr.s_numParameters = rawEvent.size();
            hdr.s_end = false;
            
            // build the array of parameter values:
            
            std::vector<FRIB_MPI_Parameter_Value> data;
            data.reserve(rawEvent.size());            // Allocate only once.
            for(auto& par : rawEvent) {
                FRIB_MPI_Parameter_Value v = {
                    .s_number = par.first,
                    .s_value  = par.second
                };
                data.push_back(v);
            }
            // Now we can push the header and value to the farmer which is
            // rank 1.
            
            int status;
            status = MPI_Send(
                &hdr, 1, m_pApp->parameterHeaderDataType(),
                1, MPI_HEADER_TAG, MPI_COMM_WORLD
            );
            m_pApp->throwMPIError(status, "Unable to send parameter data header to farmer");
            
            status = MPI_Send(
                data.data(), data.size(), m_pApp->parameterValueDataType(),
                1, MPI_DATA_TAG, MPI_COMM_WORLD
            );
            m_pApp->throwMPIError(status, "Unable to send parameter data to farmer");
        }
        /**
         * sendEndToFarmer
         *   Let's the farmer know this worker won't be sending any more data.
         */
        void 
        CMPIParametersToParametersWorker::sendEndToFarmer() {
            FRIB_MPI_Parameter_MessageHeader hdr;
            hdr.s_triggerNumber = 0;
            hdr.s_numParameters = 0;
            hdr.s_end = true;
            
            int status;
            status = MPI_Send(
                &hdr, 1, m_pApp->parameterHeaderDataType(),
                1, MPI_END_TAG, MPI_COMM_WORLD
            );
            m_pApp->throwMPIError(status, "Unable to send end of data message to farmer");
            
        }

    }
}