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

/** @file:  CTreeParameter.cpp
 *  @brief: Implement the CTreeParameter class.
 */

#include "TreeParameter.h"
#include <stdexcept>
#include <algorithm>
namespace frib {
    namespace analysis {
        /**
         * static class data:
         */
        
        // m_generation is the per process event number.  It allows an O(1)
        // setting of all tree parameters to invalid.  Note that initializing it to
        // 1 solves the initial condition that all parameters are invalid because
        // their shared data will set s_generation to 0 != 1.
        
        std::uint64_t CTreeParameter::m_generation(1);   // Increments for each event.
        
        // m_parameterDictionary provides a mapping between tree parameter names
        // and the data that's shared between instances of a tree parameter that
        // have the same name.  When a tree parameters is created, it either
        // creates a new map entry or looks up the existing one if there is one
        // to set its m_pDefinition pointer.
        //
        std::map<std::string, CTreeParameter::SharedData> CTreeParameter::m_parameterDictionary;
        
        // Each tree parameter stores its data in an element of m_event, a
        // vector of double values.  As unique tree parameters are created,
        // they are assigned sequential indices into that event.
        // m_nextId keeps track of the index that will be assigned to the next
        // unique tree parameter.
        
        unsigned CTreeParameter::m_nextId(0);
        
        // The vector below contains the values that have been stored for
        // tree paramters for this event.  nextevent wil pre-size it to
        // hold the required number of elements so that pure stores can be done.
        //
    
        std::vector<double> CTreeParameter::m_event;
        
        // As tree paramters are assigned to, if their s_generation
        // is not the same as m_generation:
        //  1.   s_generation is set to m_generation.
        //  2.   The s_parameterNumber of the tree parameters is
        //       pushed back to m_scoreboard
        // Note that nexEvent, clears the m_scoreboard vector.
        // Thus m_scoreboard contains the indices of m_event that have
        // been given values this event. For large, sparse parameter spaces,
        // this speeds up collectEvent().
        
        std::vector<unsigned> CTreeParameter::m_scoreboard;
        
        // The original tree parameter had a fixed set of default specifications:
        //  low = 0, high = 100, bins = 100 units ''   In this version, the default
        ////  specifications can be set using static methods:
        //   -   setDefaultLimits
        //   -   setDefaultBins
        //   -   setDefaultUnits
        // m_defaultSpecification store those defaults:
        
        CTreeParameter::SharedData CTreeParameter::m_defaultSpecification = {
            .s_low = 0,
            .s_high = 100,
            .s_chans = 100,
            .s_units = "Chans"
        };
        /**
         * implement CTreeParameter::SharedData
         */
        /** constructor:
         * @param low, hi - limits.
         * @param bins  - number of bins.
         * @param units - units of measure.
         */
        CTreeParameter::_SharedData::_SharedData(
            double low, double hi, unsigned  chans, const char* units
        ) : s_parameterNumber(CTreeParameter::m_nextId++),
            s_low(low), s_high(hi), s_chans(chans), s_units(units),
            s_generation(CTreeParameter::m_generation - 1),
            s_changed(false)
        {}
        // Construction.
        
        CTreeParameter::_SharedData::_SharedData(const _SharedData& rhs) :
            s_parameterNumber(rhs.s_parameterNumber),
            s_low(rhs.s_low), s_high(rhs.s_high), s_chans(rhs.s_chans),
            s_units(rhs.s_units),
            s_generation(rhs.s_generation),
            s_changed(rhs.s_changed) {}
            
        // Default construction:
        
        CTreeParameter::_SharedData::_SharedData() :
            _SharedData(CTreeParameter::m_defaultSpecification) {}
        
        /**
         * Static Method implementations:
         */
        
