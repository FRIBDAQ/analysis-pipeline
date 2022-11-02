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
#include <stdexcept>

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
        
        // Static data for CTreeVariable:
        
        std::map<std::string, CTreeVariable::Definition>
            CTreeVariable::m_dictionary;
            
        //   Static methods (public and private).
        
        /**
         *  creatDefinition
         *      Create a new definition struct and enter it in the dictionary.
         *   @param name - variable name.
         *   @param value - initial value
         *   @param pUnits - units string.
         *   @return pDefinition - pointer to the definition block.
         *   @throw std::logic_error if there's already a definition for name.
         */
        CTreeVariable::Definition*
        CTreeVariable::createDefinition(const char*name, double value, const char* pUnits) {
            if (m_dictionary.count(name)) {
                throw std::logic_error("createDefinition - definition already exists");
            } else {
                auto p = m_dictionary.insert(
                    std::make_pair(name, Definition(value, pUnits))
                );
                return &(p.first->second);
            }
        }
        /**
         * lookupDefinition
         *    Try to find a definition given its name
         *  @param name - name to lookup.
         *  @return pDefinition -definition of that variable.
         *  @retval nullptr -if there's no match.
         */
        CTreeVariable::pDefinition
        CTreeVariable::lookupDefinition(const char* name) {
            pDefinition result = nullptr;
            auto p = m_dictionary.find(name);
            if (p != m_dictionary.end()) {
                result = &p->second;
            }
            return result;
        }
        /**
         * getNames
         *    Collect the names of all tree variables:
         *  @return std::vector<std::string>
         */
        std::vector<std::string>
        CTreeVariable::getNames() {
            std::vector<std::string> result;
            for (auto& p : m_dictionary) {
                result.push_back(p.first);
            }
            return result;
        }
        /**
         * getDefinitions.
         *
         * @return std::vector<std::pair<std::string, const pDefinition>>
         *    first is the name of an item, second its definition.
         */
        std::vector<std::pair<std::string, const CTreeVariable::Definition*>>
        CTreeVariable::getDefinitions()
        {
            std::vector<std::pair<std::string, const Definition*>> result;
            for (auto& p : m_dictionary) {
                std::pair<std::string, const Definition*> def =
                    {p.first, &(p.second)};
                result.push_back(def);                    
            }
            return result;
        }
        /**
         * begin
         *    Provide standard iteration support for the tree variable dict.
         * @return CTreeVarialbe::TreeVariableIterator
         */
        CTreeVariable::TreeVariableIterator
        CTreeVariable::begin() {
            return m_dictionary.begin();
        }
        /**
         * end
         *   Provide standard iteration support for the tree variable dict.
         *   @return CTreeVariable::TreeVariableIterator
         */
        CTreeVariable::TreeVariableIterator
        CTreeVariable::end() {
            return m_dictionary.end();
        }
        /**
         * size
         *   @return size_t - number of unique tree variables.
         */
        size_t
        CTreeVariable::size() {
            return m_dictionary.size();
        }
        
        // Object methods:
        
        /**
         * constructor
         *    Default constructor is unbound from a data block.
         *    Initialize must be called prior to use:
         */
        CTreeVariable::CTreeVariable() :
            m_name(""), m_pDefinition(nullptr)
            {}
        /**
         * constructor
         *   
         * @param name -  name of the variable.
         * @note if this is a new variable we initialize the value - 0.0
         *       and units to an empty string, otherwise we don't touch.
         *       the metadata - on the assumption we're trying to make another
         *       reference to the original
         */
        CTreeVariable::CTreeVariable(std::string name)  :
            m_name(name), m_pDefinition(nullptr)
        {
            m_pDefinition = lookupDefinition(name.c_str());
            if (!m_pDefinition) {
                m_pDefinition = createDefinition(name.c_str(), 0.0, "");
            }
        }
        /**
         * constructor
         *    @param name - name of the variable.
         *    @param units  - Units for a new one.
         *    @note, if the variable exists, we replace the units but leave the
         *    value alone.
         */
        CTreeVariable::CTreeVariable(std::string name, std::string units) :
            m_name(name), m_pDefinition(nullptr)
        {
            m_pDefinition = lookupDefinition(name.c_str());
            if (m_pDefinition) {
                m_pDefinition->s_units= units;
            } else {
                m_pDefinition = createDefinition(name.c_str(), 0.0, units.c_str());
            }
        }
        /**
         * constructor
         *   @param name - variable name.
         *   @param value -initial variable value.
         *   @param units - units of measure.
         */
        CTreeVariable::CTreeVariable(std::string name, double value, std::string units) :
            m_pDefinition(nullptr)
        {
            Initialize(name, value, units);
        }
        /**
         * copy construction.
         */
        CTreeVariable::CTreeVariable(const CTreeVariable& rhs) :
            m_name(rhs.m_name), m_pDefinition(rhs.m_pDefinition)
        {}
        /**
         * Destructor doesn't actually have to do anything:
         */
        CTreeVariable::~CTreeVariable() {}
        
        /**
         * Initialize
         *    Do full initialization of a tree variable.
         * @param name -  name of of the variable.
         * @param value - initial value (replaces existing value).
         * @param units - units of measure(replaces any existing).
         */
        void
        CTreeVariable::Initialize(std::string name, double value, std::string units) {
            m_name = name;
            m_pDefinition = lookupDefinition(name.c_str());
            if (!m_pDefinition) {
                m_pDefinition = createDefinition(name.c_str(), value, units.c_str());
            } else {
                m_pDefinition->s_units= units;
                m_pDefinition->s_value = value;
            }
        }
        /**
         * Bind
         *    This is a no-op but preserved for compatibility with old programs.
         */
        void
        CTreeVariable::Bind() {}
        /**
         * operator double
         *    This is a call to getValue which does all the bind checking etc.
         * @return double - the value associated with this variable.
         */
        CTreeVariable::operator double() const {
            return getValue();
        }
        /** operator=
         *    Assignment from double
         *    Uses setValue which does bind checking
         * @return *this
         */
        CTreeVariable& CTreeVariable::operator=(double rhs) {
            setValue(rhs);
            return *this;
        }
        /**
         * assignment from another tree variable
         * @param rhs - the other tree variable.
         * @return *this
         */
        CTreeVariable&
        CTreeVariable::operator=(const CTreeVariable& rhs) {
            setValue(rhs.getValue());
            return *this;
        }
        /**
         * +=
         * @return *this
         */
        CTreeVariable&
        CTreeVariable::operator+=(double rhs) {
            setValue(getValue() + rhs);
            return *this;
        }
        /**
         * -=
         * @return *this
         */
        CTreeVariable&
        CTreeVariable::operator-=(double rhs) {
            setValue(getValue() - rhs);
            return *this;
        }
        /**
         * *=
         */
        CTreeVariable&
        CTreeVariable::operator*=(double rhs) {
            setValue(getValue()*rhs);
            return *this;
        }
        
        CTreeVariable&
        CTreeVariable::operator/=(double rhs) {
            setValue(getValue()/rhs);
            return *this;
        }
        
        /**
         * postincrement
         *   @return double -the value prior to the increment.
         */
        double
        CTreeVariable::operator++(int dummy) {
            double result = getValue();
            setValue(getValue() + 1);
            return result;
        }
        /** Pre increment
        * @return *this - after the increment.
        */
        CTreeVariable&
        CTreeVariable::operator++() {
            setValue(getValue() + 1);
            return *this;
        }
        /** post decrement
         */
        double
        CTreeVariable::operator--(int dummy) {
            double result = getValue();
            setValue(getValue() - 1.0);
            return result;
        }
        /** predecrement
         */
        CTreeVariable&
        CTreeVariable::operator--() {
            setValue(getValue() - 1.0);
            return *this;
        }
        /**
         * getName
         *   @return std::string -name given to the variable.
         */
        std::string
        CTreeVariable::getName() const {
            return m_name;
        }
        /**
         * getValue
         *    Return the value of the variable if it's bound else
         *    throw std::Logic_error.
         * @return double
         */
        double
        CTreeVariable::getValue() const {
            if (m_pDefinition) {
                return m_pDefinition->s_value;
            } else {
                throw std::logic_error("getValue on unbound treevariable");
            }
        }
        /**
         * setValue
         *  set a new value for the variable if bound or throw std::logic_error.
         *@param newValue
         */
        void
        CTreeVariable::setValue(double newValue) {
            if (m_pDefinition) {
                m_pDefinition->s_value = newValue;
                m_pDefinition->s_valueChanged = true;
            } else {
                throw std::logic_error("setValue on unbound treevariable");
            }
        }
        /**
         * getUnit
         *    Return units if bound else throw std::Logic_error.
         *  @return std::string
         */
        std::string
        CTreeVariable::getUnit() const {
            if (m_pDefinition) {
                return m_pDefinition->s_units;
            } else {
                throw std::logic_error("getUnit on unbound treevariable");
            }
        }
        /**
         * setUnit
         *    New units of measure
         */
        void
        CTreeVariable::setUnit(const char* pUnits) {
            if (m_pDefinition) {
                m_pDefinition->s_units = pUnits;
                m_pDefinition->s_definitionChanged = true;
            } else {
                throw std::logic_error("setUnit on unbound treevariable");
            }
        }
        /**
         * hasChanged
         *   @return bool true if the definition has changed.
         */
        bool
        CTreeVariable::hasChanged() const {
            if (m_pDefinition) {
                return m_pDefinition->s_definitionChanged;
            } else {
                throw std::logic_error("hasChanged called on unbound treevariable");
            }
        }
        /**
         * valueChanged
         * @return bool - if the value changed.
         */
        bool
        CTreeVariable::valueChanged() const {
            if (m_pDefinition) {
                return m_pDefinition->s_valueChanged;
            } else {
                throw std::logic_error("valueChanged called on unbound treevariable");
            }
        }
        /**
         * resetChanged
         *    reset the changed flags to false:
         *
         */
        void
        CTreeVariable::resetChanged() {
            if (m_pDefinition) {
                m_pDefinition->s_valueChanged = false;
                m_pDefinition->s_definitionChanged = false;
            } else {
                throw std::logic_error("resetChange called on unbound treevariable");
            }
        }
    }
}