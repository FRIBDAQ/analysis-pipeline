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

/** @file:  MPIOutput.h
 *  @brief: Defines the MPIOutput process class.
 */
#ifndef MPIPARAMETEROUTPUT_H
#define MPIPARAMETEROUTPUT_H
#include <string>


namespace frib {
    namespace analysis {
    class AbstractApplication;               // Has defined data types.
    class CDataWriter;
    /**
     *   CMPIOutput
     *      This object can be used as is as an MPI Output process.
     *      It accepts data as pairs of messages:
     *
     *      -  FRIB_MPI_Parameter_MessageHeader which describes the following parameters
     *      -  FRIB_MPI_Parameter_Value array which contain the parameters themselves.
     *  It uses a CDataWriter to output the data it receives.  Pretty
     *  simple beast really.  If run under something derived as an abstract
     *  application class, it will have access to the parameter definitions
     *  and the data writer will write those and the variable definitions to file.
     *  
     */
    class CMPIOutput {
    private:
        AbstractApplication* m_pApp;
        CDataWriter*         m_pWriter;
    public:
        CMPIOutput();
        virtual int operator()(int argc, char** argv, AbstractApplication* app);
    protected:
        virtual std::string getOutputFile(int argc, char** argv);
        
    };
    
    }
}

#endif