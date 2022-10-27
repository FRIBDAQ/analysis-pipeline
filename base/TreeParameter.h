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

/** @file:  TreeParameter.h
 *  @brief: Define the tree parameter class.
 */
#ifndef TREEPARAMETER_H
#define TREEPARAMETER_H
#include <cstdint>
#include <string>
#include <map>
namespace frib {
    namespace analysis {
        /**
         * @class TreeParameter
         * 
         * The tree parameter class is the basis of internal parameter
         * representation in the analysis pipeline.  It mimics a double but
         * it knows when it's been set.  It also maintains a scoreboard of the
         * set indices.  It has the property that multiple instances
         * of a tree paramter will point to the same underlying parameter.
         * in a single process instance.
         *
         * @note Tree parameters are inherently _not_ threadsafe.
         */
        class CTreeParameter {
        private:
            // internal data structs.
            
            typedef struct _SharedData {
                unsigned s_parameterNumber;       // Offset into paramter vector.
                double   s_low;
                double   s_high;                  // Spectrum recommendations.
                unsigned s_chans;
                std::string s_units;
                std::uint64_t s_generation;       // Last generation it was set at.
            } SharedData, *pSharedData;
        private:
            static std::uint64_t                     m_generation;          // For O(1) reset.
            static std::map<std::string, SharedData> m_parameterDictionary; // Registered parameters.
            static std::vector<double>               m_event;               // Event data
            static std::vector<unsigned>             m_scoreboard;          // Parameters set this event.
            static SharedData                        m_defaultSpecifications;
            // object data:
        private:
            std::string   m_name;
            pSharedData   m_pDefinition;
            
            // Static methods:
        public:
            void nextEvent();               // Called to start a new event.
            static std::vector<std::pair<unsigned, double>> collectEvent();
            pSharedData lookupParameter(const std::string& name);
            pSharedData makeSharedData(
                const std::string& name,
                double low, double  high, unsigned s_chans
            );
            void setDefaultLimits(double low, double high);
            void setDefaultChans(unsigned bins);
            void setDefaultUnits(const char* units);
            
            // These are for compatibility with ancient code and don't actually
            // do anything:
            
            static void BindParameters();
            static void setEvent(...);
        
            
            // Object methods:
            //    - construction/destruction and initialization:
        public:
            CTreeParameter();
            CTreeParameter(std::string name);
            CTreeParameter(std::string name, std::string units);
            CTreeParameter(std::string name, double lowLimit, double highLimit, 
                   std::string units);
            CTreeParameter(std::string name, UInt_t channels, 
                   double lowLimit, double highLimit, std::string units);
            CTreeParameter(std::string name, UInt_t resolution);
            CTreeParameter(std::string name, UInt_t resolution, 
                   double lowLimit, double widthOrHigh, 
                   std::string units, bool widthOrHighGiven);
            CTreeParameter(std::string name, const CTreeParameter& Template);
            CTreeParameter(const CTreeParameter& rhs);
            ~CTreeParameter();
            
            
            void Initialize(std::string name, UInt_t resolution);
            void Initialize(std::string name, UInt_t resolution, 
                    double lowLimit, double highOrWidth, std::string units, 
                    bool highOrWidthGiven);
            void Initialize(std::string name);
            void Initialize(std::string name, std::string units);
            void Initialize(std::string name, UInt_t channels, 
                    double lowLimit, double highLimit, std::string units);
                      
                 
            
            bool isBound() const;
            
            // Support for various arithmetic operations:
            
            operator double();
            CTreeParameter& operator= (double newValue);
            CTreeParameter& operator= (CTreeParameter& rhs);
            CTreeParameter& operator+=(double rhs);
            CTreeParameter& operator-=(double rhs);
            CTreeParameter& operator*=(double rhs);
            CTreeParameter& operator/=(double rhs);
            double          operator++(int dummy);
            CTreeParameter& operator++();
            double          operator--(int dummy);
            CTreeParameter& operator--();
            
            // Getters and setters:
            
            std::string getName();
            int    getId();
            double getValue();
            void   setValue(double newValue);
            UInt_t getBins();
            void   setBins(UInt_t channels);
            double getStart();
            void   setStart(double low);
            double getStop();
            void   setStop(double high);
            double getInc();
            void   setInc(double channelWidth);
            std::string getUnit();
            void   setUnit(std::string units);
            bool   isValid();
            void   setInvalid();
            void   Reset();
            void   clear();
            bool   hasChanged();
            void   setChanged();
            void   resetChanged();
            static void ResetAll();
            
            void Bind();             
        };
    }
}

#endif