        /**
         * nextEvent
         *    -  Increments m_generation.
         *    -  Clears m_scorecard
         *  This sufficient to set all tree parameters to invalid and to have
         *  an empty event for collectEvent().
         */
        void
        CTreeParameter::nextEvent() {
            m_generation++;
            m_scoreboard.clear();
        }
        /**
         * collectEvent
         *    Using the scoreboard and the event, return a vector of
         *    pairs that describe the event contents.  The first element
         *    of each pair is the parameter number while the second element
         *    is the parameter value for all elements parameters that have been
         *    set this event.
         * @return std::vector<std::pair<unsigned, double> - see above.
         * @note if, somehow the scoreboard has an invalid index, std::out_of_range
         *     is throw.
         */
        std::vector<std::pair<unsigned, double>>
        CTreeParameter::collectEvent() {
            std::vector<std::pair<unsigned, double>> result;
            for (auto n : m_scoreboard) {
                result.push_back({n, m_event.at(n)});
            }
            
            return result;
        }
        /**
         * setDefaultLimits
         *    Set the default  limits value that will be used for tree parameters
         *    that have not had those limits specified.
         *  @param low  - Low limit.
         *  @param high - high limit.
         */
        void
        CTreeParameter::setDefaultLimits(double low, double high) {
            m_defaultSpecification.s_low = low;
            m_defaultSpecification.s_high = high;
        }
        /**
         * setDefaultBins
         *    Set the number of bins that will be used for tree parameters that have
         *    not specified this.
         *
         *  @param bins - number of bins.
         */
        void
        CTreeParameter::setDefaultBins(unsigned bins) {
            m_defaultSpecification.s_chans = bins;
        }
        /**
         * setDefaultUnits
         *     Set the units string that will be associated with tree parameters
         *     that have not specified this information.
         * @param units -new default units.
         */
        void
        CTreeParameter::setDefaultUnits(const char* units) {
            m_defaultSpecification.s_units = units;
        }
        /**
         * BindParameters
         *    Supplied only for compatiblity - parameters are bound as they
         *    are created because the old SpecTcl parameters that are not
         *    tree parameters don't exist in this incarnation of the world.
         */
        void
        CTreeParameter::BindParameters() {}
        /**
         * setEvent
         *    Supplied only for compatibility.  This incarnation of tree paramter
         *    maintains a static vector that is the event.
         */
        void
        CTreeParameter::setEvent(...) {}
        
        /**
         * getEvent
         *    @return const std::vector<double>& - reference to the event vector.
         */
        const std::vector<double>&
        CTreeParameter::getEvent() {
            return m_event;
        }
        /**
         * getScoreboard
         *   @return const std::vector<unsigned>& - referencde to the event scoreboard.
         */
        const std::vector<unsigned>
        CTreeParameter::getScoreboard() {
            return m_scoreboard;
        }
        /**
         * Private static methods
         */
        
        /**
         * lookupParameter
         *    @param string -name of a tree parameter.
         *    @return pSharedData - Pointer to its shared data in the parameter dictionary.
         *    @retval nullptr -if the parameter does not (yet) exist.
         */
        CTreeParameter::pSharedData
        CTreeParameter::lookupParameter( const std::string& name) {
            CTreeParameter::pSharedData result = nullptr;
            auto p = m_parameterDictionary.find(name);
            if (p != m_parameterDictionary.end()) {
                result = &(p->second);
            }
            return result;
        }
        /**
         * makeSharedData
         *    Enter shared data for a new tree parameter.
         *  @param name -name of the parameter.
         *  @param low, high - parameter low/high limits.
         *  @param chans - channels.
         *  @param units - Units of measure.
         *  @return pSharedData - pointer to the complete shared data item created.
         *  @note - if this parameter already exists an std::logic_error is thrown.
         *  @note - a parameter number is assigned.
         *  @note - the generation is set  m_generation -1 which ensures the
         *         CTreeParameter is invalid for the current event.
         *         
         */
         CTreeParameter::pSharedData
         CTreeParameter::makeSharedData(
            const std::string& name,
            double low, double high, unsigned chans, const char* units
        ) {
            SharedData data(low, high, chans, units);
            auto result = m_parameterDictionary.insert(std::make_pair(name, data));
            if (result.second) {
                m_event.resize(m_nextId);
                return &(result.first->second);   // pointer to the data.
            } else {
                throw std::logic_error(
                    "Attemped to make shared data for an existing tree parameter"
                );
            }
        }
        /**
         * Constructors of instances of CTreeParameter:
         */
         
        /**
         * default constructor
         *    Well actually I'd prefer to phase this out, however legacy
         *    code that constructed and then initialized might use this:
         *  - The name is set empty.
         *  - the definition point is set null.
         *
         *  Someone better invoke initialize on this before doing anything
         *  meaningful or there could be hell to pay:
         */
        CTreeParameter::CTreeParameter() :
           m_pDefinition(nullptr) {}
        
