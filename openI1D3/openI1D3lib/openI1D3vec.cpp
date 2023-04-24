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


#include <math.h>
#include "openI1D3vec.h"

using namespace std;

openI1D3vec::openI1D3vec(double val)
{
    resize(3);
    openI1D3vec::operator=(val);
}


openI1D3vec::openI1D3vec(double v0, double v1, double v2)
{
    resize(3);
    _data[0] = v0;
    _data[1] = v1;
    _data[2] = v2;
}


openI1D3vec::openI1D3vec(const openI1D3vec& copy)
{
    resize(copy.size());
    openI1D3vec::operator=(copy);
}


openI1D3vec::~openI1D3vec()
{
}


void
openI1D3vec::resize(unsigned int val) 
{
    return _data.resize(val, 0.0);
}


unsigned int
openI1D3vec::size() const
{
    return (unsigned int)_data.size();
}


openI1D3vec&
openI1D3vec::operator= (const openI1D3vec& copy)
{
    for(unsigned int c(0); c < size(); ++c)
    {
        _data[c] = copy[c];
    }
    
    return *this;
}


openI1D3vec&
openI1D3vec::operator= (const double& copy)
{
    for(unsigned int c(0); c < size(); ++c)
    {
        _data[c] = copy;
    }
    
    return *this;
}


const double&
openI1D3vec::operator[](const unsigned int idx) const
{
    return _data[idx];
}


double&
openI1D3vec::operator[](const unsigned int idx)
{
    return _data[idx];
}


openI1D3vec
openI1D3vec::operator- () const
{
    openI1D3vec tmp(*this);
    
    for(unsigned int c(0); c < size(); ++c)
    {
        tmp[c] *= -1.0;
    }
    
    return tmp;
}


int
openI1D3vec::operator==(const openI1D3vec& val) const
{
    for(unsigned int c(0); c < size(); ++c)
    {
        if(_data[c] != val[c]) return 0;
    }
    
    return 1;
}


int
openI1D3vec::operator!=(const openI1D3vec& val) const
{
    return !openI1D3vec::operator==(val);
}


openI1D3vec
openI1D3vec::operator+(const double val) const
{
    openI1D3vec tmp(*this);
    
    for(unsigned int c(0); c < size(); ++c)
    {
        tmp[c] += val;
    }
    
    return tmp;
}


openI1D3vec
openI1D3vec::operator+(const openI1D3vec& val) const
{
    openI1D3vec tmp(*this);
    
    for(unsigned int c(0); c < size(); ++c)
    {
        tmp[c] += val[c];
    }
    
    return tmp;
}


openI1D3vec&
openI1D3vec::operator+=(const double val)
{
    for(unsigned int c(0); c < size(); ++c)
    {
        _data[c] += val;
    }
    
    return *this;
}


openI1D3vec&
openI1D3vec::operator+=(const openI1D3vec& val)
{
    openI1D3vec tmp(*this);
    
    for(unsigned int c(0); c < size(); ++c)
    {
        _data[c] += val[c];
    }
    
    return *this;
}


openI1D3vec
openI1D3vec::operator-(const double val) const
{
    openI1D3vec tmp(*this);
    
    for(unsigned int c(0); c < size(); ++c)
    {
        tmp[c] -= val;
    }
    
    return tmp;
}


openI1D3vec
openI1D3vec::operator-(const openI1D3vec& val) const
{
    openI1D3vec tmp(*this);
    
    for(unsigned int c(0); c < size(); ++c)
    {
        tmp[c] -= val[c];
    }
    
    return tmp;
}


openI1D3vec&
openI1D3vec::operator-=(const double val)
{
    for(unsigned int c(0); c < size(); ++c)
    {
        _data[c] -= val;
    }
    
    return *this;
}


openI1D3vec&
openI1D3vec::operator-=(const openI1D3vec& val)
{
    openI1D3vec tmp(*this);
    
    for(unsigned int c(0); c < size(); ++c)
    {
        _data[c] -= val[c];
    }
    
    return *this;
}


openI1D3vec
openI1D3vec::operator*(const double val) const
{
    openI1D3vec tmp(*this);
    
    for(unsigned int c(0); c < size(); ++c)
    {
        tmp[c] *= val;
    }
    
    return tmp;
}


openI1D3vec
openI1D3vec::operator*(const openI1D3vec& val) const
{
    openI1D3vec tmp(*this);
    
    for(unsigned int c(0); c < size(); ++c)
    {
        tmp[c] *= val[c];
    }
    
    return tmp;
}


openI1D3vec&
openI1D3vec::operator*=(const double val)
{
    for(unsigned int c(0); c < size(); ++c)
    {
        _data[c] *= val;
    }
    
    return *this;
}


openI1D3vec&
openI1D3vec::operator*=(const openI1D3vec& val)
{
    openI1D3vec tmp(*this);
    
    for(unsigned int c(0); c < size(); ++c)
    {
        _data[c] *= val[c];
    }
    
    return *this;
}


openI1D3vec
openI1D3vec::operator/(const double val) const
{
    openI1D3vec tmp(*this);
    
    for(unsigned int c(0); c < size(); ++c)
    {
        tmp[c] /= val;
    }
    
    return tmp;
}


openI1D3vec
openI1D3vec::operator/(const openI1D3vec& val) const
{
    openI1D3vec tmp(*this);
    
    for(unsigned int c(0); c < size(); ++c)
    {
        tmp[c] /= val[c];
    }
    
    return tmp;
}


openI1D3vec&
openI1D3vec::operator/=(const double val)
{
    for(unsigned int c(0); c < size(); ++c)
    {
        _data[c] /= val;
    }
    
    return *this;
}


openI1D3vec&
openI1D3vec::operator/=(const openI1D3vec& val)
{
    openI1D3vec tmp(*this);
    
    for(unsigned int c(0); c < size(); ++c)
    {
        _data[c] /= val[c];
    }
    
    return *this;
}


double
openI1D3vec::magnitude() const
{
    double sumSq(0);
    
    for(unsigned int c(0); c < size(); ++c)
    {
        sumSq += _data[c] * _data[c];
    }
    
    return sqrt(sumSq);
}


void
openI1D3vec::magnitude(double mag)
{
    normalise();
    
    for(unsigned int c(0); c < size(); ++c)
    {
        _data[c] *= mag;
    }
}


double
openI1D3vec::normalise()
{
    double mag = magnitude();
    
    for(unsigned int c(0); c < size(); ++c)
    {
        _data[c] /= mag;
    }
    
    return mag;
}


openI1D3vec
openI1D3vec::unit()
{
    openI1D3vec unit(*this);
    
    unit.normalise();
    
    return unit;
}


double
openI1D3vec::dot(const openI1D3vec& val) const
{
    double sum(0);
    
    for(unsigned int c(0); c < size(); ++c)
    {
        sum += _data[c] * val[c];
    }
    
    return sum;
}


openI1D3vec
openI1D3vec::cross(const openI1D3vec& val)  const
{
    openI1D3vec ret(0.0);

    if((_data.size() != 3) || (val.size() != 3)) return ret;
    
    ret[0] = _data[1] * val[2] - _data[2] * val[1];
    ret[1] = _data[2] * val[0] - _data[0] * val[2];
    ret[2] = _data[0] * val[1] - _data[1] * val[0];
    
    return ret;
}


std::ostream&
operator<< (std::ostream &os, openI1D3vec val)
{
    os << "(";
    for(unsigned int c(0); c < (val.size() - 1); ++c)
    {
        os << val[c] << ",";
    }
    os << val[val.size() - 1] << ")";
    
    return os;
}
