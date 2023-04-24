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


#ifndef _openI1D3spec_h
#define _openI1D3spec_h

#include <iostream>

#define OPEN_I1D3_NO_SPEC_ELEMS     401   // 401 elements, 2nm wide, 380nm -780nm inclusive
#define OPEN_I1D3_SPEC_ELEM_SPACING 1.0   // nm
#define OPEN_I1D3_FIRST_SPEC_ELEM   380   // nm 
#define OPEN_I1D3_LAST_SPEC_ELEM    780   // nm 

class openI1D3vec;
class openI1D3spec
{
public:

                                openI1D3spec();
                                openI1D3spec(const openI1D3spec &that);
                                openI1D3spec(double data[OPEN_I1D3_NO_SPEC_ELEMS]);
                                openI1D3spec(double data[OPEN_I1D3_NO_SPEC_ELEMS][3], int ch = 0);
                                openI1D3spec(const double* data, int st = 380, int num = 401);
virtual                        ~openI1D3spec();

    int                         normalise();
    
    openI1D3vec                 conv(openI1D3spec& x, openI1D3spec& y, openI1D3spec& z) const;
    int                         xyz (double& x, double& y, double& z) const;
    openI1D3vec                 xyz () const;
    int                         xyY (double& x, double& y, double& Y) const;
    openI1D3vec                         xyY () const;

    int                         clip (openI1D3spec &that);
    int                         clip (double val = 0.0);
    
    double                      sum() const;
    double                      ave() const;
    double                      maxVal() const;

                               
    const double&               operator[](const unsigned int idx) const;   // index
    double&                     operator[](const unsigned int idx);         // index

    openI1D3spec                operator=  (openI1D3spec that);
    openI1D3spec                operator+  (openI1D3spec that);
    openI1D3spec                operator-  (openI1D3spec that);
    openI1D3spec                operator*  (openI1D3spec that);
    openI1D3spec                operator/  (openI1D3spec that);
    openI1D3spec                operator+= (openI1D3spec that);
    openI1D3spec                operator-= (openI1D3spec that);
    openI1D3spec                operator*= (openI1D3spec that);
    openI1D3spec                operator/= (openI1D3spec that);
    
    openI1D3spec                operator=  (double that);
    openI1D3spec                operator+  (double that);
    openI1D3spec                operator-  (double that);
    openI1D3spec                operator*  (double that);
    openI1D3spec                operator/  (double that);
    openI1D3spec                operator+= (double that);
    openI1D3spec                operator-= (double that);
    openI1D3spec                operator*= (double that);
    openI1D3spec                operator/= (double that);
    

    double                      _data[OPEN_I1D3_NO_SPEC_ELEMS];     // from 380 to 780 nm (inclusive)
                                                                    // in 1nn incs
protected:

private:

};

std::ostream& operator<< (std::ostream &os, openI1D3spec  openI1D3spec);
std::istream& operator>> (std::istream &is, openI1D3spec &openI1D3spec);

#endif //_openI1D3spec_h
