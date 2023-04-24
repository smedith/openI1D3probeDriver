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

#include "openI1D3mtx.h"

using namespace std;

openI1D3mtx::openI1D3mtx()
{
    resize(3, 3);
    identity();
}


openI1D3mtx::openI1D3mtx(const openI1D3mtx& copy)
{
    openI1D3mtx::operator=(copy);
}


openI1D3mtx::~openI1D3mtx()
{
}


void
openI1D3mtx::resize(unsigned int row, unsigned int col) 
{
    openI1D3vec zero;
    zero.resize(col);
    _data.resize(row, zero);

    for(unsigned int c(0); c < row; ++c)
    {
        _data[c].resize(col);
    }
}


unsigned int
openI1D3mtx::size() const
{
    return (unsigned int)_data.size();
}


openI1D3mtx&
openI1D3mtx::operator= (const openI1D3mtx& copy)
{
    resize(copy.sizeR(), copy.sizeC());

    for(unsigned int c(0); c < size(); ++c)
    {
        _data[c] = copy[c];
    }
    
    return *this;
}


openI1D3mtx&
openI1D3mtx::operator= (const double& copy)
{
    for(unsigned int c(0); c < size(); ++c)
    {
        _data[c] = copy;
    }
    
    return *this;
}


const openI1D3vec&
openI1D3mtx::operator[](const unsigned int idx) const
{
    return _data[idx];
}


openI1D3vec&
openI1D3mtx::operator[](const unsigned int idx)
{
    return _data[idx];
}


openI1D3vec
openI1D3mtx::C(const unsigned int idx) const
{
    openI1D3vec tmp;
    tmp.resize(sizeC());

    for(unsigned int c(0); c < size(); ++c)
    {
        tmp[c] = _data[c][idx];
    }

    return tmp;
}


void
openI1D3mtx::C(const unsigned int idx, const openI1D3vec& val)
{
    for(unsigned int c(0); c < size(); ++c)
    {
        _data[c][idx] = val[c];
    }
}


int
openI1D3mtx::operator==(const openI1D3mtx& val) const
{
    for(unsigned int c(0); c < size(); ++c)
    {
        if(_data[c] != val[c]) return 0;
    }
    
    return 1;
}


int
openI1D3mtx::operator!=(const openI1D3mtx& val) const
{
    return !openI1D3mtx::operator==(val);
}


openI1D3mtx
openI1D3mtx::operator+(const double val) const
{
    openI1D3mtx tmp(*this);
    
    for(unsigned int c(0); c < size(); ++c)
    {
        tmp[c] += val;
    }
    
    return tmp;
}


openI1D3mtx
openI1D3mtx::operator+(const openI1D3mtx& val) const
{
    openI1D3mtx tmp(*this);
    
    for(unsigned int c(0); c < size(); ++c)
    {
        tmp[c] += val[c];
    }
    
    return tmp;
}


openI1D3mtx&
openI1D3mtx::operator+=(const double val)
{
    for(unsigned int c(0); c < size(); ++c)
    {
        _data[c] += val;
    }
    
    return *this;
}


openI1D3mtx&
openI1D3mtx::operator+=(const openI1D3mtx& val)
{
    openI1D3mtx tmp(*this);
    
    for(unsigned int c(0); c < size(); ++c)
    {
        _data[c] += val[c];
    }
    
    return *this;
}


openI1D3mtx
openI1D3mtx::operator-(const double val) const
{
    openI1D3mtx tmp(*this);
    
    for(unsigned int c(0); c < size(); ++c)
    {
        tmp[c] -= val;
    }
    
    return tmp;
}


openI1D3mtx
openI1D3mtx::operator-(const openI1D3mtx& val) const
{
    openI1D3mtx tmp(*this);
    
    for(unsigned int c(0); c < size(); ++c)
    {
        tmp[c] -= val[c];
    }
    
    return tmp;
}


openI1D3mtx&
openI1D3mtx::operator-=(const double val)
{
    for(unsigned int c(0); c < size(); ++c)
    {
        _data[c] -= val;
    }
    
    return *this;
}


openI1D3mtx&
openI1D3mtx::operator-=(const openI1D3mtx& val)
{
    openI1D3mtx tmp(*this);
    
    for(unsigned int c(0); c < size(); ++c)
    {
        _data[c] -= val[c];
    }
    
    return *this;
}


openI1D3mtx
openI1D3mtx::operator*(const double val) const
{
    openI1D3mtx tmp(*this);
    
    for(unsigned int c(0); c < size(); ++c)
    {
        tmp[c] *= val;
    }
    
    return tmp;
}


openI1D3mtx&
openI1D3mtx::operator*=(const double val)
{
    for(unsigned int c(0); c < size(); ++c)
    {
        _data[c] *= val;
    }
    
    return *this;
}

