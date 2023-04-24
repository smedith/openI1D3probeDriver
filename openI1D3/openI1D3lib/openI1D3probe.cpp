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


#define _CRT_SECURE_NO_WARNINGS 

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <setupapi.h>
#include <math.h>

#include "openI1D3hidFuncs.h"
#include "openI1D3spec.h"
#include "openI1D3probe.h"
#include "openI1D3mtx.h"


#define I1D3_CLK_FEQ 12000000.0 // 12 MHz

using namespace std;


void createUnLockResponse(int key0, int key1, unsigned char iBuf[64], unsigned char oBuf[64]);

openI1D3probe::openI1D3probe() :_hidDev(0), _blackOffSetR(0), _blackOffSetG(0), _blackOffSetB(0), _isRevB(false)
{
	memset(_errMsg, 0x00, 256);
}

openI1D3probe::~openI1D3probe()
{
	if(_hidDev)
	{
		openI1D3closeHIDdevice(_hidDev);
		delete _hidDev;
	}
}


bool
openI1D3probe::initConnection(unsigned int* productID)
{
 	if(openI1D3loadDLLfuncs() == 0)	// load the DLL functions
	{
		sprintf(_errMsg, "Failed to load DLL functions.");
		return false;
	}

	_hidDev = openI1D3findHIDdevice();
	if(!_hidDev)
	{
		sprintf(_errMsg, "Failed to find i1d3 HID device.");
		return false;
	}

	if(!openI1D3openHIDdevice(_hidDev))
	{
		sprintf(_errMsg, "Failed to open i1d3 HID device.");
		return false;
	}

	int id = openI1D3getHIDproductID(_hidDev);

	if(productID) *productID = id;

	return true;
}


bool
openI1D3probe::initCalibration(const openI1D3calMtx* calMtx, bool diffuserActive)
{
	_calMtx.identity();

	if(calMtx)
	{
		// use a supplied calibration matrix, usually from an edr file
		_calMtx = *calMtx;
	}
	else
	{
		// default hardware calibration
		openI1D3spec calSpecR(_calibrationR, 380, 351);			// calibration spetra is stored as W per nM per count
		openI1D3spec calSpecG(_calibrationG, 380, 351);			// raw calibration specta is 380 - 730nm
		openI1D3spec calSpecB(_calibrationB, 380, 351);			// convert to standard 380 - 830nm

		if(diffuserActive)
		{
			// if the diffuser is active, then compensate for the diffuser spectral charactoristics
			openI1D3spec calSpecA(_calibrationA, 380, 351);
			calSpecR *= calSpecA;
			calSpecG *= calSpecA;
			calSpecB *= calSpecA;
		}

		openI1D3mtx rgb;
		rgb.C(0, calSpecR.conv(calSpecR, calSpecG, calSpecB));	// the responce spectra are for each individual sensor
		rgb.C(1, calSpecG.conv(calSpecR, calSpecG, calSpecB));	// any light entering the probe will fall on ALL 3 sensors
		rgb.C(2, calSpecB.conv(calSpecR, calSpecG, calSpecB));	// we need to account for how the sensors are affected all light 
 		if(!rgb.invert())
		{
			sprintf(_errMsg, "Failed generate default calibration matrix.");
			return false;
		}

		calSpecR *= 683.002;	// Convert from W to Lm (Lumens, theoretical monochromatic maximum) 
		calSpecG *= 683.002;	
		calSpecB *= 683.002;	

		openI1D3mtx xyz;
		xyz.C(0, calSpecR.xyz());
		xyz.C(1, calSpecG.xyz());
		xyz.C(2, calSpecB.xyz());

		openI1D3mtx cMtx;
		cMtx = xyz * rgb;
	
		// _calMtx now converst from sensor counts to Cdm-2
		//
		// Cd.m-2 = Lm /(sr.m2), definition of Candela per square meter in terms of Lumens
		// assume perfect, ideal point light source, emmiting equally over the surface of a sphere 4/3pi stradiens (st)
		// Cd.m-2 = Lm.m-2
		for(unsigned int r(0); r <3; ++r)
		{
			for(unsigned int c(0); c < 3; ++c)
			{
				_calMtx[r][c] = cMtx[r][c];
			}
		}
	}

	return true;
}


