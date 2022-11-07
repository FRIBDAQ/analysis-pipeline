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
#include <Exception.h>
#include <TCLVariable.h>

#include "TreeParameter.h"
#include "TreeParameterArray.h"
#include "TreeVariable.h"
#include "TreeVariableArray.h"
#include <stdexcept>


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
        /////////////////////// TreeParameterArrayCommand implementation ///////
        
        /**
         * constructor
         *   @param interp - interpreter on which the command is registered.
         */
        CTCLParameterReader::TreeParameterArrayCommand::TreeParameterArrayCommand(
            CTCLInterpreter& interp
        ) : CTCLObjectProcessor(interp, "treeparameterarray", TCLPLUS::kfTRUE)
        {}
        /**
         * operator()  - perform the command.
         */
        int CTCLParameterReader::TreeParameterArrayCommand::operator()(
            CTCLInterpreter& interp, std::vector<CTCLObject>& objv
        ) {
            bindAll(interp, objv);
            requireExactly(objv, 8);
            
            std::string name = objv[1];
            double low       = objv[2];
            double high      = objv[3];
            int bins         = objv[4];
            std::string units = objv[5];
            int elements     = objv[6];
            int firstindex   = objv[7];
            
            // See the note on TreeParameterCommand::operator() about why this
            // works.   Note in pulling the data out, we'll get elements
            // separate treeeparameters but the array is just a programmatic
            // convenience.
            
            CTreeParameterArray array(
                name, bins, low, high, units, elements, firstindex
            );
            
            return TCL_OK;
        }
        //////////////////////////////////// implement TreeVariableCommand:
        
        /**
         * constructor
         *    @param interp  interpreter on which to register the command.
         */
        CTCLParameterReader::TreeVariableCommand::TreeVariableCommand(
            CTCLInterpreter& interp
        ) : CTCLObjectProcessor(interp, "treevariable", TCLPLUS::kfTRUE) {
            
        }
        /**
         * operator()  - execute the command.
         */
        int
        CTCLParameterReader::TreeVariableCommand::operator()(
            CTCLInterpreter& interp, std::vector<CTCLObject>& objv
        ) {
            bindAll(interp, objv);
            requireExactly(objv, 4);
            
            std::string name = objv[1];
            double value     = objv[2];
            std::string units = objv[3];
            
            CTreeVariable v(name, value, units);
            
            
            return TCL_OK;
        }
        ///////////////////////   implement TreeVariableArrayCommand:
        
        /**
         * constructor
         *   @param interp - interpreter on which the command is registered.
         */
        CTCLParameterReader::TreeVariableArrayCommand::TreeVariableArrayCommand(
            CTCLInterpreter& interp
        ) : CTCLObjectProcessor(interp, "treevariablearray", TCLPLUS::kfTRUE) {
            
        }
        /**
         * operator() - execute the command.
         */
        int
        CTCLParameterReader::TreeVariableArrayCommand::operator()(
            CTCLInterpreter& interp, std::vector<CTCLObject>& objv
        ) {
            bindAll(interp, objv);
            requireExactly(objv, 6);
            
            std::string name = objv[1];
            double value     = objv[2];
            std::string units = objv[3];
            int elements     = objv[4];
            int firstindex   = objv[5];
            
            CTreeVariableArray a(name, value, units, elements, firstindex);
            
            return TCL_OK;
        }
        //////////////////////////////////////////////////////////////////////
        // Implement the CTCLParameterReader class:
        
        /**
         * constructor
         *     @param pFilename - name of the file to read.
         */
        CTCLParameterReader::CTCLParameterReader(const char* pFilename) :
            CParameterReader(pFilename) {}
            
        /**
         * read
         *    Read the file.
         *    - Create an interpreter.
         *    - Evaluate the file on the interpreter.
         *
         * This should result in a full set of tree parameter and tree variable
         * definitions the user code can link to.
         */
        void
        CTCLParameterReader::read() {
            CTCLInterpreter& interp(*setupInterpreter());
            CTCLVariable traceback(&interp, "errorInfo", TCLPLUS::kfFALSE);
            bool failed(false);
            std::string msg;
            try {
                interp.EvalFile(m_filename);
            }
            catch (CException& e) {
                failed = true;
                msg    = e.ReasonText();
            }
            catch (std::string m) {
                failed = true;
                msg = m;
            }
            catch (...) {
                failed = true;
                msg = "Unexpected exception type thrown";
            }
            
            // Turn all failures into an std::runtime_error:
            
            if (failed) {
                msg += "\n";
                msg += traceback.Get();
                delete &interp;
                throw std::runtime_error(msg);
            }
            delete &interp;
        }
        /**
         * setupInterpreter [private]
         *    Setup the interpreter and register the commands on it we need:
         *  @return CTCLInterpreter*  - dynamically created interpreter.
         */
        CTCLInterpreter*
        CTCLParameterReader::setupInterpreter() {
            CTCLInterpreter& interp(*new CTCLInterpreter);
            new TreeParameterCommand(interp);
            new TreeParameterArrayCommand(interp);
            new TreeVariableCommand(interp);
            new TreeVariableArrayCommand(interp);
            
            return &interp;
        }
    }
}