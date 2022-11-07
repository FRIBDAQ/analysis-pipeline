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
#include <vector>
#include <cstdint>

/** @file:  ParameterOutputter.h
 *  @brief: Defines class to output parameter data.
 */
#ifndef PARAMETEROUTPUTTER_H
#define PARAMETEROUTPUTTER_H

namespace frib {
    namespace analysis {
        /**
         * @class ParameterOutputter
         *    This is an output class that can be used in output stages of
         *    the pipeline that output parameter data.
         *    On startup it reads a parameter definition file and
         *    converts that data into TreeParameters.  Next it
         *    outputs a parameter definition record to the output file.
         *    It then accepts data from some its client.  If the data are
         *    not parameter data then it outputs that data as is to the output file
         *    If the data are a parameter record, a parameter record is formatted
         *    and sent to the output.
         *    The presumption is that message tags can be used to separate the
         *    non event data from event data.  What is passed in event data are
         *    - the trigger number
         *    - number of items.
         *    - parameter number/value pairs.
         *    The assumption is that the same parameter definition file is used
         *    to create the parameter name/number combinations for the data generators.
         *
         *    The class is written as a bunch of virtual methods that can be
         *    overidden in a derived class and then called from something that
         *    receives the data (typically from MPI Messages).
         */
        class ParameterOutputter {
        private:
            int m_nFd;          // Output fd.
        public:
            ParameterOutputter(const char* parameterFile, int fd);
            ParameterOutputter(const char* parameterFile, const char* outputFile);
            ParameterOutputter(const ParameterOutputter& rhs);
            
            // Forbidden canonicals
        private:
            int operator==(const ParameterOuputter& rhs);
            int operator!=(const ParameterOutputter& rhs);
            ParameterOutputter& operator=(const ParameterOutputter& rhs);
        
         // strategy elements:
         
        public:   
            virtual void outputUnknownItem(size_t nBytes, const void* pData);
            virtual void outputParameters();
            virtual void outputParameterData(
                std::uint64_t triggerNumber,
                const std::vector<std::pair<unsigned, double>& parameters
            );
            virtual void getParameterDefs(const char* parameterFile);
            
        }
    }
}

#endif