bool
openI1D3probe::calMtxFromEDRdata(const unsigned char* data, unsigned long len, openI1D3calMtx& calMtx, bool diffuserActive) const
{
	vector<openI1D3spec> edrSpecs;

	if((len < 9) || (strncmp("EDR DATA1", (const char*)data, 9) != 0))
	{
		sprintf(_errMsg, "Not a valid edr file.");
		return false;
	}

	unsigned int numDataSets = (data[0x0165] << 8) + (data[0x0164] << 0);

	if(numDataSets < 3)
	{
		sprintf(_errMsg, "Less than 3 data sets in the edr file.");
		return false;
	}

	unsigned short spectralDataFlg = (data[0x022F] << 8) + (data[0x022E] << 0);

	if(spectralDataFlg != 1)
	{
		sprintf(_errMsg, "No spectral samples in the edr file.");
		return false;
	}

	double stNm = *((double*)(&data[0x0230]));
	double enNm = *((double*)(&data[0x0238]));
	double spNm = *((double*)(&data[0x0240]));
	
	unsigned char* sData = (unsigned char*) data;
	sData += 600; // the spectral data sets sart at + 600;

	for(unsigned int sCnt(0); sCnt < numDataSets; ++sCnt)
	{
		if((strncmp("DISPLAY DATA",  (const char*)&sData[0x00], 12) != 0))
		{
			sprintf(_errMsg, "No display data in the edr file.");
			return false;
		}

		if((strncmp("SPECTRAL DATA", (const char*)&sData[0x80], 13) != 0))
		{
			sprintf(_errMsg, "No spectral data in the edr file.");
			return false;
		}

		unsigned int num = *((unsigned int*)(&sData[0x90]));

		openI1D3spec newSpec;

		for(unsigned int eCnt(0); eCnt < num; ++eCnt)
		{
			unsigned int loc = eCnt + ((unsigned int)stNm - OPEN_I1D3_FIRST_SPEC_ELEM);

			double val(0);
			val = *((double*)(&sData[0x9c + (8 * eCnt)]));

			if(loc < OPEN_I1D3_NO_SPEC_ELEMS) newSpec[loc] = val;
		}
		
		edrSpecs.push_back(newSpec);

		sData += 0x9c + (8 * num);
	}

	if(sData < (data + len))
	{
		if((strncmp("CORRECTION DATA",  (const char*)&sData[0x00], 15) != 0))
		{
			sprintf(_errMsg, "No correction data in the edr file.");
			return false;
		}

		unsigned int num = *((unsigned int*)(&sData[0x50]));

		openI1D3spec corSpec;

		for(unsigned int eCnt(0); eCnt < num; ++eCnt)
		{
			unsigned int loc = eCnt + ((unsigned int)stNm - OPEN_I1D3_FIRST_SPEC_ELEM);

			double val(0);
			val = *((double*)(&sData[0x5c + (8 * eCnt)]));

			if(loc < OPEN_I1D3_NO_SPEC_ELEMS) corSpec[loc] = val;
		}

		sData += 0x5c + (8 * num);

		for(unsigned int sCnt(0); sCnt < edrSpecs.size(); ++sCnt)
		{
			edrSpecs[sCnt] *= corSpec;
		}
	}

	// default hardware calibration
	openI1D3spec calSpecR(_calibrationR, 380, 351);			// calibration spetra is stored as W per nM per count
	openI1D3spec calSpecG(_calibrationG, 380, 351);			// raw calibration specta is 380 - 730nm
	openI1D3spec calSpecB(_calibrationB, 380, 351);			// convert to standard 380 - 830nm

	if(diffuserActive)
	{
		// if the diffuser is active, then compensate for the diffuser spectral charactoristics
		openI1D3spec calSpecA(_calibrationA, 380, 351);
		calSpecR *= calSpecA;
		calSpecG *= calSpecA;
		calSpecB *= calSpecA;
	}

	openI1D3mtx xyzs;
	xyzs.resize(3, (unsigned int)edrSpecs.size());
	openI1D3mtx rgbs;
	rgbs.resize(3, (unsigned int)edrSpecs.size());

	for(unsigned int sCnt(0); sCnt < edrSpecs.size(); ++sCnt)
	{
		rgbs.C(sCnt, edrSpecs[sCnt].conv(calSpecR, calSpecG, calSpecB));
		edrSpecs[sCnt] *= 683.002;	// Convert from W to Lm (Lumens, theoretical monochromatic maximum) 
		xyzs.C(sCnt, edrSpecs[sCnt].xyz());
	}

	openI1D3mtx xyzT(xyzs);
	xyzT.transpose();
	openI1D3mtx rgbT(rgbs);
	rgbT.transpose();

	openI1D3mtx xyz;
	xyz = xyzs * rgbT;
	openI1D3mtx rgb;
	rgb = rgbs * rgbT;

	if(!rgb.invert())
	{
		sprintf(_errMsg, "Failed to generate a calibration matrix.");
		return false;
	}

	openI1D3mtx cMtx;
	cMtx = xyz * rgb;

    for(unsigned int r(0); r <3; ++r)
    {
		for(unsigned int c(0); c < 3; ++c)
		{
			calMtx[r][c] = cMtx[r][c];
		}
	}

	return true;
}


bool
openI1D3probe::calMtxFromSpectraldata(double data[][401], unsigned long num, openI1D3calMtx& calMtx, bool diffuserActive) const
{
	vector<openI1D3spec> edrSpecs;

	if(num < 3)
	{
		sprintf(_errMsg, "Less than 3 spectral data sets.");
		return false;
	}

	for(unsigned int sc(0); sc < num; ++sc)
	{
 		openI1D3spec newSpec;

		for(unsigned int eCnt(0); eCnt < 401; ++eCnt)
		{
			newSpec[eCnt] = data[sc][eCnt];
		}
		
		edrSpecs.push_back(newSpec);
	}

	// default hardware calibration
	openI1D3spec calSpecR(_calibrationR, 380, 351);			// calibration spetra is stored as W per nM per count
	openI1D3spec calSpecG(_calibrationG, 380, 351);			// raw calibration specta is 380 - 730nm
	openI1D3spec calSpecB(_calibrationB, 380, 351);			// convert to standard 380 - 830nm

	if(diffuserActive)
	{
		// if the diffuser is active, then compensate for the diffuser spectral charactoristics
		openI1D3spec calSpecA(_calibrationA, 380, 351);
		calSpecR *= calSpecA;
		calSpecG *= calSpecA;
		calSpecB *= calSpecA;
	}

	openI1D3mtx xyzs;
	xyzs.resize(3, (unsigned int)edrSpecs.size());
	openI1D3mtx rgbs;
	rgbs.resize(3, (unsigned int)edrSpecs.size());

	for(unsigned int sCnt(0); sCnt < edrSpecs.size(); ++sCnt)
	{
		rgbs.C(sCnt, edrSpecs[sCnt].conv(calSpecR, calSpecG, calSpecB));
		edrSpecs[sCnt] *= 683.002;	// Convert from W to Lm (Lumens, theoretical monochromatic maximum) 
		xyzs.C(sCnt, edrSpecs[sCnt].xyz());
	}

	openI1D3mtx xyzT(xyzs);
	xyzT.transpose();
	openI1D3mtx rgbT(rgbs);
	rgbT.transpose();

	openI1D3mtx xyz;
	xyz = xyzs * rgbT;
	openI1D3mtx rgb;
	rgb = rgbs * rgbT;

	if(!rgb.invert())
	{
		sprintf(_errMsg, "Failed to generate a calibration matrix.");
		return false;
	}

	openI1D3mtx cMtx;
	cMtx = xyz * rgb;

    for(unsigned int r(0); r <3; ++r)
    {
		for(unsigned int c(0); c < 3; ++c)
		{
			calMtx[r][c] = cMtx[r][c];
		}
	}

	return true;
}


