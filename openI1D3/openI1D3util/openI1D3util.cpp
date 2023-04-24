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
#include <iostream>
#include <fstream>

#include "openI1D3probe.h"


using namespace std;

int optind(1), optopt;
char* optarg;

#define BADCH   (int)'?'
#define BADARG  (int)':'
#define EMSG    ""

int getopt(int nargc, char * const nargv[], const char *ostr)
{
	static char *place = EMSG;              /* option letter processing */
	const char *oli;                        /* option letter list index */

	if(!*place)
	{
		if(optind >= nargc || *(place = nargv[optind]) != '-')
		{
			place = EMSG;
			return -1;
		}

		if(place[1] && *++place == '-')
		{
			++optind;
			place = EMSG;
			return -1;
		}
	}

	if((optopt = (int)*place++) == (int)':' || !(oli = strchr(ostr, optopt)))
	{
		if(optopt == (int)'-')  return -1;
		if(!*place) ++optind;
		return BADCH;
	}

	if(*++oli != ':')
	{
		optarg = NULL;
		if(!*place) ++optind;
	}
	else
	{
		if(*place) optarg = place;
		else if (nargc <= ++optind)
		{
			place = EMSG;
			if(*ostr == ':') return (BADARG);
			return (BADCH);
		}
		else optarg = nargv[optind];
		place = EMSG;
		++optind;
	}

	return optopt;
}

