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

/** @file:  TreeVariable.h
 *  @brief: Define a class that holds variables that steer computations.
 */
#ifndef TREEVARIABLE_H
#define TREEVARIABLE_H

#include <string>
#include <map>
#include <vector>

namespace frib {
    namespace analysis {
        /**
         * @class CTreeVariable
         *    A tree variable is something that looks like a double value.
         *    It has additional metadata including:
         *    -   A name
         *    -   Units of measure
         *    -   Knowledge that the value has changed.
         *    -   Knowledge that is metadata has changed.
         *   Furthermore, as with TreeParameter's two tree variables with the same name
         *   will map to the same underlying value and metadata.
         *
         *   In normal computational use, the values of treevariables wold get established
         *   via an exteranl mechanism, such as a script and the computation would then
         *   use the value of these variables to steer computation.  An example use case
         *   might be the production of a calibrated parameter from a raw parameter and
         *   some calibration coefficients that represent the fit of an estimate of the
         *   calibration function to some polynomial.
         *
         *   As they like to say, the use of Tree variables is limited only by your
         *   imagination but, if you find yourself coding constants of creating variables
         *   that you change from run to run, consider replacing those constants/variables
         *   with a tree paramter whose values are set at run time.
         */
        class CTreeVariable {
            // Tree parameter metadata/data
            public:
                typedef struct _Definition {
                    double      s_value;    
                    std::string s_units;
                    bool        s_definitionChanged;
                    bool        s_valueChanged;
                    _Definition();
                    _Definition(double value);
                    _Definition(double value, const char* pUnits);
                    _Definition(const _Definition& rhs);
                } Definition, *pDefinition;
            // Static data:
            private:
                static std::map<std::string, Definition> m_dictionary;
            // Static private methods

            public:
                typedef std::map<std::string, Definition>::iterator
                    TreeVariableIterator;
            
            // Per object data:
            private:
                std::string m_name;
                pDefinition m_pDefinition;
            private:
                
                static Definition* createDefinition(
                    const char* name, double value, const char* pUnits
                );
                
            // static public methods
            
            public:
                static pDefinition lookupDefinition(const char* name);
                static std::vector<std::string> getNames();
                static std::vector<std::pair<std::string, const Definition*>>
                    getDefinitions();
                static TreeVariableIterator begin();
                static TreeVariableIterator end();
                static TreeVariableIterator find(std::string name);
                static size_t size();

            // Object methods
            
            CTreeVariable();
            CTreeVariable(std::string name);
            CTreeVariable(std::string name, std::string units);
            CTreeVariable(std::string name, double value, std::string units);
            CTreeVariable(std::string name, Definition& properties);
            CTreeVariable(const CTreeVariable& rhs);
            
            ~CTreeVariable();
            
            void Initialize(std::string name, double value, std::string units);

            
            void Bind();
            
            
            operator double() const;
            CTreeVariable& operator=(double rhs);
            CTreeVariable& operator=(const CTreeVariable& rhs);
            CTreeVariable& operator+=(double rhs);
            CTreeVariable& operator-=(double rhs);
            CTreeVariable& operator*=(double rhs);
            CTreeVariable& operator/=(double rhs);
            double operator++(int dummy);
            CTreeVariable& operator++();
            double operator--(int dummy);
            CTreeVariable& operator--();
            
            std::string getName() const;
            double getValue() const;
            void setValue(double newValue);
            std::string getUnit() const;
            void setUnit(const char* pUnits);
            bool hasChanged() const;
            bool valueChanged() const;
            void resetChanged();
            
            
        };            
                
    }
}


#endif