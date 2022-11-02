///////////////////////////////////////////////////////////
//  CTreeParameterArray.h
//  Implementation of the Class CTreeParameterArray
//  Created on:      30-Mar-2005 11:03:51 AM
//  Original author: Ron Fox
///////////////////////////////////////////////////////////
/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/
// This implementation of TreeParameter is based on the ideas and original code of::
//    Daniel Bazin
//    National Superconducting Cyclotron Lab
//    Michigan State University
//    East Lansing, MI 48824-1321
//



#ifndef CTREEPARAMETERARRAY_H
#define CTREEPARAMETERARRAY_H


#include <vector>
#include <string>

namespace frib {
    namespace analysis {
        

        // Forward definitions.
        
        class CTreeParameter;
        
        /**
         * This class is a container for an array of Tree parameters.  Given a base
         * parameter name, it will create a set of parameters named basename.n  where n is
         * a number.  The number will be left filled with zeroes to ensure that it sorts
         * correctly. Eg.:
         * \verbatim
         * basename.00
         * basename.01
         * ..
         * basename.17
         * \endverbatim
         * for an 18 element array.
         * @author Ron Fox
         * @version 1.0
         * @created 30-Mar-2005 11:03:51 AM
         */
        class CTreeParameterArray
        {
        private:
        
          /**
           * Index of first element (there's really no reason this could not also be
           * negative BTW).
           */
          int m_nFirstIndex;  
          std::vector<CTreeParameter*> m_Parameters;
          
        public:
          /**
           * Vector of parameter pointers.
           */
          CTreeParameterArray();
          CTreeParameterArray(std::string baseName, 
                      unsigned  resolution, unsigned  numElements, int baseIndex);
          CTreeParameterArray(std::string baseName, 
                      unsigned  resolution, 
                      double lowLimit, double highOrWidth, 
                      std::string units, bool widthOrHighGiven, 
                      unsigned  elements, int firstIndex = 0);
          CTreeParameterArray(std::string baseName, unsigned  elements, int baseIndex = 0);
          CTreeParameterArray(std::string baseName, std::string units, 
                      unsigned  elements, int firstIndex = 0);
          CTreeParameterArray(std::string baseName, 
                      double low, double high, std::string units, 
                      unsigned  elements, int firstIndex = 0);
          CTreeParameterArray(std::string baseName, unsigned  channels, 
                      double low, double high, std::string units, 
                      unsigned  elements, int firstIndex = 0);
          ~CTreeParameterArray();
        
          CTreeParameter& operator[](int nIndex);
          void Reset();
          void Initialize(std::string baseName, unsigned  resolution, 
                  unsigned  elements, int baseIndex);
          void Initialize(std::string baseName, unsigned  resolution, 
                  double lowLimit, double widthOrHeight,
                  std::string units, bool widthOrHeightGiven, 
                  unsigned  elements, int firstIndex);
          void Initialize(std::string baseName, unsigned  elements, int firstIndex);
          void Initialize(std::string baseName, std::string units, unsigned  elements, 
                  int firstIndex);
          void Initialize(std::string baseName, double lowLimit, double highLimit, 
                  std::string units, unsigned  elements, int firstIndex);
          void Initialize(std::string baseName, unsigned  channels, 
                  double lowLimit, double highLimit, std::string units, 
                  unsigned  elements, int firstIndex);
          std::vector<CTreeParameter*>::iterator begin();
          std::vector<CTreeParameter*>::iterator end();
          unsigned  size();
          int lowIndex();
          bool isBound() const;
          void Bind();
        protected:
          void CreateParameters(std::string baseName, 
                    unsigned  size, CTreeParameter& Template);
          void DeleteParameters();
          
          
        };
        

    }
}


#endif
