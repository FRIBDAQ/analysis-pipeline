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

/** @file:  MPIRawReader.cpp
 *  @brief: Implementy the CMPIRawReader class.
 */
#include "MPIRawReader.h"
#include "DataReader.h"
#include "AbstractApplication.h"
#include "AnalysisRingItems.h"
#include <mpi.h>
#include <stdexcept>

using namespace frib::analysis;

static const unsigned DEFAULT_BLOCKSIZE(16*1024*1024);
namespace frib {
    namespace analysis {
        /**
         * constructor
         *   @param argc, argv - command line parameters.
         *   @param pApp       - The application.
         */
        CMPIRawReader::CMPIRawReader(int argc, char** argv, AbstractApplication* pApp) :
            m_argc(argc), m_argv(argv),
            m_pApp(pApp), m_pReader(nullptr), m_nBlockSize(DEFAULT_BLOCKSIZE),
            m_nEndsLeft(pApp->numWorkers())
        {
                
            // Note that calling virtual methods from a construtor calls _our_
            // method not the polymorphic one.
            // so we can't create the reader yet.
            
        }
        /**
         * destructor - delete the reader.  The app is owned by the caller.
         */
        CMPIRawReader::~CMPIRawReader() {
            delete m_pReader;
        }
        /**
         * operator()
         *    This is the functiuonal entry point:
         *    -  Create the reader and initialize the stuff we could not in the
         *       construtor due to restrictions on when virtual methods are honored
         *    -  Use sendData to send the data until EOF.
         *    -  Use sendEofs to send the end messages until m_nEndsLeft is 0.
         */
        void CMPIRawReader::operator()()  {
            m_nBlockSize = getBlockSize(m_argc, m_argv);
            m_pReader = new CDataReader(getInputFile(m_argc, m_argv), m_nBlockSize);
            
            sendData();
            sendEofs();
        }
                /**
         * getInputFile
         *    This method is virtual - typically users will override it to
         *    get the filename from the command parameters according to their own
         *    use of argc/argv. In the default implementation,
         *    argc must be at least 2 elements long and argv[1] is the filename.
         * @param argc - number of command line parameters.
         * @param argv - command line parameters.
         * @return const char* the filename.
         */
        const char*
        CMPIRawReader::getInputFile(int argc,  char** argv) const {
            if (argc >= 2) {
                return argv[1];
            } else{
                throw std::invalid_argument("CMPIRawReader needs at least 2 command parameters.");
            }
        }
        /**
         * getBlockSize
         *    This is a virtual method so that users can override it to e.g.
         *    send in the block size on the command line.  In the default
         *    implementation we return DEFAULT_BLOCKSIZE
         * @param argc - number of command line parameters.
         * @param argv - command line parameters.
         * @return unsigned - block size in bytes.
         */
        unsigned
        CMPIRawReader::getBlockSize(int argc, char** argv) const {
            return DEFAULT_BLOCKSIZE;
        }
        /**
         * sendData
         *    - Pull blocks of data from the Reader until and end file.
         *    - Each block is analyzed to determine its first trigger count.
         *      This is done by counting physics items in the block.
         *    - Once the number of physics items has been counted,
         *      *    Read a data request.
         *      *    Satisfy it.
         *      *    Update the next trigger count
         */
        void
        CMPIRawReader::sendData() {
            unsigned firstTrigger(0);
            
            while(1) {
                
                auto descrip = m_pReader->getBlock(m_nBlockSize);
                if (descrip.s_pData)  {
                    // not eof
                    
                    unsigned triggers = countTriggers(descrip.s_pData, descrip.s_nItems);
                    sendWorkItem(descrip.s_pData, descrip.s_nbytes, firstTrigger);
                    firstTrigger += triggers;
                } else {
                    break;                 // EOF so done sending data.
                }
            }
        }
        /**
         * sendEofs
         *    Send Eofs to all the workers.
         */
        void
        CMPIRawReader::sendEofs()
        {
            while (m_nEndsLeft) {
                sendEof();
                m_nEndsLeft--;
            }
        }
        /**
         * countTriggers
         *   Given  a block of ring items, counts the number of physics items.
         * @param pData - pointer to the data.
         * @param numItems - number of ring items known to be in the block of data.
         * @return unsigned - number of physics items.
         * @note we do this without reference to NSCLDAQ's DataFormat.h
         *   to be independent of an NSCLDAQ version.
         */
        unsigned
        CMPIRawReader::countTriggers(const void* pData, size_t numItems) const {
            // NSCLDAQ stuff normally in DataFormat.h:
            
            struct ItemHeader {
                std::uint32_t s_size;
                std::uint32_t s_type;
            };
            static const unsigned PHYSICS_EVENT=30;  // s_type for physics event.
            
            // this makes stepping through pData easy:
            
            union {
                const ItemHeader* s_pHeader;
                const std::uint8_t* s_p8;
            } p;
            p.s_p8 = reinterpret_cast<const std::uint8_t*>(pData);
            
            unsigned result(0);
            
            while (numItems) {
                if (p.s_pHeader->s_type == PHYSICS_EVENT) {
                    result++;              // Count a trigger.
                }
                p.s_p8 += p.s_pHeader->s_size;
                
                numItems--;
            }
            
            return result;
        }
        /**
         * sendWorkItem
         *    - Format the header block.
         *    - Accept the next work request.
         *    - send the header and the data to the requestor.
         *  @note - in this version the maxrequest is ignored since we overlapped
         *      the read from the file bewteen adjacent work requests.  The requstor
         *      can use the header to ensure it allocates sufficient space to receive
         *      the actual data block.
         * @param pData - pointer to the data to send.
         * @param numBytes - number of bytes to be sent.
         * @param blockNum - really the number of the first trigger in the block.
         *  The requestor should number its output data sequentially using that
         *  as a starting point.  Note that if the worker deletes data, it should
         *  send an empty parameter event to the farmer with the trigger number
         *  of the deleted event.
         */
        void
        CMPIRawReader::sendWorkItem(const void* pData, size_t nBytes, unsigned blockNum)
        {
            // fill in the reply header so its ready for the request:
            
            FRIB_MPI_Message_Header header;
            header.s_nBytes = nBytes;
            header.s_nBlockNum = blockNum;
            header.s_end = false;
            
            char errorWhy[MPI_MAX_ERROR_STRING];
            int len;
            
            // get the request:
            
            int dest = getRequest();
            
            // Send the header and send the data:
            
            int status = MPI_Send(
                &header, 1, m_pApp->messageHeaderType(), dest,
                MPI_HEADER_TAG, MPI_COMM_WORLD
            );
            if (status != MPI_SUCCESS) {
                std::string msg = "Failed to send data header to worker: ";
                MPI_Error_string(status, errorWhy, &len);
                msg += errorWhy;
                throw std::runtime_error(msg);
            }
            status = MPI_Send(
                pData, nBytes, MPI_UINT8_T, dest,
                MPI_DATA_TAG, MPI_COMM_WORLD
            );
            if (status != MPI_SUCCESS) {
                std::string msg = "Failed to send data block to worker: ";
                MPI_Error_string(status, errorWhy, &len);
                msg += errorWhy;
                throw std::runtime_error(msg);
            }
        }
        /**
         * sendEof
         *   Send a single EOF in reponse to a request that we get from a worker.
         */
        void
        CMPIRawReader::sendEof()
        {
            
            FRIB_MPI_Message_Header header;
            header.s_nBytes = 0;
            header.s_nBlockNum = 0;
            header.s_end = true;
            
            char errorWhy[MPI_MAX_ERROR_STRING];
            int len;
            
            int dest = getRequest();
            
            int status = MPI_Send(
                &header, 1, m_pApp->messageHeaderType(),
                dest, MPI_HEADER_TAG, MPI_COMM_WORLD
            );
            if (status != MPI_SUCCESS) {
                std::string msg = "Failed to send end of data message to worker: ";
                MPI_Error_string(status, errorWhy, &len);
                msg += errorWhy;
                throw std::runtime_error(msg);
            }
        }
        /**
         * getRequest
         *    Read a request message from whatever worker first gets one in:
         *  @return int - rank of worker.
         */
        int
        CMPIRawReader::getRequest()
        {
            char errorWhy[MPI_MAX_ERROR_STRING];
            int len;

            FRIB_MPI_Request_Data req;
            MPI_Status info;
            int status = MPI_Recv(
                &req, 1, m_pApp->requestDataType(), MPI_ANY_SOURCE,
                MPI_ANY_TAG, MPI_COMM_WORLD, &info
            );
            if (status != MPI_SUCCESS) {
                std::string msg = "Failed to receive a data request: ";
                MPI_Error_string(status, errorWhy, &len);
                msg += errorWhy;
                throw std::runtime_error(msg);
            }
            // Consistency check the rank in the request must be the same as
            // the one that sent us the request - note in the future,
            // this could be lifted if there's an agent that determines who
            // gets the next data item:
            
            if (req.s_requestor != info.MPI_SOURCE) {
                throw std::logic_error("Mismatch between requestor in data and actual sender");
            }
            if (info.MPI_TAG != MPI_REQUEST_TAG) {
                throw std::logic_error("Request data but not a request tag");
            }
            
            // Returning this allows later support for an agent to request
            // data on behalf of another rank.
            
            return req.s_requestor;
        }
        
    }

}