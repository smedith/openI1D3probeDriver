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


#ifndef _openI1D3probe_h
#define _openI1D3probe_h

#ifdef	_WIN32
	// for Win
    #if defined(OPEN_I1D3_LIB_STATIC)
        #define	OPEN_I1D3_LIB_API
    #else
	    #if defined(OPEN_I1D3_LIB_I1D3_EXPORTS)
		    #define	OPEN_I1D3_LIB_API __declspec(dllexport)
	    #else
		    #define	OPEN_I1D3_LIB_API __declspec(dllimport)
	    #endif
    #endif
#else
#endif  //_WIN32

class openI1D3hidIdevice;

class OPEN_I1D3_LIB_API openI1D3calMtx
{
public:
                                openI1D3calMtx();                           // default constructor
virtual                        ~openI1D3calMtx();                           // destructor

    void                        identity();
    openI1D3calMtx&             operator=(const openI1D3calMtx& copy);      // asignment
    const double*               operator[](const unsigned int idx) const;   // row index
    double*                     operator[](const unsigned int idx);         // row index
    void                        multiply(const double i0, const double i1, const double i2, double& o0, double& o1, double& o2) const;
    void                        multiply(const double iVal[3] , double oVal[3]) const;

protected:

private:

    double                      _data[3][3];
};


class OPEN_I1D3_LIB_API openI1D3probe
{
public:

        enum measurementMode
        {
            frequency       = 0,
            period          = 1,
            aio             = 2
        };


                                openI1D3probe();            // default constructor
         virtual               ~openI1D3probe();            // destructor

         bool                   initConnection(unsigned int* productID = 0);
         bool                   initCalibration(const openI1D3calMtx* calMtx = 0, bool diffuserActive = false);     //  init calibration, NULL gived default calibration
         bool                   calMtxFromEDRdata(const unsigned char* data, unsigned long len, openI1D3calMtx& calMtx, bool diffuserActive = false) const;
         bool                   calMtxFromSpectraldata(double data[][401],   unsigned long num, openI1D3calMtx& calMtx, bool diffuserActive = false) const;


         bool                   unLock(unsigned int* typeID = 0) const;
         void                   probeTypeIDtoName(unsigned int id, char* probeTypeName);

         bool                   getInfo(char rBuf[64]) const;
         bool                   readInternalEeprom (unsigned char buf[256])  const;
         bool                   writeInternalEeprom(unsigned char buf[256])  const;
         bool                   readExternalEeprom (unsigned char buf[8192]) const;
         bool                   writeExternalEeprom(unsigned char buf[8192]) const;

         void                   initFromInternalEeprom(unsigned char buf[256]);
         void                   initFromExternalEeprom(unsigned char buf[8192]);

         void                   getInternalEepromBufferSerialNumber(unsigned char buf[256], char* serNum) const;
         void                   setInternalEepromBufferSerialNumber(unsigned char buf[256], char* serNum) const;

         unsigned int           calExternalEepromChkksum(unsigned char buf[8192], bool alt = false) const;
         void                   setExternalEepromChkksum(unsigned char buf[8192], unsigned int sum) const;
         unsigned int           getExternalEepromChkksum(unsigned char buf[8192]) const;

         bool                   enableEepromWriteAccess() const;


                                // take a measurement over an intagration time (seconds), returns CIE CdM-2
         bool                   measure(measurementMode mode, double integrationTime, double& X, double& Y, double& Z, bool rawRGBflg = false) const;


                                // Integration time in seconds, return signal counts per second. Optional RrawCnt,GrawCnt,GrawCnt raw sensor count
         bool                   measureOverIntegrationTime(double integrationTime, double& Rcnt, double& Gcnt, double& Bcnt, unsigned int* RrawCnt = 0, unsigned int* GrawCnt = 0, unsigned int* BrawCnt = 0) const;
                                // rc, gc, gb are desired signal count, return (Rcnt, Gcnt, Bcnt) signal counts per second
         bool                   measureOverSignalCount(unsigned short rc, unsigned short gc, unsigned short bc, double& Rcnt, double& Gcnt, double& Bcnt) const;
                                // Integration time in seconds, return signal counts per second. Optional RrawCnt,GrawCnt,GrawCnt raw sensor count
         bool                   measureAIO(double integrationTime, double& Rcnt, double& Gcnt, double& Bcnt, unsigned int* RrawCnt = 0, unsigned int* GrawCnt = 0, unsigned int* BrawCnt = 0) const;
                                // Converts counts per second to CIE XYZ Cdm-2
         void                   rgb2XYZ(double Rcnt, double Gcnt, double Bcnt, double& X, double& Y, double& Z) const;                                          

                                // contol the visual feed back led.  times in seconds, count max is 0x80 which is forever
         bool                   ledCtrl(double onTime, double offTime, unsigned int count = 0x80, bool fadeOnFlg = false) const;                                          
         bool                   ledOn()  const { return ledCtrl(0.5, 0.0, 0x80); };                                          
         bool                   ledOff() const { return ledCtrl(0.0, 0.0, 0x00); };
         
         bool                   isDiffuserInPosition(bool& isActive) const;


         bool                   getIsRevB()       const { return _isRevB; };
         const char*            getSerialNumber() const { return _serNum; };
         const char*            getHWrevNumber()  const { return _revNum; };
         const char*            getErrMsg()       const { return _errMsg; };

protected:

private:
        
                                // Measures for a specific integration time defined as system clock count, return signal counts per second
        bool                    measureRawOverIntegrationClockCycleCount(unsigned int clockCycleCount, unsigned int& Rct, unsigned int& Gct, unsigned int& Bct) const;

                                // Measures for a specific signal count (rc,gc,bc), returms the number of system clocks (time) required to get that signal count.
        bool                    measureRawClockCycleCountForSignalCount(unsigned short rc, unsigned short gc, unsigned short bc, unsigned int& Rct, unsigned int& Gct, unsigned int& Bct) const;

        bool                    measureRawAIO(unsigned int clockCycleCount, unsigned short threshold, unsigned int& Rcnt, unsigned int& Gcnt, unsigned int& Bcnt, unsigned int& Rtik, unsigned int& Gtik, unsigned int& Btik) const;

        bool                    sendCommand(unsigned short cmdCode, unsigned char* sBuf, unsigned char* rBuf, double timeout = 1.0) const;


	    char                    _serNum[21];
	    char                    _revNum[11];
        bool                    _isRevB;

        unsigned int            _blackOffSetR;
        unsigned int            _blackOffSetG;
        unsigned int            _blackOffSetB;

        double                  _calibrationR[351];
        double                  _calibrationG[351];
        double                  _calibrationB[351];
        double                  _calibrationA[351];

        openI1D3calMtx          _calMtx;

        openI1D3hidIdevice*     _hidDev;

        mutable char            _errMsg[256];
};

#endif //_openI1D3probe_h