        /**
         *  construct with only the name.
         *  Initialize with all the default definitions.
         *
         * @param name - name of the object.
         */
        CTreeParameter::CTreeParameter(std::string name) {
            Initialize(name);
        }
        /**
         * construct with the units overriding the defaults.
         *
         * @param name - tree parameter name.
         * @param units - units of measure.
         */
        CTreeParameter::CTreeParameter(std::string name, std::string units) {
            Initialize(name, units);
        }
        /**
         * construct withy low, high and units overriding the defaaults.
         *
         * @param name -name of the parameter
         * @param lowLimit -  suggested low limit
         * @param highLimit - suggested high limit
         * @param units  -units of measure.
         */
        CTreeParameter::CTreeParameter(
                std::string name,
                double lowLimit, double highLimit, std::string units
        ) {
            Initialize(
                name, m_defaultSpecification.s_chans, lowLimit, highLimit,
                units
            );
        }
        /**
         * constructor fully specified:
         *
         * @param name - name of the parameter.
         * @param channels - suggested bins.
         * @param lowLimit, highLimit - suggested axis limits.
         * @param units - units of measure.
         */
        CTreeParameter::CTreeParameter(
            std::string name, unsigned channels, double lowLimit, double highLimit,
            std::string units
        ) {
            Initialize(name, channels, lowLimit, highLimit, units);
        }
        /**
         * constructor for simple raw parameter.
         *
         * @param name - name of parameter.
         * @param resolution - bits of resolution e.g. 0-2^resolution with
         *                     2^resolution channels.
         */
        CTreeParameter::CTreeParameter(
            std::string name, unsigned resolution)
        {
            
            Initialize(name, resolution);
        }
        /**
         * This constructor is no longer supported because it's too wonky, and
         * overdetermined to boot.
         * convince me otherwise.
         */
        CTreeParameter::CTreeParameter(
            std::string name, unsigned resolution, 
            double lowLimit, double widthOrHigh, 
            std::string units, bool widthOrHighGiven
        ) {
            throw std::logic_error(
                "This Tree parameter constructor is no longer supported"
            );
        }
        /**
         * Construct a tree parameter from an existing one (just a different name).
         *
         * @param name - name of the new tree parameter.
         * @param t    - the existing tree parameter that will be duplicated into
         *               the new one.
         */
        CTreeParameter::CTreeParameter(std::string name, const CTreeParameter& t) {
            Initialize(
                name, t.getBins(), t.getStart(), t.getStop(), t.getUnit()
            );
        }
        /**
         * Copy construction.
         */
        CTreeParameter::CTreeParameter(const CTreeParameter& rhs) :
            m_name(rhs.m_name), m_pDefinition(rhs.m_pDefinition) {
            
        }
        /**
         * Destructor is no-op now:
         */
        CTreeParameter::~CTreeParameter() {}
        
        /**
         *  Initialize
         *     These overloads do the actual heavy lifting of creating a tree parameter
         *     Note that all of these funnel into the fully specified Initialize.
         */
        
        
        /**
         * Initialize
         *   @param name - name of the parameter.
         *   @param resolution - the number of bits of resolution.
         */
        void
        CTreeParameter::Initialize(std::string name, unsigned resolution) {
            unsigned chans = 1 << resolution;
            double high = chans;
            
            Initialize(name, chans, 0.0, high, m_defaultSpecification.s_units);
        }
        /**
         * Initialize
         *    This is no longer available - it's over determined and wonky to boot.
         */
        void
        CTreeParameter::Initialize(std::string name, unsigned resolution, 
                    double lowLimit, double highOrWidth, std::string units, 
                    bool highOrWidthGiven
        ) {
            throw std::logic_error(
                "This version of CTreeParameter::Initialize is no longer supported"
            );
        }
        /**
         * Initialize
         *    With only the name and everything else default:
         * @param name - the parameter name.
         */
        void
        CTreeParameter::Initialize(std::string name) {
          Initialize(
                name,
                m_defaultSpecification.s_chans,
                m_defaultSpecification.s_low, m_defaultSpecification.s_high,
                m_defaultSpecification.s_units
            );
        }
        /**
         * Initialize
         *   WIth just the name and units.
         * @param name - name of the parameter.
         * @param units - units of measure
         */
        void
        CTreeParameter::Initialize(std::string name, std::string units) {
            Initialize(
                name,
                m_defaultSpecification.s_chans,
                m_defaultSpecification.s_low, m_defaultSpecification.s_high,
                units
            );
        }
        /**
         * Initialize (full)
         *    - If there's already a shared data associated with this,
         *      overwrite it with the requested metadata.
         *    - If there's not a shared data associated with this,
         *      create it.
         *  @note If the tree parameter is bound and the name is different than
         *     its current binding it will be bound to a different parameter.
         *  @param name - name of the parameter.
         *  @param channes - suggested channels
         */
        void
        CTreeParameter::Initialize(
            std::string name, unsigned channels, 
            double lowLimit, double highLimit, std::string units
        ) {
            m_name = name;
            auto pData = lookupParameter(name);
            if (pData) {
                pData->s_low   = lowLimit;
                pData->s_high  = highLimit;
                pData->s_chans = channels;
                pData->s_units = units;
            } else {
                pData = makeSharedData(name, lowLimit, highLimit, channels, units.c_str());
            }
            m_pDefinition = pData;
        }
        /**
         * isBound
         *    A parameter is bound if it has shared data:
         * @return bool - true if the parameter is bound.
         */
        bool
        CTreeParameter::isBound() const {
            return m_pDefinition != nullptr;
        }
        /**
         * Support for treating instances as doubles.  Note all fetches
         * are via getValue and sets via setValue so the validity handling
         * is done in a single spot.
         */
        
