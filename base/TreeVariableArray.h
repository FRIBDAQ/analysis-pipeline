///////////////////////////////////////////////////////////
//  CTreeVariableArray.h
//  Implementation of the Class CTreeVariableArray
//  Created on:      30-Mar-2005 11:03:53 AM
//  Original author: Ron Fox
///////////////////////////////////////////////////////////

/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/
// This implementation of TreeParameter is based on the ideas and original code of::
//    Daniel Bazin
//    National Superconducting Cyclotron Lab
//    Michigan State University
//    East Lansing, MI 48824-1321
//



#ifndef CTREEVARIABLEARRAY_H
#define CTREEVARIABLEARRAY_H

#include "TreeVariable.h"
#include <string>
#include <vector>

// Forward definitions.

namespace frib {
    namespace analysis {
            
            
            /**
             * This class wraps a vector of tree variables in a nice convenient package.
             * The
             * treevariables are named given a base name, one more level of hierarchy and
             * individual names like
             * nn where nn is a number with enough digits to ensure a good sort order.  For
             * example, if I have 16 pads
             * in some detector named somedetector.fp, the individual variables might have
             * names like:
             * somedetector.fp.00, somedetector.fp.01 .. somedetector.fp.10 .. somedetector.fp.
             * 15
             * 
             * The vector wrapping also allows for a first index specification much like in
             * Fortran where you can do
             * dimension a(firstdim:lastdim) .. although for 'traditional' reasons tree
             * prameters express this as firstdim,size.
             * The original implementation required firstdim positive.  Fortran and this
             * implementation make no such restrictions, allowing negative indices if this is
             * appropriate to the application.. in the example above, if the first index was
             * given as -1, the names might be e.g.:
             * somedetector.fp.-01, somedetector.fp.00 .. somedetector.fp.14
             * @author Ron Fox
             * @version 1.0
             * @created 30-Mar-2005 11:03:53 AM
             */
            class CTreeVariableArray
            {
              // object member  data:
              
            private:
              /**
               * Lowest index... negative indices can be allowed.
               */
              int m_nFirstIndex;
              /**
               * Contains the parameters that we care about.
               */
              std::vector<CTreeVariable*> m_TreeVariables;
            public:
              CTreeVariableArray();
              CTreeVariableArray(std::string baseName, double initialValue, 
                         std::string units, unsigned size, int firstIndex = 0);
              CTreeVariableArray(std::string baseName, unsigned size, int firstIndex = 0);
              CTreeVariableArray(const CTreeVariableArray& rhs);
              ~CTreeVariableArray();
              void Initialize(std::string baseName, double initialValue, 
                      std::string units, unsigned size, int firstElement);
              CTreeVariable& operator[](int index);
              const CTreeVariable& operator[](int index) const;
              CTreeVariableArray& operator=(const CTreeVariableArray& rhs);
              
              unsigned size() const;
              int  firstIndex() const;
            
            protected:
              void BuildArray(std::string basename, unsigned int size, 
                      const CTreeVariable::Definition& example);
              void CopyArray(const CTreeVariableArray&  rhs);
              
            private:
              
              void DestroyArray();
              
            };
        
    }
    
}





#endif 