bool
openI1D3probe::unLock(unsigned int* typeID) const
{
	const unsigned int __i1d3numUnLockKeys(10);
	const unsigned int __i1d3UnLockKeys[][2] =
	 {
		{ 0xe9622e9f, 0x8d63e133 },
		{ 0xe01e6e0a, 0x257462de },
		{ 0xcaa62b2c, 0x30815b61 }, //oem
		{ 0xa9119479, 0x5b168761 },
		{ 0x160eb6ae, 0x14440e70 },
		{ 0x291e41d7, 0x51937bdd },
		{ 0x1abfae03, 0xf25ac8e8 },
		{ 0x828c43e9, 0xcbb8a8ed },
		{ 0xe8d1a980, 0xd146f7ad },
		{ 0xc9bfafe0, 0x02871166 } //c6
	};


	unsigned char tBuf[64];
	unsigned char fBuf[64];
	unsigned short cmd;

	for(unsigned int c(0); c < __i1d3numUnLockKeys; ++c)
	{
		memset(tBuf, 0, 64);
		memset(fBuf, 0, 64);

		// Send the challenge
		cmd = 0x9900;
		sendCommand(cmd, tBuf, fBuf);

		// Convert challenge to response
		createUnLockResponse((int)__i1d3UnLockKeys[c][0], (int)__i1d3UnLockKeys[c][1], fBuf, tBuf);

		// Send the response
		cmd = 0x9a00;
		sendCommand(cmd, tBuf, fBuf);

		if(fBuf[2] == 0x77)
		{
			// successfully unlocked the probe
			if(typeID) *typeID = c;
			return true;
		}
	}

	sprintf(_errMsg, "Failed to unlock the i1d3 probe.");
	return false;
}

void
openI1D3probe::probeTypeIDtoName(unsigned int id, char* probeTypeName)
{
	switch(id)
	{
		case 0:
		{
			sprintf(probeTypeName, "Display Pro");
			break;
		}

		case 1:
		{
			sprintf(probeTypeName, "ColorMunki");
			break;
		}

		case 2:
		{
			sprintf(probeTypeName, "Display Pro OEM");
			break;
		}

		case 3:
		{
			sprintf(probeTypeName, "NEC");
			break;
		}

		case 4:
		{
			sprintf(probeTypeName, "Quoto");
			break;
		}

		case 5:
		{
			sprintf(probeTypeName, "HP Dreamcolor");
			break;
		}

		case 6:
		{
			sprintf(probeTypeName, "Wacom");
			break;
		}

		case 7:
		{
			sprintf(probeTypeName, "TPA1");
			break;
		}

		case 8:
		{
			sprintf(probeTypeName, "Barco");
			break;
		}

		case 9:
		{
			sprintf(probeTypeName, "SpectraCal C6");
			break;
		}

		default:
		{
			sprintf(probeTypeName, "Not known");
		}
	}
}


bool
openI1D3probe::getInfo(char rBuf[64]) const
{
	unsigned char tBuf[64];
	unsigned char fBuf[64];
	unsigned short cmd;

	memset(tBuf, 0, 64);
	memset(fBuf, 0, 64);

	cmd = 0x0000;

	if(!sendCommand(cmd, tBuf, fBuf))
	{
		sprintf(_errMsg, "Failed to get i1d3 probe info.");
		return false;
	}
	
	strncpy((char *)rBuf, (char *)fBuf + 2, 62);

	return true;
}


bool
openI1D3probe::readInternalEeprom(unsigned char buf[256]) const
{
	unsigned char tBuf[64];
	unsigned char fBuf[64];
	unsigned short cmd;

	memset(tBuf, 0, 64);
	memset(fBuf, 0, 64);

	cmd = 0x0800;

	unsigned char* bPtr = buf;

	// read up into 60 byte packets
	unsigned short addr(0);
	for(int len(256), inc(0); len > 0; addr += inc, bPtr += inc, len -= inc)
	{
		inc = len;
		if(inc > 60) inc = 60;

		tBuf[1]	= (unsigned char)addr;
		tBuf[2] = (unsigned char)inc;

		if(!sendCommand(cmd, tBuf, fBuf))
		{
			sprintf(_errMsg, "Failed to read i1d3 internal eeprom.");
			return false;
		}
	
		memcpy(bPtr, fBuf + 4, inc);
	}

	return true;
}


bool
openI1D3probe::writeInternalEeprom(unsigned char buf[256]) const
{
	unsigned char tBuf[64];
	unsigned char fBuf[64];
	unsigned short cmd;

	memset(tBuf, 0, 64);
	memset(fBuf, 0, 64);

	cmd = 0x0700;

	unsigned char* bPtr = buf;

	// write up into 32 byte packets
	unsigned short addr(0);
	for(int len(256), inc(0); len > 0; addr += inc, bPtr += inc, len -= inc)
	{
		inc = len;
		if(inc > 32) inc = 32;

		tBuf[1]	= (unsigned char)addr;
		tBuf[2] = (unsigned char)inc;
	
		memcpy(tBuf + 3, bPtr, inc);

		if(!sendCommand(cmd, tBuf, fBuf))
		{
			sprintf(_errMsg, "Failed to wright i1d3 internal eeprom.");
			return false;
		}
	}

	return true;
}


bool
openI1D3probe::readExternalEeprom(unsigned char buf[8192]) const
{
	unsigned char tBuf[64];
	unsigned char fBuf[64];
	unsigned short cmd;

	memset(tBuf, 0, 64);
	memset(fBuf, 0, 64);

	cmd = 0x1200;

	unsigned char* bPtr = buf;

	// read up into 59 byte packets
	unsigned short addr(0);
	for(int len(8192), inc(0); len > 0; addr += inc, bPtr += inc, len -= inc)
	{
		inc = len;
		if(inc > 59) inc = 59;

		tBuf[1]	= (addr >> 8) & 0xff;
		tBuf[2] = addr & 0xff;
		tBuf[3] = (unsigned char)inc;

		if(!sendCommand(cmd, tBuf, fBuf))
		{
			sprintf(_errMsg, "Failed to read i1d3 external eeprom.");
			return false;
		}
	
		memcpy(bPtr, fBuf + 5, inc);
	}

	return true;
}