        /**
         * operator double()
         *    Convert value to a double.
         * @return double.
         * @throw std::Logic_error if the value is invalid or not bound.
         */
        CTreeParameter::operator double() const {
            return getValue();
        }
        /**
         * operator= - assign from double.
         * @param newValue -the double to assign to this.
         * @return *this.
         */
        CTreeParameter&
        CTreeParameter::operator=(double newValue) {
            setValue(newValue);
            return *this;
        }
        /**
         * operator= assign from another tree parameter.
         * @param rhs - tree parameter to get the data fromm.
         * @return *this.
         */
        CTreeParameter&
        CTreeParameter::operator=(const CTreeParameter& rhs) {
            setValue(rhs.getValue());
            return *this;
        }
        /**
         * operator+=
         *    Add to from double
         * @param rhs - double to add to this.
         */
        CTreeParameter&
        CTreeParameter::operator+=(double rhs) {
            setValue(getValue() + rhs);
            return *this;
        }
        /**
         * operator-=
         *    Subtract double from us.
         *  @param rhs value to subtract
         */
        CTreeParameter&
        CTreeParameter::operator-=(double rhs) {
            setValue(getValue() - rhs);
            return *this;
        }
        /**
         * operator *=
         * @param rhs value ot multiply this by
         */
        CTreeParameter&
        CTreeParameter::operator*=(double rhs) {
            setValue(getValue() * rhs);
            return *this;
        }
        /**
         * operator /=
         * @param rhs value to divide us by
         */
        CTreeParameter&
        CTreeParameter::operator/=(double rhs) {
            setValue(getValue()/rhs);
            return *this;
        }
        // post increment.
        // @return double since that's the only way we can get postincrement
        //    semantics.
        //
        double
        CTreeParameter::operator++(int dummy) {
            double result = getValue();
            setValue(result + 1);
            return result;
        }
        /**
         * pre-increment
         * @return *this
         */
        CTreeParameter&
        CTreeParameter::operator++() {
            setValue(getValue() + 1);
            return *this;
        }
        /**
         * post decrement
         * @return double
         */
        double
        CTreeParameter::operator--(int dummy) {
            double result = getValue();
            setValue(result - 1);
            return result;
        }
        /**
         * pre-decrement
         * @return *this
         */
        CTreeParameter&
        CTreeParameter::operator--() {
            setValue(getValue() - 1);
            return *this;
        }
        /**
         * getters/setters and testers.
         */
        
