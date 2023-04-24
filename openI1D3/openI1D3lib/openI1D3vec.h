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


#ifndef _openI1D3vec_h
#define _openI1D3vec_h

#include <iostream>
#include <vector>

class openI1D3vec
{
public:

                                            openI1D3vec(double val = 0);                        // default constructor
                                            openI1D3vec(double v0, double v1, double v2);       // constructor
                                            openI1D3vec(const openI1D3vec& copy);               // copy constructor
virtual                                    ~openI1D3vec();                                      // destructor


            void                            resize(unsigned int val);                           // set the number of elements in the openI1D3vector
            unsigned int                    size() const;                                       // return number of elements in the openI1D3vector

            openI1D3vec&                    operator= (const openI1D3vec&  copy);               // asignment
            openI1D3vec&                    operator= (const double& copy);                     // asignment
            const double&                   operator[](const unsigned int idx) const;           // index
            double&                         operator[](const unsigned int idx);                 // index
            openI1D3vec                     operator- () const;                                 // unary minus, negate eg, x = -y
                                
            int                             operator==(const openI1D3vec& val) const;           // equality
            int                             operator!=(const openI1D3vec& val) const;

 
            openI1D3vec                     operator+ (const double  val) const;
            openI1D3vec                     operator+ (const openI1D3vec& val) const;
            openI1D3vec&                    operator+=(const double  val);
            openI1D3vec&                    operator+=(const openI1D3vec& val);
    
            openI1D3vec                     operator- (const double  val) const;
            openI1D3vec                     operator- (const openI1D3vec& val) const;
            openI1D3vec&                    operator-=(const double  val);
            openI1D3vec&                    operator-=(const openI1D3vec& val);
  
            openI1D3vec                     operator* (const double  val) const;
            openI1D3vec                     operator* (const openI1D3vec& val) const;
            openI1D3vec&                    operator*=(const double  val);
            openI1D3vec&                    operator*=(const openI1D3vec& val);
    
            openI1D3vec                     operator/ (const double  val) const;
            openI1D3vec                     operator/ (const openI1D3vec& val) const;
            openI1D3vec&                    operator/=(const double  val);
            openI1D3vec&                    operator/=(const openI1D3vec& val);
    
            double                          magnitude() const;
            void                            magnitude(double mag);
            double                          normalise();            // returns magnitude
            openI1D3vec                     unit();
            double                          dot   (const openI1D3vec& val) const;
            openI1D3vec                     cross (const openI1D3vec& val) const;

    friend
    std::ostream&                           operator<< (std::ostream &os, const openI1D3vec  val);
    
protected:

    std::vector<double>                     _data;

private:

};

#endif //_openI1D3vec_h