bool
openI1D3probe::writeExternalEeprom(unsigned char buf[8192]) const
{
	unsigned char tBuf[64];
	unsigned char fBuf[64];
	unsigned short cmd;

	memset(tBuf, 0, 64);
	memset(fBuf, 0, 64);

	cmd = 0x1300;

	unsigned char* bPtr = buf;

	// write up into 59 byte packets
	unsigned short addr(0);
	for(int len(8192), inc(0); len > 0; addr += inc, bPtr += inc, len -= inc)
	{
		inc = len;
		if(inc > 32) inc = 32;

		tBuf[1]	= (addr >> 8) & 0xff;
		tBuf[2] = addr & 0xff;
		tBuf[3] = (unsigned char)inc;

		memcpy(tBuf + 4, bPtr, inc);
	
		if(!sendCommand(cmd, tBuf, fBuf))
		{
			sprintf(_errMsg, "Failed to write i1d3 external eeprom.");
			return false;
		}
	}

	return true;
}


void openI1D3probe::initFromInternalEeprom(unsigned char buf[256])
{
	memset(_serNum, 0x0, 21);
	memset(_revNum, 0x0, 11);

	memcpy(_serNum, buf + 0x10, 20);
	memcpy(_revNum, buf + 0x2C, 10);

	_blackOffSetR = (buf[4]  << 0) | (buf[5]  << 8) |(buf[6]  << 16) | (buf[7]  << 24);
	_blackOffSetG = (buf[8]  << 0) | (buf[9]  << 8) |(buf[10] << 16) | (buf[11] << 24);
	_blackOffSetB = (buf[12] << 0) | (buf[13] << 8) |(buf[14] << 16) | (buf[15] << 24);

	if(_blackOffSetR == 0xffffffff) _blackOffSetR = 0;
	if(_blackOffSetG == 0xffffffff) _blackOffSetG = 0;
	if(_blackOffSetB == 0xffffffff) _blackOffSetB = 0;

	if(strncmp(_revNum, "B-02", 4) == 0) _isRevB = true;
}


void openI1D3probe::initFromExternalEeprom(unsigned char buf[8192])
{
	// extract the raw calibration spectra
	//
	// they are Hz per W/nm, 1nm spacing
	// start 380nm to 730nm inclusive, 351 samples. index 0-350

	float tFlt;
	unsigned int* uiPtr;
	uiPtr = (unsigned int*)(&tFlt);

	for(int n(0); n < 351; ++n)
	{
		unsigned int locX = 0x0026 + (n * 4) + (0 * 351);
		unsigned int locY = 0x0026 + (n * 4) + (4 * 351);
		unsigned int locZ = 0x0026 + (n * 4) + (8 * 351);
		unsigned int locA = 0x10bc + (n * 4);

		// may need to byte swap
		*uiPtr = (buf[locX + 0]  << 0) | (buf[locX + 1]  << 8) |(buf[locX + 2]  << 16) | (buf[locX + 3]  << 24);
		_calibrationR[n] = (double)tFlt;

		*uiPtr = (buf[locY + 0]  << 0) | (buf[locY + 1]  << 8) |(buf[locY + 2]  << 16) | (buf[locY + 3]  << 24);
		_calibrationG[n] = (double)tFlt;

		*uiPtr = (buf[locZ + 0]  << 0) | (buf[locZ + 1]  << 8) |(buf[locZ + 2]  << 16) | (buf[locZ + 3]  << 24);
		_calibrationB[n] = (double)tFlt;

		*uiPtr = (buf[locA + 0]  << 0) | (buf[locA + 1]  << 8) |(buf[locA + 2]  << 16) | (buf[locA + 3]  << 24);
		_calibrationA[n] = (double)tFlt;
	}
}


void
openI1D3probe::getInternalEepromBufferSerialNumber(unsigned char buf[256], char* serNum) const
{
	memset(serNum, 0x00, 21);
	memcpy(serNum, &buf[16], 20);
}


void
openI1D3probe::setInternalEepromBufferSerialNumber(unsigned char buf[256], char* serNum) const
{
	memcpy(&buf[16], serNum, 20);
}


unsigned int
openI1D3probe::calExternalEepromChkksum(unsigned char buf[8192], bool alt) const
{
	unsigned int sum(0);
	unsigned int sz(0x178e);  // rev2
	if(alt) sz = 0x179a;	  // rev1

	for(unsigned int i(4); i < sz; i++)
	{
		sum += buf[i];
	}
	sum &= 0xffff;

	return sum;
}


void
openI1D3probe::setExternalEepromChkksum(unsigned char buf[8192], unsigned int sum) const
{
	buf[2] = (unsigned char)(sum >> 0) & 0xff;
	buf[3] = (unsigned char)(sum >> 8) & 0xff;
}


unsigned int
openI1D3probe::getExternalEepromChkksum(unsigned char buf[8192]) const
{
	unsigned int sum(0);

	sum =  buf[2] | (buf[3] << 8);

	return sum;
}


bool
openI1D3probe::enableEepromWriteAccess() const
{
	unsigned char tBuf[64];
	unsigned char fBuf[64];
	unsigned short cmd;


	memset(tBuf, 0, 64);
	memset(fBuf, 0, 64);

	// Send the challenge
	cmd = 0xab00;

	tBuf[1]	= 0xa3;
	tBuf[2]	= 0x80;
	tBuf[3]	= 0x25;
	tBuf[4]	= 0x41;

	if(!sendCommand(cmd, tBuf, fBuf))
	{
		sprintf(_errMsg, "Failed to enable i1d3 write access.");
		return false;
	}

	return true;
}