         /**
          * getName
          *   @return std::string - the tree parameter name.
          */
         std::string
         CTreeParameter::getName() const
         {
            return m_name;
         }
        /**
         * getId
         *   - Requires that the parameter is bound to a slot.
         * @return unsigned - parameter number binding
         * @throws std::logic_error if the parameter is not bound.
         */
        unsigned
        CTreeParameter::getId() const
        {
           if (!isBound()) {
               throw std::logic_error(
                   "Tree parameter must be bound to call getId()"
               );
           }
            return m_pDefinition->s_parameterNumber;
        }
        /**
         *  getValue
         *     Return the current value of the parameter.
         *     - The parameter must be bound.
         *     - The parameter must have been assigned to for this
         *       event.
         * @return double - the parameter's value for this event.
         * @throw std::logic_error - the parameter is not bound.
         * @throw std::range_error - the parameter has no value yet.
         */
        double
        CTreeParameter::getValue() const
        {
            if (!isBound()) {
                throw std::logic_error(
                    "Tree parameter must be bound to call getValue"
                );
            }
            if (!isValid()) {
                throw std::range_error(
                    "Tree parameter does not have a valid value in getValue"
                );
            }
            return m_event.at(m_pDefinition->s_parameterNumber);
        }
        /**
         * setValue
         *    Set the parameter to a new value and do the book-keeping needed
         *    to ensure that the world knows our value is valid.
         *
         * @param newValue - new value to assign to *this.
         * @throw std::logic_error if the tree parameter is not yet bound.
         */
        void
        CTreeParameter::setValue(double newValue) {
            if (!isBound()) {
                throw std::logic_error(
                    "Tree parameter must be bound to call setValue"
                );
            }
            m_event.at(m_pDefinition->s_parameterNumber) = newValue;
            
            // If we were not valid before we are now:
            
            if (m_pDefinition->s_generation != m_generation) {
                m_pDefinition->s_generation = m_generation;
                m_scoreboard.push_back(m_pDefinition->s_parameterNumber);
            }
        }
        /**
         * getBins
         *   @return unsigned  - number of bins suggested for this parameter.
         *   @throw std::logic_error - if we are not yet bound.
         */
        unsigned
        CTreeParameter::getBins() const {
            if (!isBound()) {
                throw std::logic_error(
                    "Tree parameter must be bound to call getBins"
                );
            }
            return m_pDefinition->s_chans;
        }
        /**
         * setBins
         *    Set a new suggested bin count for the parameter.
         *    This also sets the changed flag.
         * @param channels - new suggested binning.
         * @throw std::logic_error - the parameter is not bound.
         * @throw std::range_error - the new channel count is zero.
         */
        void
        CTreeParameter::setBins(unsigned channels) {
            if (!isBound()) {
                throw std::logic_error(
                    "Tree parameter must be bound to call setBins"
                );
            }
            if (channels == 0) {
                throw std::range_error("Can't set bins to zero in setBins");
            }
            m_pDefinition->s_chans =  channels;
            m_pDefinition->s_changed = true;
        }
        /**
         * getStart
         *    @return double - the suggested low limit for histograms.
         *    @throw std::logic_error if not bound.
         */
        double
        CTreeParameter::getStart() const {
            if (!isBound()) {
                throw std::logic_error(
                    "Tree parameter must be bound to call getStart"
                );
            }
            return m_pDefinition->s_low;
        }
        /**
         * setStart
         *    Set a new low value for the suggested histograms.
         * @param low - new vlaue to use.
         * @throw std::logic_error -if the parameter is not yet bound.
         */
        void
        CTreeParameter::setStart(double low) {
            if (!isBound()) {
                throw std::logic_error(
                    "Tree parameter must be bound to call setStart"
                );
            }
            m_pDefinition->s_low = low;
            m_pDefinition->s_changed = true;
        }
        /**
         * getStop
         *   @return double - the suggested high axis value for this parameter.
         *   @throw std::Logic_error - if the parameter is not yet bound.
         */
        double
        CTreeParameter::getStop() const {
            if (!isBound()) {
                throw std::logic_error(
                    "Tree parameter must be bound to call getStop"
                );
            }
            return m_pDefinition->s_high;
        }
        /**
         * setStop
         *  @param high - new stop value.
         *  @throw std::logic_error -if the parameter is not yet bound.
         */
        void
        CTreeParameter::setStop(double high) {
            if (!isBound()) {
                throw std::logic_error(
                    "Tree parameter must be bound to call setStop"
                );
            }
            m_pDefinition->s_high = high;
            m_pDefinition->s_changed = true;
        }
        /**
         * getInc
         *   Get the width of a channel, in parameter value.  This is just
         *   high - low / bins.
         * @return double - width of a channel
         * @throw std::logic_error - if not bound yet.
         * @note this can be negative if the axis is 'backwards' (high < low).
         */
        double
        CTreeParameter::getInc() const {
            if (!isBound()) {
                throw std::logic_error(
                    "Tree parameter must be bound to call getInc"
                );
            }
            if (m_pDefinition->s_chans == 0) return 0;    // special case.
            return (m_pDefinition->s_high - m_pDefinition->s_low)/m_pDefinition->s_chans;
        }
        /**
         * setInc
         *    Set the channel width.  This just means adjusting bins so that the
         *    high-low/bins = inc.  If high -low == 0 we set 0 for bins.
         * @param channelWidth - noew channel width.
         * @throw std::logic_error if the parameter is not bound.
         * @note - the actual increment may only be an approximation since the
         *         bins are integer typed.
         */
        void
        CTreeParameter::setInc(double channelWidth) {
            if (!isBound()) {
                throw std::logic_error(
                    "Tree parameter must be bound to call setInc"
                );
            }
            if (channelWidth == 0.0) {
                throw std::domain_error("Cannot set increment to zero");
            }
            double range = m_pDefinition->s_high - m_pDefinition->s_low;
            if (range == 0.0) {
                m_pDefinition->s_chans = 0;
            } else {
                m_pDefinition->s_chans = range/channelWidth;
            }
            m_pDefinition->s_changed = true;            
        }
        /**
         *getUnit
         *   @return std::string - the units of measure of the parameter.
         *   @throw std::Logic_error if the parameter has not yet been bound.
         */
        std::string
        CTreeParameter::getUnit() const {
            if (!isBound()) {
                throw std::logic_error(
                    "Tree parameter must be bound to call getUnit"
                );
            }
            return m_pDefinition->s_units;
        }
        /**
         * setUnit
         *    Change the units of measure for the parameter.
         *  @param units - new units of measure.
         *  @thow std::Logic_error - if not bound.
         */
        void
        CTreeParameter::setUnit(std::string units) {
            if (!isBound()) {
                throw std::logic_error(
                    "Tree parameter must be bound to call setUnit"
                );
            }
            m_pDefinition->s_units = units;
            m_pDefinition->s_changed = true;
        }
        /**
         * isValid
         *    @return bool - true if the parameter has been set
         *    @throw std::logic_error if the parameter is not yet bound.
         */
        bool
        CTreeParameter::isValid() const {
            if (!isBound()) {
                throw std::logic_error(
                    "Tree parameter must be bound to call isValid"
                );
            }
            return m_pDefinition->s_generation == m_generation;
        }
        /**
         * setInvalid
         *    -  If the parameter is not bound throw logic_error.,
         *    -  If the parameter's generation is the same as current:
         *       #    Remove its index from the set of indices in the scoreboard.
         *       #    Set the generation to one less than current.
         */
        void
        CTreeParameter::setInvalid() {
            
            if (isValid()) {     // Checks bindings too.
                auto p = std::find(
                    m_scoreboard.begin(), m_scoreboard.end(),
                    m_pDefinition->s_parameterNumber
                );
                m_scoreboard.erase(p);
                m_pDefinition->s_generation--;   // We've established it was current.
            }
        }
        /**
         * Reset, clear
         *   Synonyms for setInvalid:
         */
        void CTreeParameter::Reset() { setInvalid(); }
        void CTreeParameter::clear() { setInvalid(); }
        