openI1D3mtx
openI1D3mtx::operator/(const double val) const
{
    openI1D3mtx tmp(*this);
    
    for(unsigned int c(0); c < size(); ++c)
    {
        tmp[c] /= val;
    }
    
    return tmp;
}


openI1D3mtx&
openI1D3mtx::operator/=(const double val)
{
    for(unsigned int c(0); c < size(); ++c)
    {
        _data[c] /= val;
    }
    
    return *this;
}


openI1D3vec
openI1D3mtx::operator* (const openI1D3vec& val) const
{
    openI1D3vec tmp;
    tmp.resize(sizeR());
    
    for(unsigned int r(0); r < sizeR(); ++r)
    {
        for(unsigned int c(0); c < sizeC(); ++c)
        {
            tmp[r] += _data[r][c] * val[c];
        }
    }
    
    return tmp;
}


openI1D3mtx
openI1D3mtx::operator* (const openI1D3mtx& val) const
{
    openI1D3mtx tmp;
    tmp.resize(sizeR(), val.sizeC());
    
    for(unsigned int r(0); r < sizeR(); ++r)
    {
        for(unsigned int c(0); c < val.sizeC(); ++c)
        {
            tmp[r][c] = 0;
            for(unsigned int s(0); s < sizeC(); ++s)
            {
                tmp[r][c] += _data[r][s] * val[s][c];
            }
        }
    }
    
    return tmp;
}


openI1D3mtx&
openI1D3mtx::operator*=(const openI1D3mtx& val)
{
    openI1D3mtx tmp;
    tmp.resize(sizeR(), val.sizeC());
    
    for(unsigned int r(0); r < sizeR(); ++r)
    {
        for(unsigned int c(0); c < val.sizeC(); ++c)
        {
            tmp[r][c] = 0;
            for(unsigned int s(0); s < sizeC(); ++s)
            {
                tmp[r][c] += _data[r][s] * val[s][c];
            }
        }
    }
    
    *this = tmp;
    
    return *this;
}


void
openI1D3mtx::transpose()
{
    openI1D3mtx tmp;
    tmp.resize(sizeC(), sizeR());
    
    for(unsigned int r(0); r < sizeR(); ++r)
    {
        for(unsigned int c(0); c < sizeC(); ++c)
        {
            tmp[c][r] = _data[r][c];
        }
    }
    
    *this = tmp;
}


void
openI1D3mtx::identity()
{
    for(unsigned int r(0); r < sizeR(); ++r)
    {
        for(unsigned int c(0); c < sizeC(); ++c)
        {
            _data[r][c] = 0.0;
            if(r == c) _data[r][c] = 1.0;
        }
    }
}


double
openI1D3mtx::determinant()
{
    double sum;
    
    sum  = _data[0][0] *_data[1][1] *_data[2][2];
    sum += _data[0][1] *_data[1][2] *_data[2][0];
    sum += _data[0][2] *_data[1][0] *_data[2][1];
    sum -= _data[0][2] *_data[1][1] *_data[2][0];
    sum -= _data[0][1] *_data[1][0] *_data[2][2];
    sum -= _data[0][0] *_data[1][2] *_data[2][1];
     
    return sum;
}


void
openI1D3mtx::adjoint()
{
    openI1D3mtx adj(*this);
    double sign(1.0);
    for(unsigned int r(0); r < sizeR(); ++r)
    {
        for(unsigned int c(0); c < sizeC(); ++c, sign *= -1.0)
        {
            double tmp[2][2];
            unsigned int rt(0);
            for(unsigned int rr(0); rr < sizeR(); ++rr, ++rt)
            {
                if(rr == r) rr++;
                if(rr > 2) continue;
                unsigned int ct(0);
                for(unsigned int cc(0); cc < sizeC(); ++cc,++ct)
                {
                    if(cc == c) cc++;
                    if(cc > 2) continue;
                    tmp[rt][ct] = _data[rr][cc];
                }
            }
            
            adj[r][c] = ((tmp[0][0] * tmp[1][1]) - (tmp[0][1] * tmp[1][0])) * sign;
        }
    }
    
    adj.transpose();
    
    *this = adj;
}


bool
openI1D3mtx::invert()
{
    double det = determinant();
    if(det == 0) return false;
    openI1D3mtx inv(*this);
    inv.adjoint();
    inv /= det;
    *this = inv;
    return true;
}


std::ostream&
operator<< (std::ostream &os, openI1D3mtx val)
{
    os << "(";
    for(unsigned int c(0); c < (val.size() - 1); ++c)
    {
        os << val[c] << ",";
    }
    os << val[val.size()] << ")";
    
    return os;
}
