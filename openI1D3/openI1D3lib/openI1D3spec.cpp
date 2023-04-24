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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef _WIN32
#include <float.h>
#endif

#include "openI1D3vec.h"
#include "openI1D3mtx.h"
#include "openI1D3spec.h"
#include "openI1D3stdSpec.h"

using namespace std;


openI1D3spec::openI1D3spec()
{
    memset(_data,0x00,sizeof(double) * OPEN_I1D3_NO_SPEC_ELEMS);
}


openI1D3spec::openI1D3spec(const openI1D3spec &that)
{
    for(int c(0); c < OPEN_I1D3_NO_SPEC_ELEMS; c++) _data[c] = that._data[c];
}


openI1D3spec::openI1D3spec(double data[OPEN_I1D3_NO_SPEC_ELEMS])
{
    for(int c(0); c < OPEN_I1D3_NO_SPEC_ELEMS; c++) _data[c] = data[c];
}


openI1D3spec::openI1D3spec(double data[OPEN_I1D3_NO_SPEC_ELEMS][3], int ch)
{
    for(int c(0); c < OPEN_I1D3_NO_SPEC_ELEMS; c++) _data[c] = data[c][ch];
}


openI1D3spec::openI1D3spec(const double* data, int st, int num)
{
    for(int c(0); c < OPEN_I1D3_NO_SPEC_ELEMS; c++)
    {
        int idx = c - (OPEN_I1D3_FIRST_SPEC_ELEM - st);
        if((idx >= 0) && (c < num)) _data[c] = data[idx];
        else _data[c] = 0;
    }
}


openI1D3spec::~openI1D3spec()
{
}


int
openI1D3spec::normalise()
{
    double sum(0);
    double min(_data[0]);
    for(int c(0); c < OPEN_I1D3_NO_SPEC_ELEMS; c++)
    {
        if(_data[c] < min) min = _data[c];
    }
    
    for(int c(0); c < OPEN_I1D3_NO_SPEC_ELEMS; c++)
    {
        _data[c] -= min;
    }
    
    for(int c(0); c < OPEN_I1D3_NO_SPEC_ELEMS; c++)
    {
        sum += _data[c];
    }
    
    for(int c(0); c < OPEN_I1D3_NO_SPEC_ELEMS; c++)
    {
        _data[c] /= sum;
    }
    
    return 1;
}


int
openI1D3spec::xyz(double& x, double& y, double& z) const
{
    x = 0;
    y = 0;
    z = 0;
    
    for(int c(0); c < OPEN_I1D3_NO_SPEC_ELEMS; c++)
    {
        x += xyzWt[c][0] * _data[c];
        y += xyzWt[c][1] * _data[c];
        z += xyzWt[c][2] * _data[c];
    }
    
    return 1;
}


openI1D3vec
openI1D3spec::conv(openI1D3spec& x, openI1D3spec& y, openI1D3spec& z) const
{
    openI1D3vec val;
    
    for(int c(0); c < OPEN_I1D3_NO_SPEC_ELEMS; c++)
    {
        val[0] += x[c] * _data[c];
        val[1] += y[c] * _data[c];
        val[2] += z[c] * _data[c];
    }
    
    return val;
}


openI1D3vec
openI1D3spec::xyz() const
{
    openI1D3vec val;
    openI1D3spec::xyz(val[0], val[1], val[2]);
    return val;
}


int
openI1D3spec::xyY(double& x, double& y, double& Y) const
{
    double X(0), Z(0);
    
    xyz(X, Y, Z);

    x = X / (X + Y + Z);
    y = Y / (X + Y + Z);
    
    return 1;
}


openI1D3vec
openI1D3spec::xyY() const
{
    openI1D3vec val;
    openI1D3spec::xyY(val[0], val[1], val[2]);
    return val;
}


int
openI1D3spec::clip (openI1D3spec &that)
{
    for(int c(0); c < OPEN_I1D3_NO_SPEC_ELEMS; c++)
    {
        if(_data[c] < that._data[c]) _data[c] = that._data[c];
    }
    
    return 1;
}