        /**
         *  hasChanged
         *     Parameter metatdata has, associated with it, a flag that indicates
         *     if it any metadata has been modified in the course of running the
         *     program.  This can be used, in large parameter spaces, to limit
         *     the set of parameter definitions that are output.
         *   @return bool  if the parameter's metadata has changed.
         *   @throw std::logic_error - if not bound.
         */
        bool
        CTreeParameter::hasChanged() const {
            if (!isBound()) {
                throw std::logic_error(
                    "Tree parameter must be bound to call hasChanged"
                );
            }
            return m_pDefinition->s_changed;
        }
        /**
         * setChanged
         *    Set the changed flag in the parameter.
         */
        void
        CTreeParameter::setChanged() {
            if (!isBound()) {
                throw std::logic_error(
                    "Tree parameter must be bound to call setChanged"
                );
            }
            m_pDefinition->s_changed = true;
        }
        /**
         * resetChanged
         *    Clear the metadata changed flag.
         */
        void
        CTreeParameter::resetChanged() {
            if (!isBound()) {
                throw std::logic_error(
                    "Tree parameter must be bound to call resetChanged"
                );
            m_pDefinition->s_changed = false;
            }
        }
        /**
         * ResetAll
         *    Synonym for nextEvent
         */
        void
        CTreeParameter::ResetAll() {
            nextEvent();
        }
        /**
         * Bind
         *    Bind the parameter to a slot.
         *    If the parameter is already bound, this is a no-op.
         *    The metadata values will be those of the default.
         */
        void
        CTreeParameter::Bind() {
            if (!isBound()) {
                m_pDefinition = makeSharedData(
                    m_name,
                    m_defaultSpecification.s_low, m_defaultSpecification.s_high,
                    m_defaultSpecification.s_chans,
                    m_defaultSpecification.s_units.c_str()
                );
            }
        }
        
    } 
}