bool
openI1D3probe::measure(openI1D3probe::measurementMode mode, double integrationTime, double& X, double& Y, double& Z, bool rawRGBflg) const
{
	if(integrationTime > 6.0) integrationTime = 6.0; // limit to a maximum integration time of 6 seconds

	double Rcnt(0), Gcnt(0), Bcnt(0);

	X = 0;
	Y = 0;
	Z = 0;

	if(mode == openI1D3probe::frequency)
	{
		unsigned int RrawCnt(0), GrawCnt(0), BrawCnt(0);
		if(!measureOverIntegrationTime(integrationTime, Rcnt, Gcnt, Bcnt, &RrawCnt, &GrawCnt, &BrawCnt)) return false;
		// if the raw count of ANY channel is under 100, then double the exposure time and try again
		integrationTime *= 2.0;
		if(integrationTime > 6.0) integrationTime = 6.0;
		if((RrawCnt < 100) || (GrawCnt < 100) || (BrawCnt < 100))
		{
			if(!measureOverIntegrationTime(integrationTime, Rcnt, Gcnt, Bcnt, &RrawCnt, &GrawCnt, &BrawCnt)) return false;
		}
	}
	else if(mode == openI1D3probe::period)
	{
		unsigned short rc(2), gc(2), bc(2);
		// initial period measurement
		if(!measureOverSignalCount(rc, gc, bc, Rcnt, Gcnt, Bcnt)) return false;

		// Rcnt, Gcnt, Bcnt are in signal counts per second
		// so calculate the desired count
		Rcnt *= integrationTime;
		Gcnt *= integrationTime;
		Bcnt *= integrationTime;

		if(Rcnt < 2.0) Rcnt = 2.0;
		if(Gcnt < 2.0) Gcnt = 2.0;
		if(Bcnt < 2.0) Bcnt = 2.0;
		if(Rcnt > 65534.0) Rcnt = 65535.0;
		if(Gcnt > 65534.0) Gcnt = 65535.0;
		if(Bcnt > 65534.0) Bcnt = 65535.0;

		rc = (unsigned short) Rcnt;
		gc = (unsigned short) Gcnt;
		bc = (unsigned short) Bcnt;

		// actual period measurement
		if(!measureOverSignalCount(rc, gc, bc, Rcnt, Gcnt, Bcnt)) return false;
	}
	else if(mode == openI1D3probe::aio)
	{
		unsigned int RrawCnt(0), GrawCnt(0), BrawCnt(0);
		if(!measureAIO(integrationTime, Rcnt, Gcnt, Bcnt, &RrawCnt, &GrawCnt, &BrawCnt)) return false;

		if((RrawCnt < 2) || (GrawCnt < 2) || (BrawCnt < 2))
		{
			// one or more mwaesurements were too low
			// so try again with double the integration tim
			integrationTime *= 2.0;
			if(integrationTime > 6.0) integrationTime = 6.0;
			if(!measureAIO(integrationTime, Rcnt, Gcnt, Bcnt, &RrawCnt, &GrawCnt, &BrawCnt)) return false;
			
			//if the raw counts are ALL 1, then the reading is bad
			if(!((RrawCnt < 2) && (GrawCnt < 2) && (BrawCnt < 2)))
			{
				// if any count is 1, remeasure using period
				if((RrawCnt < 2) || (GrawCnt < 2) || (BrawCnt < 2))
				{
					double Rcnt2(0), Gcnt2(0), Bcnt2(0);
					if(!measure(measurementMode::period, integrationTime, Rcnt2, Gcnt2, Bcnt2, true)) return false;
					// replace the low count with the frequency count
					if(RrawCnt < 2) Rcnt = Rcnt2;
					if(GrawCnt < 2)	Gcnt = Gcnt2;
					if(BrawCnt < 2)	Bcnt = Bcnt2;
				}
			}
			else
			{
				Rcnt = 0;
				Gcnt = 0;
				Bcnt = 0;
			}
		}
	}

	if(rawRGBflg)
	{
		X = Rcnt;
		Y =	Gcnt;
		Z =	Bcnt;
	}
	else
	{
		if(Rcnt && Gcnt && Bcnt) rgb2XYZ(Rcnt, Gcnt, Bcnt, X, Y, Z);
	}

	return true;
}


bool
openI1D3probe::measureOverIntegrationTime(double integrationTime, double& Rcnt, double& Gcnt, double& Bcnt, unsigned int* RrawCnt, unsigned int* GrawCnt, unsigned int* BrawCnt) const
{
	if(integrationTime > 6.0) integrationTime = 6.0;	// maximum integration time

	// integrationTime in seconds, converted to the number of clock cycles of the system clock (12Mhz)
	unsigned int clockCycleCount = (unsigned int)((integrationTime * I1D3_CLK_FEQ) + 0.5);

	// calculate the actual time integrated over in seconds.  This compensates for the integer nature of the
	// time period of a single clock cycle
	integrationTime = (double)clockCycleCount / I1D3_CLK_FEQ; 

	unsigned int Rct(0);
	unsigned int Gct(0);
	unsigned int Bct(0);

	if(!measureRawOverIntegrationClockCycleCount(clockCycleCount, Rct, Gct, Bct))
	{
		sprintf(_errMsg, "Failed measure command.");
		return false;
	}

	Rct = (Rct > _blackOffSetR) ? (Rct -_blackOffSetR) : 0; 
	Gct = (Gct > _blackOffSetG) ? (Gct -_blackOffSetG) : 0; 
	Bct = (Bct > _blackOffSetB) ? (Bct -_blackOffSetR) : 0;
	
	if(RrawCnt) *RrawCnt = Rct;
	if(GrawCnt) *GrawCnt = Gct;
	if(BrawCnt) *BrawCnt = Bct;

	// it seems the count value is always twice what it shoud be
	// this may be an edge count rather than a cycle clount
	Rcnt = ((double)Rct + 0.5) / 2.0;
	Gcnt = ((double)Gct + 0.5) / 2.0;
	Bcnt = ((double)Bct + 0.5) / 2.0;

	Rcnt /=	integrationTime;
	Gcnt /=	integrationTime;
	Bcnt /=	integrationTime;

	if(!Rct) Rcnt = 0.0;
	if(!Gct) Gcnt = 0.0;
	if(!Bct) Bcnt = 0.0;

	return true;
}


bool
openI1D3probe::measureOverSignalCount(unsigned short rc, unsigned short gc, unsigned short bc, double& Rval, double& Gval, double& Bval) const
{
	// measure for a specific measurment count has been reached, rc, gc, bc 2-65534 even
	unsigned int Rcnt(0), Gcnt(0), Bcnt(0);

	if(rc) { rc /= 2 ; rc *= 2; (rc) ? rc: 2; }	// make even, miniumum 2
	if(gc) { gc /= 2 ; gc *= 2; (gc) ? gc: 2; }	// make even, miniumum 2
	if(bc) { bc /= 2 ; bc *= 2; (bc) ? bc: 2; }	// make even, miniumum 2

	if(!measureRawClockCycleCountForSignalCount(rc, gc, bc, Rcnt, Gcnt, Bcnt))
	{
		sprintf(_errMsg, "Failed measure command.");
		return false;
	}

	if(rc) Rval = ((double)I1D3_CLK_FEQ * (double) rc * 0.5) / (double)Rcnt;
	else   Rval = 0;
	if(gc) Gval = ((double)I1D3_CLK_FEQ * (double) gc * 0.5) / (double)Gcnt;
	else   Gval = 0;
	if(bc) Bval = ((double)I1D3_CLK_FEQ * (double) bc * 0.5) / (double)Bcnt;
	else   Bval = 0;

	return true;
}