int
openI1D3spec::clip (double val)
{
    for(int c(0); c < OPEN_I1D3_NO_SPEC_ELEMS; c++)
    {
        if(_data[c] < val) _data[c] = val;
    }
    
    return 1;
}


double
openI1D3spec::sum() const
{
    double sum(0);

    for(int c(0); c < OPEN_I1D3_NO_SPEC_ELEMS; c++)
    {
        sum += _data[c];
    }
    
    return sum;
}


double
openI1D3spec::ave() const
{
    double sum = openI1D3spec::sum();
    
    return sum / (double)OPEN_I1D3_NO_SPEC_ELEMS;
}


double
openI1D3spec::maxVal() const
{
    double max(0);

    for(int c(0); c < OPEN_I1D3_NO_SPEC_ELEMS; c++)
    {
        if(max < _data[c]) max = _data[c];
    }
    
    return max;
}


openI1D3spec
openI1D3spec::operator=(double that)
{
    for(int c(0); c < OPEN_I1D3_NO_SPEC_ELEMS; c++) _data[c] = that;
    
    return *this;
}
    

openI1D3spec
openI1D3spec::operator+(double that)
{
    openI1D3spec tmp;
    
    for(int c(0); c < OPEN_I1D3_NO_SPEC_ELEMS; c++) tmp._data[c] = _data[c] + that;
    
    return tmp;
}
    

openI1D3spec
openI1D3spec::operator-(double that)
{
    openI1D3spec tmp;
    
    for(int c(0); c < OPEN_I1D3_NO_SPEC_ELEMS; c++) tmp._data[c] = _data[c] - that;
    
    return tmp;
}
    

openI1D3spec
openI1D3spec::operator*(double that)
{
    openI1D3spec tmp;
    
    for(int c(0); c < OPEN_I1D3_NO_SPEC_ELEMS; c++) tmp._data[c] = _data[c] * that;
    
    return tmp;
}
    

openI1D3spec
openI1D3spec::operator/(double that)
{
    openI1D3spec tmp;
    
    for(int c(0); c < OPEN_I1D3_NO_SPEC_ELEMS; c++) tmp._data[c] = _data[c] / that;
#ifdef _WIN32
    for(int c(0); c < OPEN_I1D3_NO_SPEC_ELEMS; c++) if(_isnan(tmp._data[c])) tmp._data[c] = 0.0;
#else
    for(int c(0); c < OPEN_I1D3_NO_SPEC_ELEMS; c++) if(isnan(tmp._data[c])) tmp._data[c] = 0.0;
#endif
    
    return tmp;
}


openI1D3spec
openI1D3spec::operator+=(double that)
{
    for(int c(0); c < OPEN_I1D3_NO_SPEC_ELEMS; c++) _data[c] += that;
    
    return *this;
}
    

openI1D3spec
openI1D3spec::operator-=(double that)
{
    for(int c(0); c < OPEN_I1D3_NO_SPEC_ELEMS; c++) _data[c] -= that;
    
    return *this;
}
    

openI1D3spec
openI1D3spec::operator*=(double that)
{
    for(int c(0); c < OPEN_I1D3_NO_SPEC_ELEMS; c++) _data[c] *= that;
    
    return *this;
}
    

openI1D3spec
openI1D3spec::operator/=(double that)
{
    for(int c(0); c < OPEN_I1D3_NO_SPEC_ELEMS; c++) _data[c] /= that;
#ifdef _WIN32
    for(int c(0); c < OPEN_I1D3_NO_SPEC_ELEMS; c++) if(_isnan(_data[c])) _data[c] = 0.0;
#else
    for(int c(0); c < OPEN_I1D3_NO_SPEC_ELEMS; c++) if(isnan(_data[c])) _data[c] = 0.0;
#endif
    
    return *this;
}


