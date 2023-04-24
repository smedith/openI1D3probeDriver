//********************************************************************************************
//*
//*         Copyright (c) Neil Robinson. 2010
//*
//*         This software is free software; you can redistribute it and/or modify
//*         it under the terms of the GNU Lesser General Public License as published by
//*         the Free Software Foundation; either version 2.1 of the License, or (at your
//*         option) any later version.
//* 
//*         The software is distributed in the hope that it will be useful, but
//*         WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
//*         or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
//*         License for more details.
//*         
//********************************************************************************************


#ifndef _openI1D3mtx_h
#define _openI1D3mtx_h

#include <iostream>

#include "openI1D3vec.h"
#include <vector>

class openI1D3mtx
{
public:

                                            openI1D3mtx();                                      // default constructor
                                            openI1D3mtx(const openI1D3mtx& copy);               // copy constructor
virtual                                    ~openI1D3mtx();                                      // destructor


            void                            resize(unsigned int row, unsigned int col);         // set the matrix size
            unsigned int                    size()  const;                                      // return number of vectors in the matrix
            unsigned int                    sizeR() const { return size();};                    // return number of row vectors
            unsigned int                    sizeC() const { return _data[0].size();};           // return number of columns, elements in a row vector

            openI1D3mtx&                    operator= (const openI1D3mtx& copy);                // asignment
            openI1D3mtx&                    operator= (const double& copy);                     // asignment
            const openI1D3vec&              operator[](const unsigned int idx) const;           // row index
            openI1D3vec&                    operator[](const unsigned int idx);                 // row index
            openI1D3vec                     C(const unsigned int idx) const;                    // column index
            void                            C(const unsigned int idx, const openI1D3vec& val);  // column index
                                
            int                             operator==(const openI1D3mtx& val) const;           // equality
            int                             operator!=(const openI1D3mtx& val) const;

 
            openI1D3mtx                     operator+ (const double  val) const;
            openI1D3mtx                     operator+ (const openI1D3mtx& val) const;
            openI1D3mtx&                    operator+=(const double  val);
            openI1D3mtx&                    operator+=(const openI1D3mtx& val);
    
            openI1D3mtx                     operator- (const double  val) const;
            openI1D3mtx                     operator- (const openI1D3mtx& val) const;
            openI1D3mtx&                    operator-=(const double  val);
            openI1D3mtx&                    operator-=(const openI1D3mtx& val);
  
            openI1D3mtx                     operator* (const double  val) const;
            openI1D3mtx&                    operator*=(const double  val);
            openI1D3mtx                     operator/ (const double  val) const;
            openI1D3mtx&                    operator/=(const double  val);
    
            openI1D3vec                     operator* (const openI1D3vec& val) const;

            openI1D3mtx                     operator* (const openI1D3mtx& val) const;
            openI1D3mtx&                    operator*=(const openI1D3mtx& val);
    
            void                            transpose();
            void                            identity();
            double                          determinant();
            void                            adjoint();
            bool                            invert();


    friend
    std::ostream&                           operator<< (std::ostream &os, const openI1D3mtx  val);
    
protected:

    std::vector<openI1D3vec>                _data;

private:

};

#endif //_openI1D3mtx_h