int main(int argc, char **argv)
{
	cout << "openI1D3util ver 1.2" << endl;


    char* fileName(0);
	bool verNum(false);
	bool forceOverWrite(false);
	bool enableEEPROMwrite(false);

	bool rSerNum(false);
	bool wSerNum(false);
	bool rIeeprom(false);
	bool wIeeprom(false);
	bool rEeeprom(false);
	bool wEeeprom(false);
	bool rSig(false);
	bool wSig(false);

	bool mRaw(false);
	openI1D3probe::measurementMode mType(openI1D3probe::frequency);
	double mMeasurementTime(0.25);
	bool   mMeasureContinuously(false);
	char* mEDFfilename(0);
	unsigned char* mEDFfileBuf(0);
	unsigned int   mEDFfileLen(0);

	bool led(false);
	bool ledOn(false);

	bool dif(false);

    int  opt(0);
    while(1)
    {
        opt = getopt(argc, argv, "fwvnNiIeEsSm:M:t:cl:d");
        
        if(opt == -1) break;
                
        switch(opt)
        {
            case 'f':
            {
                forceOverWrite = true;
            }
            break;
            
            case 'w':
            {
                enableEEPROMwrite = true;
            }
            break;
            
            case 'v':
            {
				verNum = true;
            }
            break;
            
            case 'n':
            {
				rSerNum = true;
            }
            break;
            
            case 'N':
            {
				wSerNum = true;
            }
            break;
            
            case 'i':
            {
				rIeeprom = true;
            }
            break;
            
            case 'I':
            {
				wIeeprom = true;
            }
            break;
            
            case 'e':
            {
				rEeeprom = true;
            }
            break;
            
            case 'E':
            {
				wEeeprom = true;
            }
            break;
            
            case 's':
            {
				rSig = true;
            }
            break;
            
            case 'S':
            {
				wSig = true;
            }
            break;
            
            case 'm':
            {
				mRaw = true;
				if(optarg)
				{
					if     (strcmp(optarg, "freq")   == 0) mType = openI1D3probe::frequency;
					else if(strcmp(optarg, "period") == 0) mType = openI1D3probe::period;
					else if(strcmp(optarg, "aio")    == 0) mType = openI1D3probe::aio;
					else                                   mType = openI1D3probe::frequency;
				}
            }
            break;
            
            case 'M':
            {
				if(optarg)
				{
					unsigned int len = strlen(optarg);
					mEDFfilename = new char[len + 1];
					strcpy(mEDFfilename, optarg);
				}
            }
            
            case 't':
            {
				if(optarg)
				{
					double val = atof(optarg);
					if((val > 0.0) & (val < 6.0)) mMeasurementTime = val;
				}
            }
            break;
            
            case 'c':
            {
				mMeasureContinuously = true;
            }
            break;
            
            case 'l':
            {
				led = true;
				if(strcmp(optarg, "on") == 0) ledOn = true;
            }
            break;
            
            case 'd':
            {
				dif = true;
            }
            break;
            
            case '?':
            {
 	        cout																			<< endl;
	        cout << "openI1D3util <options> <filename>"										<< endl;
	        cout																			<< endl;
            cout << " -v              read the i1d3 firmware version information"			<< endl;
	        cout																			<< endl;
            cout << " -n              read the i1d3 serial number"							<< endl;
            cout << " -N              write the i1d3 serial number"							<< endl;
	        cout																			<< endl;
            cout << " -i              read the internal eeprom and write it to a file"		<< endl;
            cout << " -I              read a file and load it into the internal eeprom"		<< endl;
	        cout																			<< endl;
            cout << " -e              read the external eeprom and write it to a file"		<< endl;
            cout << " -E              read a file and load it into the external eeprom"		<< endl;
	        cout																			<< endl;
            cout << " -s              read external eeprom signature and write to a file"	<< endl;
            cout << " -S              read a signature file and update the external eeprom"	<< endl;
	        cout																			<< endl;
            cout << " -f              force file overwrite"									<< endl;
            cout << " -w              enable eeprom writing"								<< endl;
	        cout																			<< endl;
            cout << " -m <type>       measure type <freq, period, aio>"                     << endl;
            cout << " -t <time>       measure time <sec, default 0.25>"                     << endl;
            cout << " -c              measure continuously"                                 << endl;
            cout << " -M <filename>   measure edr file"					                    << endl;
	        cout																			<< endl;
            cout << " -l <on:off>     led"                                                  << endl;
            cout << " -d              diffuser position"                                    << endl;
	        exit(1);
            }
            break;
            
            default:
            {
                cout << "Error: bad option" << endl;
                exit(1);
            }
        }
    }
    
    if(optind < argc)
    {
        fileName = new char[strlen(argv[optind]) + 1];
		strcpy(fileName, argv[optind]);
    }
	else if(rIeeprom || wIeeprom || rEeeprom || wEeeprom || rSig || wSig)
	{
		cout << "Error: missing filename" << endl;
		exit(1);
	}
	
	if(wSerNum && !fileName)
	{
		cout << "Error: missing serial number" << endl;
		exit(1);
	}

    if(!fileName && !verNum && !rSerNum && !wSerNum && !rIeeprom && !wIeeprom && !rEeeprom && !wEeeprom && !rSig && !wSig && !led && !dif)
	{
        cout << "i1d3util -? for help" << endl;
	}

	
	if(mEDFfilename)
	{
		HANDLE hd = CreateFile(mEDFfilename, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
		if(hd == INVALID_HANDLE_VALUE)
		{
			cout << "Error: Failed to open file " << mEDFfilename << " for reading" << endl;
			if(mEDFfilename) delete[] mEDFfilename;
			exit(-1);
		}

		DWORD edrFileLen= GetFileSize(hd, NULL);
		if(edrFileLen == INVALID_FILE_SIZE )
		{
			cout << "Error: Failed to open file " << mEDFfilename << " for reading" << endl;
			if(mEDFfilename) delete[] mEDFfilename;
			exit(-1);
		}

		if(SetFilePointer(hd, 0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
		{
			cout << "Error: Failed to open file " << mEDFfilename << " for reading" << endl;
			if(mEDFfilename) delete[] mEDFfilename;
			exit(-1);
		}

		mEDFfileBuf = new unsigned char[edrFileLen];
		memset(mEDFfileBuf, 0x00, edrFileLen);

		DWORD noRead(0);
		if(!ReadFile(hd, mEDFfileBuf, edrFileLen, &noRead, NULL))
		{
			cout << "Error: Failed to read file " << mEDFfilename << endl;
			if(mEDFfilename) delete[] mEDFfilename;
			delete[] mEDFfileBuf;
			exit(-1);
		}

		if (noRead != edrFileLen)
		{
			cout << "Error: Failed to read file " << mEDFfilename << endl;
			if(mEDFfilename) delete[] mEDFfilename;
			delete[] mEDFfileBuf;
			exit(-1);
		}

		if(!CloseHandle(hd))
		{
			cout << "Error: Failed to close file " << mEDFfilename << endl;
			if(mEDFfilename) delete[] mEDFfilename;
			delete[] mEDFfileBuf;
			exit(-1);
		}

		mEDFfileLen	= edrFileLen;
	}

	openI1D3probe probeI1D3;
	unsigned int probeProductID(0);

 	if(!probeI1D3.initConnection(&probeProductID))
	{
        cout << "Error: failed to init i1d3" << endl;
        exit(-1);
	}

	if(probeProductID == 0x5021)
	{
        cout << "Warning: The product ID is 0x5021!  This may mean you internal eeprom is corrupt." << endl;
        cout << "We will attempt to reset it." << endl;

		// For some reason, if you modify the internal eeprom to change the serial number, the i1d3 returns back with the product code 0x5021
		//
		// Reading the internal eeprom seems to reset this issue, but you MUST unplug and plug back in the USB connection
		// I think this resets the device driver ?
		unsigned int id(0);
		if(!probeI1D3.unLock(&id))
		{
			cout << "Error: " <<  probeI1D3.getErrMsg() << endl;
			exit(-1);
		}

		if(!probeI1D3.enableEepromWriteAccess())
		{
			cout << "Error: " <<  probeI1D3.getErrMsg() << endl;
			exit(-1);
		}

		unsigned char fBuf[256];
		memset(fBuf, 0x00, 256);
		probeI1D3.readInternalEeprom(fBuf);

		if(fileName) delete[] fileName;
        cout << "Please disconnect then reconnect the USB before re-running this program" << endl;

		exit(0);
	}

	if(verNum)
	{
		char rBuf[64];
		memset(rBuf, 0x00, 64);
		probeI1D3.getInfo(rBuf);
        cout << rBuf << endl;

		unsigned int id(0);
		if(!probeI1D3.unLock(&id))
		{
			cout << "Error: " <<  probeI1D3.getErrMsg() << endl;
			exit(-1);
		}

		char name[256];
		probeI1D3.probeTypeIDtoName(id, name);
		cout << "I1D3 " << name << endl;

		exit(0);
	}
	else if(rSerNum)
	{
		if(!probeI1D3.unLock())
		{
			if(fileName) delete[] fileName;
			cout << "Error: " <<  probeI1D3.getErrMsg() << endl;
			exit(-1);
		}

		unsigned char* eBuf = new unsigned char[256];
		memset(eBuf, 0x00, 256);

		if(!probeI1D3.readInternalEeprom(eBuf))
		{
			cout << "Error: " <<  probeI1D3.getErrMsg() << endl;
			exit(-1);
		}

		char serNum[21];
		probeI1D3.getInternalEepromBufferSerialNumber(eBuf, serNum);

		cout << serNum << endl;

		delete[] eBuf;

		exit(0);
	}
	else if(wSerNum)
	{
		if(!probeI1D3.unLock())
		{
			if(fileName) delete[] fileName;
			cout << "Error: " <<  probeI1D3.getErrMsg() << endl;
			exit(-1);
		}

		if(!probeI1D3.enableEepromWriteAccess())
		{
			if(fileName) delete[] fileName;
			cout << "Error: " <<  probeI1D3.getErrMsg() << endl;
			exit(-1);
		}

		unsigned char* eBuf = new unsigned char[256];
		memset(eBuf, 0x00, 256);
		
		if(!probeI1D3.readInternalEeprom(eBuf))
 		{
			if(fileName) delete[] fileName;
			cout << "Error: " <<  probeI1D3.getErrMsg() << endl;
			exit(-1);
		}


		char serNum[21];
		memset(serNum, 0x00, 21);
		memcpy(serNum, fileName, 20);

		probeI1D3.setInternalEepromBufferSerialNumber(eBuf, serNum);

		if(enableEEPROMwrite)
		{
			if(!probeI1D3.writeInternalEeprom(eBuf))
 			{
				if(fileName) delete[] fileName;
				cout << "Error: " <<  probeI1D3.getErrMsg() << endl;
				exit(-1);
			}
		}
		else
		{
			cout << "EEPROM write not enabled, use -w" << endl;
			exit(-1);
		}

		cout << "Serial number " << fileName << " successfully written to the internal eeprom" << endl;
		cout << "Now unplug and plugin the USB connection" << endl;

		delete[] eBuf;

		exit(0);
	}
	else if(rIeeprom)
	{
		if(!probeI1D3.unLock())
		{
			if(fileName) delete[] fileName;
			cout << "Error: " <<  probeI1D3.getErrMsg() << endl;
			exit(-1);
		}

		HANDLE hd;
		if(forceOverWrite)
		{
			hd = CreateFile(fileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
		}
		else
		{
			hd = CreateFile(fileName, GENERIC_WRITE, 0, NULL, CREATE_NEW, 0, NULL);
		}

		if(hd == INVALID_HANDLE_VALUE)
		{
			cout << "Error: Failed to open file " << fileName << " for writing" << endl;
			if(fileName) delete[] fileName;
			exit(-1);
		}

		if(SetFilePointer(hd, 0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
		{
			cout << "Error: Failed to open file " << fileName << " for writing" << endl;
			if(fileName) delete[] fileName;
			exit(-1);
		}

		unsigned char* eBuf = new unsigned char[256];
		memset(eBuf, 0x00, 256);
		if(!probeI1D3.readInternalEeprom(eBuf))
 		{
			if(fileName) delete[] fileName;
			cout << "Error: " <<  probeI1D3.getErrMsg() << endl;
			exit(-1);
		}

		DWORD noWritten(0);
		if(!WriteFile(hd, eBuf, 256, &noWritten, NULL)) return -1;
		if (noWritten != 256)
		{
			cout << "Error: Failed to write file " << fileName << endl;
			if(fileName) delete[] fileName;
			delete[] eBuf;
			exit(-1);
		}

		cout << "Internal eeprom memory written to file " << fileName << endl;

		if(!CloseHandle(hd))
		{
			cout << "Error: Failed to close file " << fileName << endl;
			if(fileName) delete[] fileName;
			delete[] eBuf;
			exit(-1);
		}

		delete[] eBuf;

		exit(0);
	}
	else if(wIeeprom)
	{
		HANDLE hd = CreateFile(fileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
		if(hd == INVALID_HANDLE_VALUE)
		{
			cout << "Error: Failed to open file " << fileName << " for reading" << endl;
			if(fileName) delete[] fileName;
			exit(-1);
		}

		if(SetFilePointer(hd, 0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
		{
			cout << "Error: Failed to open file " << fileName << " for reading" << endl;
			if(fileName) delete[] fileName;
			exit(-1);
		}

		unsigned char* eBuf = new unsigned char[256];
		memset(eBuf, 0x00, 256);

		DWORD noRead(0);
		if(!ReadFile(hd, eBuf, 256, &noRead, NULL)) return -1;
		if (noRead != 256)
		{
			cout << "Error: Failed to read file " << fileName << endl;
			if(fileName) delete[] fileName;
			delete[] eBuf;
			exit(-1);
		}

		if(!CloseHandle(hd))
		{
			cout << "Error: Failed to close file " << fileName << endl;
			if(fileName) delete[] fileName;
			delete[] eBuf;
			exit(-1);
		}

		if(!probeI1D3.unLock())
		{
			if(fileName) delete[] fileName;
			cout << "Error: " <<  probeI1D3.getErrMsg() << endl;
			exit(-1);
		}

		if(!probeI1D3.enableEepromWriteAccess())
		{
			if(fileName) delete[] fileName;
			cout << "Error: " <<  probeI1D3.getErrMsg() << endl;
			exit(-1);
		}

		if(enableEEPROMwrite)
		{
			if(!probeI1D3.writeInternalEeprom(eBuf))
			{
				if(fileName) delete[] fileName;
				cout << "Error: " <<  probeI1D3.getErrMsg() << endl;
				exit(-1);
			}
		}
		else
		{
			cout << "EEPROM write not enabled, use -w" << endl;
			exit(-1);
		}

		cout << "File " << fileName << " successfully written to the internal eeprom" << endl;
		cout << "Now unplug and plugin the USB connection" << endl;

		delete[] eBuf;

		exit(0);
	}
	else if(rEeeprom)
	{
		if(!probeI1D3.unLock())
		{
			if(fileName) delete[] fileName;
			cout << "Error: " <<  probeI1D3.getErrMsg() << endl;
			exit(-1);
		}

		HANDLE hd;
		if(forceOverWrite)
		{
			hd = CreateFile(fileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
		}
		else
		{
			hd = CreateFile(fileName, GENERIC_WRITE, 0, NULL, CREATE_NEW, 0, NULL);
		}

		if(hd == INVALID_HANDLE_VALUE)
		{
			cout << "Error: Failed to open file " << fileName << " for writing" << endl;
			if(fileName) delete[] fileName;
			exit(-1);
		}

		if(SetFilePointer(hd, 0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
		{
			cout << "Error: Failed to open file " << fileName << " for writing" << endl;
			if(fileName) delete[] fileName;
			exit(-1);
		}

		unsigned char* eBuf = new unsigned char[8192];
		memset(eBuf, 0x00, 8192);
		if(!probeI1D3.readExternalEeprom(eBuf))
		{
			if(fileName) delete[] fileName;
			cout << "Error: " <<  probeI1D3.getErrMsg() << endl;
			exit(-1);
		}

		DWORD noWritten(0);
		if(!WriteFile(hd, eBuf, 8192, &noWritten, NULL)) return -1;
		if (noWritten != 8192)
		{
			cout << "Error: Failed to write file " << fileName << endl;
			if(fileName) delete[] fileName;
			delete[] eBuf;
			exit(-1);
		}

		cout << "External eeprom memory written to file " << fileName << endl;

		if(!CloseHandle(hd))
		{
			cout << "Error: Failed to close file " << fileName << endl;
			if(fileName) delete[] fileName;
			delete[] eBuf;
			exit(-1);
		}

		delete[] eBuf;

		exit(0);
	}
	else if(wEeeprom)
	{
		HANDLE hd = CreateFile(fileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
		if(hd == INVALID_HANDLE_VALUE)
		{
			cout << "Error: Failed to open file " << fileName << " for reading" << endl;
			if(fileName) delete[] fileName;
			exit(-1);
		}

		if(SetFilePointer(hd, 0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
		{
			cout << "Error: Failed to open file " << fileName << " for reading" << endl;
			if(fileName) delete[] fileName;
			exit(-1);
		}

		unsigned char* eBuf = new unsigned char[8192];
		memset(eBuf, 0x00, 8192);

		DWORD noRead(0);
		if(!ReadFile(hd, eBuf, 8192, &noRead, NULL))
		{
			cout << "Error: Failed to read file " << fileName << endl;
			if(fileName) delete[] fileName;
			delete[] eBuf;
			exit(-1);
		}

		if (noRead != 8192)
		{
			cout << "Error: Failed to read file " << fileName << endl;
			if(fileName) delete[] fileName;
			delete[] eBuf;
			exit(-1);
		}

		if(!CloseHandle(hd))
		{
			cout << "Error: Failed to close file " << fileName << endl;
			if(fileName) delete[] fileName;
			delete[] eBuf;
			exit(-1);
		}

		if(!probeI1D3.unLock())
		{
			if(fileName) delete[] fileName;
			cout << "Error: " <<  probeI1D3.getErrMsg() << endl;
			exit(-1);
		}

		if(!probeI1D3.enableEepromWriteAccess())
		{
			if(fileName) delete[] fileName;
			cout << "Error: " <<  probeI1D3.getErrMsg() << endl;
			exit(-1);
		}

		if(enableEEPROMwrite)
		{
			if(!probeI1D3.writeExternalEeprom(eBuf))
			{
				if(fileName) delete[] fileName;
				cout << "Error: " <<  probeI1D3.getErrMsg() << endl;
				exit(-1);
			}
		}
		else
		{
			cout << "EEPROM write not enabled, use -w" << endl;
			exit(-1);
		}

		cout << "File " << fileName << " successfully written to the external eeprom" << endl;
		cout << "Now unplug and plugin the USB connection" << endl;

		delete[] eBuf;

		exit(0);
	}
	else if(rSig)
	{
		HANDLE hd;
		if(forceOverWrite)
		{
			hd = CreateFile(fileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
		}
		else
		{
			hd = CreateFile(fileName, GENERIC_WRITE, 0, NULL, CREATE_NEW, 0, NULL);
		}

		if(hd == INVALID_HANDLE_VALUE)
		{
			cout << "Error: Failed to open file " << fileName << " for writing" << endl;
			if(fileName) delete[] fileName;
			exit(-1);
		}

		if(SetFilePointer(hd, 0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
		{
			cout << "Error: Failed to open file " << fileName << " for writing" << endl;
			if(fileName) delete[] fileName;
			exit(-1);
		}

		unsigned char* eBuf = new unsigned char[8192];
		memset(eBuf, 0x00, 8192);
		if(!probeI1D3.readExternalEeprom(eBuf))
		{
			if(fileName) delete[] fileName;
			cout << "Error: " <<  probeI1D3.getErrMsg() << endl;
			exit(-1);
		}

		unsigned char* buf = new unsigned char[0x48];
		memset(buf, 0x00, 0x48);

		memcpy(buf, &eBuf[0x1638], 0x48);
		delete[] eBuf;

		DWORD noWritten(0);
		if(!WriteFile(hd, buf, 0x48, &noWritten, NULL))
		{
			cout << "Error: Failed to write file " << fileName << endl;
			if(fileName) delete[] fileName;
			delete[] buf;
			exit(-1);
		}

		if (noWritten != 0x48)
		{
			cout << "Error: Failed to write file " << fileName << endl;
			if(fileName) delete[] fileName;
			delete[] buf;
			exit(-1);
		}

		cout << "External eeprom memory written to file " << fileName << endl;

		if(!CloseHandle(hd))
		{
			cout << "Error: Failed to close file " << fileName << endl;
			if(fileName) delete[] fileName;
			delete[] buf;
			exit(-1);
		}

		delete[] buf;

		exit(0);
	}
	else if(wSig)
	{
		HANDLE hd = CreateFile(fileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
		if(hd == INVALID_HANDLE_VALUE)
		{
			cout << "Error: Failed to open file " << fileName << " for reading" << endl;
			if(fileName) delete[] fileName;
			exit(-1);
		}

		if(SetFilePointer(hd, 0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
		{
			cout << "Error: Failed to open file " << fileName << " for reading" << endl;
			if(fileName) delete[] fileName;
			exit(-1);
		}

		unsigned char* buf = new unsigned char[0x48];
		memset(buf, 0x00, 0x48);

		DWORD noRead(0);
		if(!ReadFile(hd, buf, 0x48, &noRead, NULL))
		{
			cout << "Error: Failed to read file " << fileName << endl;
			if(fileName) delete[] fileName;
			delete[] buf;
			exit(-1);
		}

		if (noRead != 0x48)
		{
			cout << "Error: Failed to read file " << fileName << endl;
			if(fileName) delete[] fileName;
			delete[] buf;
			exit(-1);
		}

		if(!CloseHandle(hd))
		{
			cout << "Error: Failed to close file " << fileName << endl;
			if(fileName) delete[] fileName;
			delete[] buf;
			exit(-1);
		}

		if(!probeI1D3.unLock())
		{
			if(fileName) delete[] fileName;
			cout << "Error: " <<  probeI1D3.getErrMsg() << endl;
			exit(-1);
		}

		if(!probeI1D3.enableEepromWriteAccess())
		{
			if(fileName) delete[] fileName;
			cout << "Error: " <<  probeI1D3.getErrMsg() << endl;
			exit(-1);
		}

		unsigned char* eBuf = new unsigned char[8192];
		memset(eBuf, 0x00, 8192);
		if(!probeI1D3.readExternalEeprom(eBuf))
		{
			if(fileName) delete[] fileName;
			cout << "Error: " <<  probeI1D3.getErrMsg() << endl;
			exit(-1);
		}

		bool rev1flg(false);
		unsigned int fsum(0);
		fsum =  eBuf[2] | (eBuf[3] << 8);
		unsigned int csum = probeI1D3.calExternalEepromChkksum(eBuf);

		if(csum != fsum)
		{
			// try rev1 calculation
			rev1flg = true;
			csum = probeI1D3.calExternalEepromChkksum(eBuf, rev1flg);
		}

		if(csum != fsum)
		{
			if(fileName) delete[] fileName;

			delete[] buf;
			delete[] eBuf;

			cout << "Error: Checksum of i1d3 external eeprom failed.  This may mean it is not Rev1 or Rev2 hardware" << endl;
			exit(-1);
		}

		memcpy(&eBuf[0x1638], buf, 0x48);

		csum = probeI1D3.calExternalEepromChkksum(eBuf, rev1flg);
		eBuf[2] = (unsigned char)(csum >> 0) & 0xff;
		eBuf[3] = (unsigned char)(csum >> 8) & 0xff;

		
		if(enableEEPROMwrite)
		{
			if(!probeI1D3.writeExternalEeprom(eBuf))
			{
				if(fileName) delete[] fileName;
				cout << "Error: " <<  probeI1D3.getErrMsg() << endl;
				exit(-1);
			}
		}
		else
		{
			cout << "EEPROM write not enabled, use -w" << endl;
			exit(-1);
		}

		cout << "File " << fileName << " signature successfully written to the external eeprom" << endl;
		cout << "Now unplug and plugin the USB connection" << endl;

		delete[] buf;
		delete[] eBuf;

		exit(0);
	}
	else if(mRaw)
	{
		if(!probeI1D3.unLock())
		{
			if(fileName) delete[] fileName;
			cout << "Error: " <<  probeI1D3.getErrMsg() << endl;
			exit(-1);
		}

		unsigned char* eBuf = new unsigned char[8192];
		memset(eBuf, 0x00, 8192);
		probeI1D3.readInternalEeprom(eBuf);
		probeI1D3.initFromInternalEeprom(eBuf);
		probeI1D3.readExternalEeprom(eBuf);
		probeI1D3.initFromExternalEeprom(eBuf);
		delete[] eBuf;

		bool isActive;
		probeI1D3.isDiffuserInPosition(isActive);

		if(isActive) cout << "Diffuser is active " << endl;
		else         cout << "Diffuser is not active " << endl;

		if(mEDFfileBuf)
		{
 			openI1D3calMtx calMtx;
			if(probeI1D3.calMtxFromEDRdata(mEDFfileBuf, mEDFfileLen, calMtx, isActive))
			{
				if(fileName) delete[] fileName;
				cout << "Error: " <<  probeI1D3.getErrMsg() << endl;
				exit(-1);
			}

			if(!probeI1D3.initCalibration(&calMtx, isActive))
			{
				if(fileName) delete[] fileName;
				cout << "Error: " <<  probeI1D3.getErrMsg() << endl;
				exit(-1);
			}
		}
		else
		{
			if(!probeI1D3.initCalibration(NULL, isActive))
			{
				if(fileName) delete[] fileName;
				cout << "Error: " <<  probeI1D3.getErrMsg() << endl;
				exit(-1);
			}
		}
		
		do
		{
			double X(0), Y(0), Z(0);

			if(!probeI1D3.measure(mType, mMeasurementTime, X, Y,  Z))
			{
				if(fileName) delete[] fileName;
				cout << "Error: " <<  probeI1D3.getErrMsg() << endl;
				exit(-1);
			}

			printf("%lf, %lf, %lf\n", X, Y, Z);
		}
		while(mMeasureContinuously);
	}
	else if(led)
	{
		if(!probeI1D3.unLock())
		{
			if(fileName) delete[] fileName;
			cout << "Error: " <<  probeI1D3.getErrMsg() << endl;
			exit(-1);
		}

		unsigned char* eBuf = new unsigned char[8192];
		memset(eBuf, 0x00, 8192);
		probeI1D3.readInternalEeprom(eBuf);
		probeI1D3.initFromInternalEeprom(eBuf);
		probeI1D3.readExternalEeprom(eBuf);
		probeI1D3.initFromExternalEeprom(eBuf);
		delete[] eBuf;

		if(ledOn) probeI1D3.ledOn();
		else      probeI1D3.ledOff();
	}
	else if(dif)
	{
		if(!probeI1D3.unLock())
		{
			if(fileName) delete[] fileName;
			cout << "Error: " <<  probeI1D3.getErrMsg() << endl;
			exit(-1);
		}

		unsigned char* eBuf = new unsigned char[8192];
		memset(eBuf, 0x00, 8192);
		probeI1D3.readInternalEeprom(eBuf);
		probeI1D3.initFromInternalEeprom(eBuf);
		probeI1D3.readExternalEeprom(eBuf);
		probeI1D3.initFromExternalEeprom(eBuf);
		delete[] eBuf;

		bool isActive;
		probeI1D3.isDiffuserInPosition(isActive);

		if(isActive) cout << "Diffuser is active " << endl;
		else         cout << "Diffuser is not active " << endl;
	}

	if(fileName)	 delete[] fileName;
	if(mEDFfilename) delete[] mEDFfilename;
	if(mEDFfileBuf)	 delete[] mEDFfileBuf;


	return 0;
}
