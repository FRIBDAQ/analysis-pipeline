/*/.
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

/** @file:  AnalysisRingItems.h
 *  @brief: Defines the analysis specific ring items
 *  @note normally a user will also include an appropriate DataFormat.h
 *    from the version of NSCLDAQ their data used.
 */
#ifndef ANALYSISRINGITEMS_H
#define ANALYSISRINGITEMS_H
#include <cstdint>
namespace frib {
    namespace analysis {
#pragma pack(push, 1)        // We want structs packed tight.
        static const unsigned MAX_UNITS_LENGTH(32);
        /**
         * Analysis ring items don't have body headers so:
         */
        typedef struct _RingItemHeader {
            std::uint32_t s_size;
            std::uint32_t s_type;
            std::uint32_t s_unused;    // must be sizeof(std::uint32_t).
        } RingItemHeader, *pRingItemHeader;
        
        /**
         * This item is a parameter definition:
         *  - sizeof is not useful.
         */
        typedef struct _ParameterDefintion {
            std::uint32_t s_parameterNumber;
            char          s_parameterName[0];   // Actually a cz string.
        } ParameterDefinition, *pParameterDefinition;
        
        /**
         *  parameter defintion ring item
         *  sizeof  is not useful.
         */
        typedef struct  _ParameterDefinitions {
            RingItemHeader s_header;
            std::uint32_t  s_numParameters;
            ParameterDefinition s_parameters [0];
        } ParameterDefinitions, *pParameterDefinitions;
        
        /**
         *    This contains the value of one parameter.
         */
        typedef struct _ParameterValue {
            std::uint32_t s_number;
            double        s_value;
        } ParameterValue, *pParameterValue;
        
        /*
         * Ring item of parameter unpacked data.
         * sizeof is worthless.
         */
        typedef struct _ParameterItem {
            RingItemHeader s_header;
            std::uint64_t  s_triggerCount;
            std::uint32_t  s_parameterCount;
            ParameterValue s_parameters[0];
        } ParameterItem, *pParameterItem;
        
        /** Variable data is used to document steering parameters:
         *
         */
        typedef struct _Variable {
            double s_value;
            char   s_variableUnits[MAX_UNITS_LENGTH];     // Fixed length
            char   s_variableName[0];       // variable length
        } Variable, *pVariable;
        
        typedef struct _VariableItem {
            RingItemHeader s_header;
            std::uint32_t  s_numVars;
            Variable       s_variables[0];
            
        } VariableItem, *pVariableItem;
        
        /* Ring Item types - these begin at 32768 (0x8000). - the first user type
         * documented in the NSCLDAQ ring item world:
         *
         *  LAST_PASSTHROUGH - ring items with types <=  are just passed through.
         *  
         */
        
        static const std::uint32_t LAST_PASSTHROUGH      = 32767;
        static const std::uint32_t PARAMETER_DEFINITIONS = 32768;
        static const std::uint32_t VARIABLE_VALUES       = 32769;
        static const std::uint32_t PARAMETER_DATA        = 32770;
        static const std::uint32_t TEST_DATA             = 32771;
        
        // MPI Message tags
        
        
        static const int  MPI_HEADER_TAG = 1;
        static const int  MPI_END_TAG  = 2;
        static const int  MPI_DATA_TAG = 3;
        static const int  MPI_REQUEST_TAG = 4;
        
        
        
#pragma pack(pop)
        // MPI Messages structures: must not be packed or MPI will pad them out
        // in messages which causes all sorts of consternation on the
        // receiving end.
        

        // Request for data message:
        
        typedef struct _FRIB_MPI_Request_Data {
            int s_requestor;                  // Rank of requestor.
            int s_maxdata;                    // Max data I can get.
        } FRIB_MPI_Request_Data, *pFRIB_MPI_Request_Data;
    
        // Data are sent with this header followed by a block of char data
        // that must be re-interpreted by the receiver:
        
        typedef struct _FRIB_MPI_Message_Header {
            unsigned s_nBytes;                       // Size of subsequent msg.
            unsigned s_nBlockNum;                    // Work Item number.
            bool s_end;                         // End data marker.
            
        } FRIB_MPI_Message_Header, *pFRIB_MPI_MessageHeader;
        
        // These typedefs are message structs that send parameters around:
        typedef struct _FRIB_MPI_Parameter_MessageHeader {
            std::uint64_t s_triggerNumber;
            std::uint32_t s_numParameters;
            bool          s_end;
        } FRIB_MPI_Parameter_MessageHeader, *pFRIB_MPI_Parameter_MessageHeader;
        typedef struct _FRIB_MPI_Parameter_Value {
            std::uint32_t s_number;
            double   s_value;
        } FRIB_MPI_Parameter_Value, *pFRIB_MPI_Parameter_Value;
        
    }
}


#endif