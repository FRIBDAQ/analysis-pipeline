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

/** @file:  TCLParmeterReader.cpp
 *  @brief: Implementation of the TCL Parameter reader.
 */
#include "TCLParameterReader.h"
#include <TCLObject.h>
#include <TCLInterpreter.h>

#include "TreeParameter.h"
#include "TreeParameterArray.h"
#include "TreeVariable.h"
#include "TreeVariableArray.h"


namespace frib {
    namespace analysis {
        //////////////////////////// TreeParameterCommand implementation /////
        
        /**
         *constructor
         *  @param interp - references the interpreter on which to register
         *      the command.
         */
        CTCLParameterReader::TreeParameterCommand::TreeParameterCommand(
            CTCLInterpreter& interp
        ) : CTCLObjectProcessor(interp, "treeparameter", ::TCLPLUS::kfTRUE)
        {}
        /**
         * operator() - perform the operation
         */
        int CTCLParameterReader::TreeParameterCommand::operator()(
            CTCLInterpreter& interp, std::vector<CTCLObject>& objv
        ) {
            bindAll(interp, objv);
            requireExactly(objv, 6);
            
            std::string name = objv[1];
            double      low  = objv[2];
            double      high = objv[3];
            int         bins = objv[4];
            std::string units = objv[5];
            
            /**
               This next line may seem a bit odd...creating a tree parameter which
               will get destroyed when this method exits.  Under the hood what happens
               is that this makes an entry in the tree parameter dictionary that
               associates the name with the tree parameter definition block that's
               going to be common to all tree parameters with this name.
            **/
            CTreeParameter parameter(name, bins, low, high, units);
            
            return TCL_OK;
        }
        ///////////////////////////////////////////////////////////
    }
}