bool
openI1D3probe::measureAIO(double integrationTime, double& Rval, double& Gval, double& Bval, unsigned int* RrawCnt, unsigned int* GrawCnt, unsigned int* BrawCnt)	const
{
	if(!_isRevB)
	{
		sprintf(_errMsg, "AIO measurment only supported on B-02 harware.");
		return false;
	}

	if(integrationTime > 6.0) integrationTime = 6.0;	// maximum integration time

	// integrationTime in seconds, converted to the number of clock cycles of the system clock (12Mhz)
	unsigned int clockCycleCount = (unsigned int)((integrationTime * I1D3_CLK_FEQ) + 0.5);
	// calculate the actual time integrated over in seconds.  This compensates for the integer nature of the
	// time period of a single clock cycle
	integrationTime = (double)clockCycleCount / I1D3_CLK_FEQ; 

	unsigned int Rcnt(0);
	unsigned int Gcnt(0);
	unsigned int Bcnt(0);
	unsigned int Rtik(0);
	unsigned int Gtik(0);
	unsigned int Btik(0);

	unsigned short threshhold(1000);

	if(!measureRawAIO(clockCycleCount, threshhold, Rcnt, Gcnt, Bcnt, Rtik, Gtik, Btik))
	{
		sprintf(_errMsg, "Failed to take raw AIO measurement.");
		return false;
	}

	Rcnt = (Rcnt > _blackOffSetR) ? (Rcnt -_blackOffSetR) : 0; 
	Gcnt = (Gcnt > _blackOffSetG) ? (Gcnt -_blackOffSetG) : 0; 
	Bcnt = (Bcnt > _blackOffSetB) ? (Bcnt -_blackOffSetR) : 0; 

	if(RrawCnt) *RrawCnt = Rcnt;
	if(GrawCnt) *GrawCnt = Gcnt;
	if(BrawCnt) *BrawCnt = Bcnt;

 	// it seems the value is alwas twice what it shoud be
	// this may be an edge count rather than a cycle clount
	Rval = ((double)Rcnt + 0.5) / 2.0;
	Gval = ((double)Gcnt + 0.5) / 2.0;
	Bval = ((double)Bcnt + 0.5) / 2.0;

	if(Rtik) Rval = ((double)I1D3_CLK_FEQ * Rval * 2.0) / (double)Rtik;
	else	 Rval = 0;
	if(Gtik) Gval = ((double)I1D3_CLK_FEQ * Gval * 2.0) / (double)Gtik;
	else	 Gval = 0;
	if(Btik) Bval = ((double)I1D3_CLK_FEQ * Bval * 2.0) / (double)Btik;
	else	 Bval = 0;

	return true;
}

#if 0
void
openI1D3probe::rgb2XYZ(double Rval, double Gval, double Bval, double& X, double& Y, double& Z) const
{

	openI1D3vec i(Rval, Gval, Bval);
	openI1D3vec o;
	o = _calMtx * i;

	X = o[0];
	Y = o[1];
	Z = o[2];
}
#endif

void
openI1D3probe::rgb2XYZ(double Rval, double Gval, double Bval, double& X, double& Y, double& Z) const
{
	_calMtx.multiply(Rval, Gval, Bval, X, Y, Z);
}


bool
openI1D3probe::ledCtrl(double onTime, double offTime, unsigned int count, bool fadeOnFlg) const
{
	unsigned int onT;
	unsigned int ofT;

	// on and off value is a multiple of 0x80000 * 1/12Mhz clock period
	onT = (int)((onTime  * ((double)I1D3_CLK_FEQ / 0x80000)) + 0.5);
	ofT = (int)((offTime * ((double)I1D3_CLK_FEQ / 0x80000)) + 0.5);

	// if we are fading the resolution increased by a factor of 16
	// on fade value is a multiple of 0x800000 * 1/12Mhz clock period
	if(fadeOnFlg) onT = (int)((onTime  * ((double)I1D3_CLK_FEQ / 0x800000)) + 0.5);

	onT = (onT < 0)	? 0 : ((onT > 255) ? 255 : onT);
	ofT = (ofT < 0)	? 0 : ((ofT > 255) ? 255 : ofT);

	count = (count > 0x80) ? 0x80 : count;

	unsigned char tBuf[64];
	unsigned char fBuf[64];
	unsigned short cmd;

	memset(tBuf, 0, 64);
	memset(fBuf, 0, 64);

	tBuf[1] = fadeOnFlg ? 0x03 : 0x01;
	tBuf[2] = ofT;
	tBuf[3] = onT;
	tBuf[4] = count;

	cmd = 0x2100;

	if(!sendCommand(cmd, tBuf, fBuf, 60))
	{
		sprintf(_errMsg, "Failed led command.");
		return false;
	}

	return true;
}


bool
openI1D3probe::isDiffuserInPosition(bool& isActive) const
{ 
	unsigned char tBuf[64];
	unsigned char fBuf[64];
	unsigned short cmd;


	memset(tBuf, 0, 64);
	memset(fBuf, 0, 64);

	// Check diffuser position
	cmd = 0x9400;

	// this command does not seem to operate like the others
	// it does not return the command code in byte fBuf[1],
	// so sendCommand with return error, even though there is no error
	sendCommand(cmd, tBuf, fBuf);

	isActive = false;
	if(fBuf[1] != 0) isActive = true;

	return true;
}


