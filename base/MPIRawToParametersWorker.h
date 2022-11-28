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

/** @file:  MPIRawToParametersWorker.h
 *  @brief: Raw events -> Parameter worker skeleton.
 */
#ifndef MPIRAWTOPARAMETERSWORKER_H
#define MPIRAWTOPARAMETERSWORKER_H

namespace frib {
    namespace analysis {
        class AbstractApplication;
        
        /**
         * @class CMPIRawToParametersWorker
         *    This is an abstract base class for a worker that maps raw
         *    parameter data into parameter data.  The framework takes care of
         *    getting data from the dealer, asking user code to map the
         *    associated data into tree parameters and then
         *    firing those parameters on to the trigger sorter.
         *
         *    Normally this is used by deriving from this class and supplying an
         *    implementation for the unpackData method. This method is expected to
         *    analyze the ring item it is given and fill in associated tree
         *    parameters.  The tree parameters must be those which are defined
         *    in the tree parameter setup file.  Any new tree parameters can be
         *    created and output, but they will not be described in the output file
         *    which makes it possible they won't be decipherable at other stages
         *    of the analysis pipeline.
         *
         *    The very top levels of the MPI framework are expected to have
         *    read in the parameter definition file and assignment of names to
         *    ids is assumed to be deterministic.
         *
         *    Users _must_ implement unpackData as it is pure virtual.
         *    Users _may_ implement initializeUserCode if they have one-time
         *    initialization code they must run.
         *
         *    @note unpack data will only get PHYSICS_EVENT ring items.
         *          all other ring item types are treated as passthrough items
         *          and sent directly, as such, to the outputter.
         *    @note implementers that are porting SpecTcl code should look at
         *       MPISpecTclWorker which tries to allow users to re-use SpecTcl
         *         event processor code as much as possible.
         */
        class CMPIRawToParametersWorker {
        public:
            CMPIRawToParametersWorker();
            virtual ~CMPIRawToParametersWorker();
            virtual void operator()(int argc, char** argv, Application* pApp);
            virtual void initializeUserCode(int argc, char** argv, Application* pApp);
            virtual void unpackData(const void* pData);
        };
        
    }
}
#endif