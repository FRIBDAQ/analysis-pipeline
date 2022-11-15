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

/** @file:  DataWriter.cpp
 *  @brief: Implement the data writer class.
 */
#include "DataWriter.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include <stdexcept>
#include <string>
#include <string.h>
#include <sstream>
#include <iostream>
#include "AnalysisRingItems.h"

namespace frib {
    namespace analysis {
        /**
         * constructor
         *   @param pFilename - path to the output file.
         */
        CDataWriter::CDataWriter(const char* pFilename) :
            m_fd(-1) {
                m_fd = creat(pFilename, S_IRUSR | S_IWUSR | S_IRGRP);
                if(m_fd < 0) {
                    const char* pReason = strerror(errno);
                    std::stringstream s;
                    s << "Failed to create : " << pFilename
                         << " : " << pReason;
                    std::string msg = s.str();
                    throw std::runtime_error(msg);
                }
                writeFrontMatter();
        }
        /**
         * constructor from fd
         *   @param fd - file descriptor already open on the output file:
         */
        CDataWriter::CDataWriter(int fd) :
            m_fd(fd)
        {
            writeFrontMatter();
        }
        
        /**
         * destructor
         */
        CDataWriter::~CDataWriter() {
            close(m_fd);
        }
        //////////////////////////////////////////////////////////////////////
        // Public methods.
        
        /**
         * writeEvent
         *   Write an event that's been marshalled into a parameter #/value set of pairs.
         * @param event -the event to write.
         */
        void CDataWriter::writeEvent(
            const std::vector<std::pair<unsigned, double>>& event,
            std::uint64_t trigger
        ) {
            size_t nBytes = sizeEvent(event);
            writeHeader(nBytes, PARAMETER_DATA);
            write(m_fd, &trigger, sizeof(trigger));
            std::uint32_t nParams = event.size();
            write(m_fd, &nParams, sizeof(nParams));
            
            // You might think you could just point to the data using
            // event.data and write event.size()*sizeof(std::pari<unsigned, double)
            // and that would write the data _but:
            // - unsigned may not be std::uint32_t.
            // - There's no assurance the pair is packed as required by the
            //   spec.

            for (auto& item : event) {
                ParameterValue v = {item.first, item.second};
                write(m_fd, &v, sizeof(v));
            }
        }
        /**
         * writeItem
         *   Write a non-event item -- this is just a passthrough item.
         * @param pItem - pointer to the item to write.
         */
        void
        CDataWriter::writeItem(const void* pItem) {
            // Item is a ring item so:
            
            const RingItemHeader* p = reinterpret_cast<const RingItemHeader*>(pItem);
            write(m_fd, p, p->s_size);
        }
        ///////////////////////////////////////////////////////////////////////
        // Private utilities:
        
        /**
         * writeFrontMatter
         *     Write the stuff at the front of every file.
         */
        void
        CDataWriter::writeFrontMatter()
        {
            writeParameterDefs();
            writeVariableDefs();
        }
        /**
         * writeParameterDefs
         *    Write definitions of all of the (tree) parameters that have been
         *    defined.
         */
        void 
        CDataWriter::writeParameterDefs()
        {
            auto defs = CTreeParameter::getDefinitions();
            size_t nBytes = sizeParameterDefItem(defs);
            writeHeader(nBytes, PARAMETER_DEFINITIONS);
            std::uint32_t n = defs.size();
            write(m_fd, &n, sizeof(n));
            for (auto& d : defs) {
                std::uint32_t number = d.second.s_parameterNumber;
                write(m_fd, &number, sizeof(number));
                write(m_fd, d.first.c_str(), d.first.size()+1);   // +1 is the null terminator.
            }
        }
        /**
         * writeVariableDefs
         *    Write a variable definitions item.
         */
        void
        CDataWriter::writeVariableDefs() {
            auto defs = CTreeVariable::getDefinitions();
            size_t nBytes = sizeVariableDefItem(defs);
            writeHeader(nBytes, VARIABLE_VALUES);
            std::uint32_t n = defs.size();
            write(m_fd, &n, sizeof(n));
            
            for (auto& d : defs) {
                write(m_fd, &d.second->s_value, sizeof(double));
                char units[MAX_UNITS_LENGTH];
                memset(units, 0, sizeof(units));
                strncpy(units, d.second->s_units.c_str(), MAX_UNITS_LENGTH);
                write(m_fd, units, MAX_UNITS_LENGTH);
                write(m_fd, d.first.c_str(), d.first.size() + 1);
            }
        }
        /**
         * sizeParameterDefItem
         *   Given the parameter definitions, determines the size of the
         *   parameter definition ring item:
         *   This consists of a fixed sized header consisting of the ring item
         *   header, and the number of parameters.  That's followed by a variable
         *   number of ParameterDefinitions, each one varying length due to the
         *   variable length of the paramter name:
         * @parma defs - Refers to the vector of parameter definition
         * @return size_t - number of bytes needed to hold the whole ring item.
         */
        size_t
        CDataWriter::sizeParameterDefItem(
            const std::vector<std::pair<std::string, CTreeParameter::SharedData>>& defs
        ) {
            size_t result = sizeof(RingItemHeader) + sizeof(std::uint32_t);
            for (auto& d : defs) {
                result += sizeof(uint32_t)            // Param #
                        +  d.first.size() + 1;   // Parameter name.
            }
            return result;
        }
        /**
         * sizeVariableDefItem
         *    Given the variable defintions/values, returns the number of bytes
         *    needed for the ring item that holds that.
         *  @param def - references the variable definition array.
         *  @return size_t - number of bytes needed in the ring item.
         * 
         */
        size_t
        CDataWriter::sizeVariableDefItem(
            const std::vector<std::pair<std::string, const CTreeVariable::Definition*>>& defs
        ) {
            size_t result = sizeof(RingItemHeader) + sizeof(std::uint32_t);
            for (auto& d : defs) {
                result += sizeof(double) + MAX_UNITS_LENGTH * sizeof(char)
                       + d.first.size() + 1;
            }
            return result;
        }
        /**
         * sizeEvent
         *   Figure out how big an event as parameters is.
         *  @param event - references the parameter data.
         *  @return size_t number of bytes for the ring item.
         */
        size_t
        CDataWriter::sizeEvent(
            const std::vector<std::pair<unsigned, double>>& event
        ) {
            size_t result = sizeof(RingItemHeader)
                + sizeof(std::uint64_t) + sizeof(std::uint32_t); // trigger/count
            result += event.size() * (sizeof(double) + sizeof(std::uint32_t));
            return result;
        }
        /**
         * writeHeader
         *    Write the header of a ring item.
         *  @param nBytes  - number of bytes in the item.
         *  @param type    - type of the item.
         */
        void
        CDataWriter::writeHeader(size_t nBytes, unsigned type) {
            RingItemHeader header = {
                std::uint32_t(nBytes), std::uint32_t(type), sizeof(std::uint32_t)
            };
            write(m_fd, &header, sizeof(header));
        }
    }
}