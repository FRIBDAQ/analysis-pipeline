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

/** @file:  DataWriter.h
 *  @brief: Write data to an output file.
 */
#ifndef DATAWRITER_H
#define DATAWRITER_H
#include <vector>
#include <cstdint>

#include "TreeVariable.h"
#include "TreeParameter.h"

namespace frib {
    namespace analysis {
        /**
         * @class CDataWriter
         *    Writes data to some sink.  We assume that the
         *    parameter and variables have been set up and defined prior
         *    to our first operation as we're going to access andwrite
         *    the parameter and variable definitions to the output sink.
         *    Once that's done, we just accept data from the client and
         *    write it to our sink.  We closee the sink on destruction.
         */
        class CDataWriter {
        private:
            int m_fd;
        public:
            CDataWriter(const char* pFilename);
            CDataWriter(int fd);
            virtual ~CDataWriter();
        private:
            CDataWriter(const CDataWriter& rhs);
            CDataWriter& operator=(const CDataWriter& rhs);
            int operator==(const CDataWriter& rhs) const;
            int operator!=(const CDataWriter& rhs) const;
        public:
            void writeEvent(
                const std::vector<std::pair<unsigned, double>>& event,
                std::uint64_t eventNum
            );
            void writeItem(const void* pItem);
        private:
            void writeFrontMatter();
            void writeParameterDefs();
            void writeVariableDefs();
            size_t sizeParameterDefItem(const std::vector<std::pair<std::string, frib::analysis::CTreeParameter::SharedData>>& defs);
            size_t sizeVariableDefItem(const std::vector<std::pair<std::string, const frib::analysis::CTreeVariable::Definition*>>& defs);
            size_t sizeEvent(const std::vector<std::pair<unsigned, double>>& event);
            void writeHeader(size_t nBytes, unsigned type);
        };
    }
}

#endif