bool
openI1D3probe::measureRawOverIntegrationClockCycleCount(unsigned int clockCycleCount, unsigned int& Rct, unsigned int& Gct, unsigned int& Bct) const
{
	// measures the raw sensor count over a specific time period.
	// the time period is defined as the number of edged of a 12MHz clock
	// there are 2 edges in each clock cycle
	// so 0.2 sec = 200 mSec, so we need 12000000 x 0.2 x 2 = 4800000 edges
	unsigned char tBuf[64];
	unsigned char fBuf[64];
	unsigned short cmd;

	memset(tBuf, 0, 64);
	memset(fBuf, 0, 64);

	tBuf[1] = (clockCycleCount >> 0)  & 0xff;
	tBuf[2] = (clockCycleCount >> 8)  & 0xff;
	tBuf[3] = (clockCycleCount >> 16) & 0xff;
	tBuf[4] = (clockCycleCount >> 24) & 0xff;

	cmd = 0x0100;

	if(!sendCommand(cmd, tBuf, fBuf, 60))
	{
		sprintf(_errMsg, "Failed measure command.");
		return false;
	}

	Rct = (fBuf[2]  << 0) | (fBuf[3]  << 8) |(fBuf[4]  << 16) | (fBuf[5]  << 24);
	Gct = (fBuf[6]  << 0) | (fBuf[7]  << 8) |(fBuf[8]  << 16) | (fBuf[9]  << 24);
	Bct = (fBuf[10] << 0) | (fBuf[11] << 8) |(fBuf[12] << 16) | (fBuf[13] << 24);

	return true;
}


bool
openI1D3probe::measureRawClockCycleCountForSignalCount(unsigned short rc, unsigned short gc, unsigned short bc, unsigned int& Rct, unsigned int& Gct, unsigned int& Bct) const
{
	// measure the period of time (in system clock cycles) edges to achieve specific signal count (rc, gc, bc)
	// if rc or gc or bc are zero, then the coresponding channel is NOT measured
	// returned values Rct, Gct, Bct are the measurement

	unsigned char tBuf[64];
	unsigned char fBuf[64];
	unsigned short cmd;

	unsigned char channelMask(0);
	if(rc) channelMask += 4;
	if(gc) channelMask += 2;
	if(bc) channelMask += 1;

	memset(tBuf, 0, 64);
	memset(fBuf, 0, 64);

	tBuf[1] = (rc >> 0) & 0xff;
	tBuf[2] = (rc >> 8) & 0xff;
	tBuf[3] = (gc >> 0) & 0xff;
	tBuf[4] = (gc >> 8) & 0xff;
	tBuf[5] = (bc >> 0) & 0xff;
	tBuf[6] = (bc >> 8) & 0xff;
	tBuf[7] = channelMask;

	cmd = 0x0200;

	if(!sendCommand(cmd, tBuf, fBuf, 60))
	{
		sprintf(_errMsg, "Failed measure command.");
		return false;
	}

	if(rc) Rct = (fBuf[2]  << 0) | (fBuf[3]  << 8) |(fBuf[4]  << 16) | (fBuf[5]  << 24);
	else   Rct = 0;
	if(gc) Gct = (fBuf[6]  << 0) | (fBuf[7]  << 8) |(fBuf[8]  << 16) | (fBuf[9]  << 24);
	else   Gct = 0;
	if(bc) Bct = (fBuf[10] << 0) | (fBuf[11] << 8) |(fBuf[12] << 16) | (fBuf[13] << 24);
	else   Bct = 0;

	return true;
}


bool
openI1D3probe::measureRawAIO(unsigned int clockCycleCount, unsigned short threshold, unsigned int& Rcnt, unsigned int& Gcnt, unsigned int& Bcnt, unsigned int& Rtik, unsigned int& Gtik, unsigned int& Btik) const
{
	if(!_isRevB)
	{
		sprintf(_errMsg, "AIO measurment only supported on B-02 harware.");
		return false;
	}

	// measures the raw sensor count over a specific time period.
	// the time period is defined as the number of edged of a 12MHz clock
	// there are 2 edges in each clock cycle
	// so 0.2 sec = 200 mSec, so we need 12000000 x 0.2 x 2 = 4800000 edges
	unsigned char tBuf[64];
	unsigned char fBuf[64];
	unsigned short cmd;

	memset(tBuf, 0, 64);
	memset(fBuf, 0, 64);

	tBuf[1] = (clockCycleCount >> 0)  & 0xff;
	tBuf[2] = (clockCycleCount >> 8)  & 0xff;
	tBuf[3] = (clockCycleCount >> 16) & 0xff;
	tBuf[4] = (clockCycleCount >> 24) & 0xff;

	tBuf[5] = 0x00;
	tBuf[6] = 0x07;

 	tBuf[7] = (threshold >> 0) & 0xff;
	tBuf[8] = (threshold >> 8) & 0xff;

	cmd = 0x0400;

	if(!sendCommand(cmd, tBuf, fBuf, 60))
	{
		sprintf(_errMsg, "Failed measure command.");
		return false;
	}

	Rcnt = (fBuf[2]  << 0) | (fBuf[3]  << 8) |(fBuf[4]  << 16) | (fBuf[5]  << 24);
	Gcnt = (fBuf[6]  << 0) | (fBuf[7]  << 8) |(fBuf[8]  << 16) | (fBuf[9]  << 24);
	Bcnt = (fBuf[10] << 0) | (fBuf[11] << 8) |(fBuf[12] << 16) | (fBuf[13] << 24);

	Rtik = (fBuf[14] << 0) | (fBuf[15] << 8) |(fBuf[16] << 16) | (fBuf[17] << 24);
	Gtik = (fBuf[18] << 0) | (fBuf[19] << 8) |(fBuf[20] << 16) | (fBuf[21] << 24);
	Btik = (fBuf[22] << 0) | (fBuf[23] << 8) |(fBuf[24] << 16) | (fBuf[25] << 24);

	return true;
}