const double&
openI1D3spec::operator[](const unsigned int idx) const
{
    return _data[idx];
}


double&
openI1D3spec::operator[](const unsigned int idx)
{
    return _data[idx];
}


openI1D3spec
openI1D3spec::operator=(openI1D3spec that)
{
    for(int c(0); c < OPEN_I1D3_NO_SPEC_ELEMS; c++) _data[c] = that._data[c];
    
    return *this;
}
    

openI1D3spec
openI1D3spec::operator+(openI1D3spec that)
{
    openI1D3spec tmp;
    
    for(int c(0); c < OPEN_I1D3_NO_SPEC_ELEMS; c++) tmp._data[c] = _data[c] + that._data[c];
    
    return tmp;
}
    

openI1D3spec
openI1D3spec::operator-(openI1D3spec that)
{
    openI1D3spec tmp;
    
    for(int c(0); c < OPEN_I1D3_NO_SPEC_ELEMS; c++) tmp._data[c] = _data[c] - that._data[c];
    
    return tmp;
}
    

openI1D3spec
openI1D3spec::operator*(openI1D3spec that)
{
    openI1D3spec tmp;
    
    for(int c(0); c < OPEN_I1D3_NO_SPEC_ELEMS; c++) tmp._data[c] = _data[c] * that._data[c];
    
    return tmp;
}
    

openI1D3spec
openI1D3spec::operator/(openI1D3spec that)
{
    openI1D3spec tmp;
    
    for(int c(0); c < OPEN_I1D3_NO_SPEC_ELEMS; c++) tmp._data[c] = _data[c] / that._data[c];

#ifdef _WIN32
    for(int c(0); c < OPEN_I1D3_NO_SPEC_ELEMS; c++) if(_isnan(tmp._data[c])) tmp._data[c] = 0.0;
#else
    for(int c(0); c < OPEN_I1D3_NO_SPEC_ELEMS; c++) if(isnan(tmp._data[c])) tmp._data[c] = 0.0;
#endif

    
    return tmp;
}


openI1D3spec
openI1D3spec::operator+=(openI1D3spec that)
{
    for(int c(0); c < OPEN_I1D3_NO_SPEC_ELEMS; c++) _data[c] += that._data[c];
    
    return *this;
}
    

openI1D3spec
openI1D3spec::operator-=(openI1D3spec that)
{
    for(int c(0); c < OPEN_I1D3_NO_SPEC_ELEMS; c++) _data[c] -= that._data[c];
    
    return *this;
}
    

openI1D3spec
openI1D3spec::operator*=(openI1D3spec that)
{
    for(int c(0); c < OPEN_I1D3_NO_SPEC_ELEMS; c++) _data[c] *= that._data[c];
    
    return *this;
}
    

openI1D3spec
openI1D3spec::operator/=(openI1D3spec that)
{
    for(int c(0); c < OPEN_I1D3_NO_SPEC_ELEMS; c++) _data[c] /= that._data[c];
 #ifdef _WIN32
    for(int c(0); c < OPEN_I1D3_NO_SPEC_ELEMS; c++) if(_isnan(_data[c])) _data[c] = 0.0;
#else
    for(int c(0); c < OPEN_I1D3_NO_SPEC_ELEMS; c++) if(isnan(_data[c])) _data[c] = 0.0;
#endif
   
    return *this;
}


std::ostream&
operator<< (std::ostream &os, openI1D3spec  openI1D3spec)
{
    for(int c(0); c < OPEN_I1D3_NO_SPEC_ELEMS; c++)
    {
        os << (c * 2) + 380 << "    " << openI1D3spec._data[c] << std::endl;
    }
       
    return os;
}


std::istream&
operator>> (std::istream &is, openI1D3spec &openI1D3spec)
{
    int     wn(0);
    
    for(int c(0); c < OPEN_I1D3_NO_SPEC_ELEMS; c++)
    {
        is >> wn >> openI1D3spec._data[c];
    }
        
    return is;
}
    
