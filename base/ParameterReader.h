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

/** @file:  ParameterReader.h
 *  @brief: Reads a parameter/variable definition file
 */
#ifndef PARAMETERREADER_H
#define PARAMETERREADER_H
#include <string>
namespace frib {
    namespace analysis {
        /**
         * @class CParameterReader
         *    Reads parameter/variable definition files.
         *    This is an abstract base class so that definition files can be
         *    in e.g. Tcl or in Python or any other useful language.
         */
        class CParameterReader {
        protected:
            std::string m_filename;
        public:
            CParameterReader(const char* pFilename);
            virtual void read() = 0;
        };
    }
}

#endif