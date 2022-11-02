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

/** @file:  TreeVariable.cpp
 *  @brief: Implementation of the Tree Variable class
 */
#include "TreeVariable.h"

namespace frib {
    namespace analysis {
        
        // Constructors for CTreeVariable::Definition:
        
        /**
         * Default construtor value -> 0.0, units ""
         */
        CTreeVariable::Definition::_Definition() :
            s_value(0.0), s_units(""), s_definitionChanged(false), s_valueChanged(false)
        {}
        /**
         * constructor:
         *   @param value - initial value of the variable:
         */
        CTreeVariable::Definition::_Definition(double value) :
            s_value(value), s_units(""), s_definitionChanged(false), s_valueChanged(false)
        {}
        /**
         * constructor
         *  @param value -initial value
         *  @param pUnits - unists of measure.
         */
        CTreeVariable::Definition::_Definition(double value, const char* pUnits) :
            s_value(value), s_units(pUnits), s_definitionChanged(false), s_valueChanged(false)
        {}
        /**
         * copy constructor
         */
        CTreeVariable::Definition::_Definition(const _Definition & rhs) :
            s_value(rhs.s_value), s_units(rhs.s_units),
            s_definitionChanged(rhs.s_definitionChanged),
            s_valueChanged(rhs.s_valueChanged)
        {}
         
    }
}