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

/** @file:  TCLParameterReader.h
 *  @brief: Implement a parameter/variable reader for Tcl based config files.
 */
#ifndef TCLPARAMETERREADER_H
#define TCLPARAMETERREADER_H

#include "ParameterReader.h"
#include <TCLObjectProcessor.h>

class CTCLInterpreter;

namespace frib {
    namespace analysis {
        /**
         * @class CTCLParameterReader
         *    A parameter reader class that uses an extended Tcl interpreter
         *    to read the parameter and variable definition.  The
         *    extensions to the interpreter are four new commands:
         *
         *  -  treeparameter name low high bins units - Defines a treee parameter.
         *  -  treeparameterarray name low high bins units elements firstindex
         *  -  treevariable name value units
         *  -  treevariablearray name value units elements firstindex
         *
         *  @note that these create initial definitions but user code
         *    can modify those definitions as well.  Therefore it's normally
         *    needed to have all computational elements not only read the
         *    configuration file but to define any CTreeParameters/CTreeVariables
         *    as well...otherwise, since MPI is a multiprocessing, SPMD system,
         *    there's danger that one or more processes will operate with
         *    differing parameter/variable definitions.
         *    
         */    
        class CTCLParameterReader : public CParameterReader {
            // nested classes that implement the commands:
        private:    
            class TreeParameterCommand : public CTCLObjectProcessor {
            public:
                TreeParameterCommand(CTCLInterpreter& interp);
                int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);                
            };
            class TreeParameterArrayCommand : public CTCLObjectProcessor {
            public:
                TreeParameterArrayCommand(CTCLInterpreter& interp);
                int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);                
            };
            class TreeVariableCommand : public CTCLObjectProcessor {
            public:
                TreeVariableCommand(CTCLInterpreter& interp);
                int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);                
            };
            class TreeVariableArrayCommand : public CTCLObjectProcessor {
            public:
                TreeVariableArrayCommand(CTCLInterpreter& interp);
                int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);                
            };
            
        public:
            CTCLParameterReader(const char* pFilename);
            virtual void read();
        private:
            CTCLInterpreter* setupInterpreter();
        };
    }
}



#endif