bool
openI1D3probe::sendCommand(unsigned short cmdCode, unsigned char* sBuf, unsigned char* rBuf, double timeout) const
{
	if(!_hidDev)
	{
		sprintf(_errMsg, "Failed send cmd, no valid HID device.");
		return false;
	}

	unsigned char cmd;		// Major command code
	int num;

	cmd = (cmdCode >> 8) & 0xff;	// Major command == HID report number
	sBuf[0] = cmd;

	if(cmd == 0x00) sBuf[1] = (cmdCode & 0xff);	// Minor command

	num = openI1D3writeHIDdevice(_hidDev, sBuf, 64, timeout);
	if(num == -1)
	{
		// flush any crap
		num = openI1D3readHIDdevice(_hidDev, rBuf, 64, timeout);

		sprintf(_errMsg, "Failed send cmd, bad HID write.");
		return false;
	}

	num = openI1D3readHIDdevice(_hidDev, rBuf, 64, timeout);
	if(num == -1)
	{
		// flush any crap
		num = openI1D3readHIDdevice(_hidDev, rBuf, 64, timeout);

		sprintf(_errMsg, "Failed send cmd, bad HID read.");
		return false;
	}

	// The first byte returned is command result error code
	if((rBuf[0] != 0x00) || (rBuf[1] != cmd))
	{
		sprintf(_errMsg, "Failed send cmd, i1d3 internal error.");
		return false;
	}

	return true; 
}


void createUnLockResponse(int key0, int key1, unsigned char iBuf[64], unsigned char oBuf[64])
{
	union int4uchar
	{
		int				_i;
		unsigned char	_uc[4];
	};

	unsigned char iXor(iBuf[3]);	// the input buffer is decoded with XOR function using byte 3
	unsigned char oXor(iBuf[2]);	// the output buffer will ecoded with XOR function using byte 2

	// decode the input buffer and re-order
	int4uchar val[2];
	val[0]._uc[2] = iXor ^ iBuf[0x23];
	val[1]._uc[3] = iXor ^ iBuf[0x24];
	val[1]._uc[1] = iXor ^ iBuf[0x25];
	val[0]._uc[3] = iXor ^ iBuf[0x26];
	val[0]._uc[1] = iXor ^ iBuf[0x27];
	val[1]._uc[0] = iXor ^ iBuf[0x28];
	val[0]._uc[0] = iXor ^ iBuf[0x29];
	val[1]._uc[2] = iXor ^ iBuf[0x2a];

	int4uchar sum[2];
	sum[0]._i = -(key0 + val[1]._i);
	sum[1]._i = -(key1 + val[0]._i);

	int4uchar prod[2];
	prod[0]._i = -(key0 * val[1]._i);
	prod[1]._i = -(key1 * val[0]._i);

	int4uchar key;
	key._i  = val[0]._uc[0] + val[0]._uc[1] + val[0]._uc[2] + val[0]._uc[3];
	key._i += val[1]._uc[0] + val[1]._uc[1] + val[1]._uc[2] + val[1]._uc[3];
	key._i += ((-key0 >> 0) & 0xff) + ((-key0 >> 8) & 0xff) + ((-key0 >> 16) & 0xff) + ((-key0 >> 24) & 0xff);
	key._i += ((-key1 >> 0) & 0xff) + ((-key1 >> 8) & 0xff) + ((-key1 >> 16) & 0xff) + ((-key1 >> 24) & 0xff);

	memset(oBuf, 0x00, 64);

	oBuf[0x18] = oXor ^ (sum [0]._uc[2] + key._uc[0]);
	oBuf[0x19] = oXor ^ (prod[0]._uc[1] - key._uc[1]);
	oBuf[0x1a] = oXor ^ (prod[1]._uc[0] + key._uc[1]);
	oBuf[0x1b] = oXor ^ (sum [1]._uc[2] + key._uc[0]);

	oBuf[0x1c] = oXor ^ (prod[0]._uc[2] - key._uc[1]);
	oBuf[0x1d] = oXor ^ (prod[1]._uc[2] - key._uc[0]);
	oBuf[0x1e] = oXor ^ (sum [1]._uc[3] - key._uc[0]);
	oBuf[0x1f] = oXor ^ (sum [0]._uc[0] - key._uc[1]);

	oBuf[0x20] = oXor ^ (prod[1]._uc[1] + key._uc[0]);
	oBuf[0x21] = oXor ^ (prod[0]._uc[3] - key._uc[1]);
	oBuf[0x22] = oXor ^ (sum [0]._uc[1] + key._uc[0]);
	oBuf[0x23] = oXor ^ (sum [1]._uc[1] - key._uc[1]);

	oBuf[0x24] = oXor ^ (sum [1]._uc[0] + key._uc[1]);
	oBuf[0x25] = oXor ^ (prod[1]._uc[3] + key._uc[1]);
	oBuf[0x26] = oXor ^ (prod[0]._uc[0] + key._uc[0]);
	oBuf[0x27] = oXor ^ (sum [0]._uc[3] - key._uc[0]);
}

openI1D3calMtx::openI1D3calMtx()
{
	identity();
}


openI1D3calMtx::~openI1D3calMtx()
{
}


void
openI1D3calMtx::identity()
{
    for(unsigned int r(0); r <3; ++r)
    {
		for(unsigned int c(0); c < 3; ++c)
		{
			if(r == c)_data[r][c] = 1.0;
			else      _data[r][c] = 0.0;
		}
	}
}


openI1D3calMtx&
openI1D3calMtx::operator=(const openI1D3calMtx& copy)
{
    for(unsigned int r(0); r <3; ++r)
    {
		for(unsigned int c(0); c < 3; ++c)
		{
			_data[r][c] = copy._data[r][c];
		}
	}

	return *this;
}


const double*
openI1D3calMtx::operator[](const unsigned int idx) const
{
	return _data[idx];
}


double*
openI1D3calMtx::operator[](const unsigned int idx)
{
	return _data[idx];
}


void
openI1D3calMtx::multiply(const double i0, const double i1, const double i2, double& o0, double& o1, double& o2) const
{
	double iVal[3],  oVal[3];

	iVal[0] = i0;  iVal[1] = i1;  iVal[2] = i2; 
	oVal[0] = 0.0; oVal[1] = 0.0; oVal[2] = 0.0;
	
	multiply(iVal, oVal);

	o0 = oVal[0]; o1 = oVal[1]; o2 = oVal[2]; 
}


void
openI1D3calMtx::multiply(const double iVal[3] , double oVal[3]) const
{
    for(unsigned int r(0); r < 3; ++r)
    {
		oVal[r] = 0.0;

        for(unsigned int c(0); c <3; ++c)
        {
            oVal[r] += _data[r][c] * iVal[c];
        }
    }
}
