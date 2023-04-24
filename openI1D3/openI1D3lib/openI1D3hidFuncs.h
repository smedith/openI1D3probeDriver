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


#ifndef _openI1D3hidFuncs_h
#define _openI1D3hidFuncs_h

#include <windows.h>
#include <setupapi.h>

class openI1D3hidIdevice;

HINSTANCE		openI1D3loadDLLfuncs();
unsigned int	openI1D3getHIDproductID(openI1D3hidIdevice* hidDev);
openI1D3hidIdevice*		openI1D3findHIDdevice();
bool			openI1D3openHIDdevice (openI1D3hidIdevice* dev);
void			openI1D3closeHIDdevice(openI1D3hidIdevice* dev);
int				openI1D3readHIDdevice (openI1D3hidIdevice* dev, unsigned char* rbuf, int numToRead,  double timeout);
int				openI1D3writeHIDdevice(openI1D3hidIdevice* dev, unsigned char* wbuf, int numToWrite, double timeout = 1.0);


/* Declartions to enable HID access without using the DDK */
#define DIDD_BUFSIZE sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA) + (sizeof(TCHAR)*MAX_PATH)

class openI1D3hidIdevice
{
	public:
					openI1D3hidIdevice():dpath(0), fh(0) {};
				   ~openI1D3hidIdevice(){ if(dpath) delete[] dpath;};

	char*			dpath;
	HANDLE			fh;
	OVERLAPPED		ols;
	unsigned int	ProductID;
};

typedef struct _HIDD_ATTRIBUTES
{
	ULONG	Size;
	USHORT	VendorID;
	USHORT	ProductID;
	USHORT	VersionNumber;
} HIDD_ATTRIBUTES, *PHIDD_ATTRIBUTES;

typedef void (__stdcall *FP_HidD_GetHidGuid)   (LPGUID HidGuid);
typedef BOOL (__stdcall *FP_HidD_GetAttributes)(HANDLE , PHIDD_ATTRIBUTES Attributes);
FP_HidD_GetHidGuid    HidD_GetHidGuid;
FP_HidD_GetAttributes HidD_GetAttributes;

HINSTANCE openI1D3loadDLLfuncs()
{
	static HINSTANCE lib(0);

	if(!lib)
	{
		lib = LoadLibrary("HID");
		if(lib)
		{
			HidD_GetHidGuid    = (FP_HidD_GetHidGuid)    GetProcAddress(lib, "HidD_GetHidGuid");
			HidD_GetAttributes = (FP_HidD_GetAttributes) GetProcAddress(lib, "HidD_GetAttributes");
		}

		if((HidD_GetHidGuid == 0) || (HidD_GetAttributes == 0)) lib = 0;
	}

	return lib;
}

unsigned int openI1D3getHIDproductID(openI1D3hidIdevice* hidDev)
{
	if(hidDev) return hidDev->ProductID;
	return 0;
}

openI1D3hidIdevice* openI1D3findHIDdevice()
{
// Get the GUID for HIDClass devices
	GUID HidGuid;
	HidD_GetHidGuid(&HidGuid);

	// Get the device information for all devices of the HID class
	HDEVINFO hdinfo;
	hdinfo = SetupDiGetClassDevs(&HidGuid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE); 
	if(hdinfo == INVALID_HANDLE_VALUE) return 0;

	/* Get each devices interface data in turn */
	SP_DEVICE_INTERFACE_DATA diData;
	diData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

	PSP_DEVICE_INTERFACE_DETAIL_DATA pdiDataDetail;
	char* diddBuf[DIDD_BUFSIZE];
	pdiDataDetail = (PSP_DEVICE_INTERFACE_DETAIL_DATA)diddBuf;  
	pdiDataDetail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

	SP_DEVINFO_DATA dinfoData;
	dinfoData.cbSize = sizeof(SP_DEVINFO_DATA);

	openI1D3hidIdevice* hidDev(0);

	for(unsigned int c(0); ; ++c)
	{
		if(SetupDiEnumDeviceInterfaces(hdinfo, NULL, &HidGuid, c, &diData) == 0)
		{
			if (GetLastError() == ERROR_NO_MORE_ITEMS) break;

			return 0;
		}

		if(SetupDiGetDeviceInterfaceDetail(hdinfo, &diData, pdiDataDetail, DIDD_BUFSIZE, NULL, &dinfoData) == 0)
		{
			return 0;
		}

		// Extract the vid and pid from the device path
		unsigned int VendorID(0);
		unsigned int ProductID(0);
	
		char *cPtr;
		char cBuf[20];

		if((cPtr = strchr(pdiDataDetail->DevicePath, 'v')) == NULL) continue;
		if(strlen(cPtr) < 8) continue;
		if(cPtr[1] != 'i' || cPtr[2] != 'd' || cPtr[3] != '_') continue;
		memcpy(cBuf, cPtr + 4, 4);
		cBuf[4] = 0;
		if(sscanf(cBuf, "%x", &VendorID) != 1) continue;

		if((cPtr = strchr(pdiDataDetail->DevicePath, 'p')) == NULL) continue;
		if(strlen(cPtr) < 8) break;
		if(cPtr[1] != 'i' || cPtr[2] != 'd' || cPtr[3] != '_') continue;
		memcpy(cBuf, cPtr + 4, 4);
		cBuf[4] = 0;
		if(sscanf(cBuf, "%x", &ProductID) != 1) break;

		//Is it an X-Rite i1DisplayPro, ColorMunki Display (HID)
		if((VendorID == 0x0765) && ((ProductID == 0x5020) || (ProductID == 0x5021)))
		{
			hidDev = new openI1D3hidIdevice;
			if(!hidDev) return 0;
			hidDev->dpath = new char[strlen(pdiDataDetail->DevicePath) + 2];
			if(!hidDev->dpath) return 0;
			memset(hidDev->dpath, 0x00, strlen(pdiDataDetail->DevicePath) + 2);

			/* Windows 10 seems to return paths without the leading '\\' */
			if(pdiDataDetail->DevicePath[0] == '\\' &&	pdiDataDetail->DevicePath[1] != '\\') strcpy(hidDev->dpath, "\\");
			strcpy(hidDev->dpath, pdiDataDetail->DevicePath);

			hidDev->ProductID = ProductID;

			break;
		}
	}

	//cleanup hdifo
	if(SetupDiDestroyDeviceInfoList(hdinfo) == 0) return 0;

    return hidDev;
}

bool openI1D3openHIDdevice(openI1D3hidIdevice* dev)
{
	// Open the device
	dev->fh = CreateFile(dev->dpath, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);

	if(dev != INVALID_HANDLE_VALUE)
	{
		memset(&dev->ols,0,sizeof(OVERLAPPED));
		dev->ols.hEvent = CreateEvent(NULL, 0, 0, NULL);
  		if(dev->ols.hEvent == NULL) return false;
		
		return true;
	}

	return false;
}


void openI1D3closeHIDdevice(openI1D3hidIdevice* dev)
{
	if(dev != NULL)
	{
		CloseHandle(dev->ols.hEvent);
		CloseHandle(dev->fh);
	}
}


int	openI1D3readHIDdevice(openI1D3hidIdevice* dev, unsigned char* rbuf,	int numToRead, double timeout)
{
	int numRead(0);

	unsigned char* lBuf;
	lBuf = new unsigned char[numToRead + 1];
	if(!lBuf) return -1;
	memset(lBuf, 0x00, numToRead + 1);

	if (ReadFile(dev->fh, lBuf, numToRead + 1, (LPDWORD)&numRead, &dev->ols) == 0)
	{
		if(GetLastError() != ERROR_IO_PENDING)
		{
			numRead = -1; 
		}
		else
		{
			int res;
			res = WaitForSingleObject(dev->ols.hEvent, (int)(timeout * 1000.0 + 0.5));
			if(res == WAIT_FAILED)
			{
				numRead = -1;
			}
			else if
			(res == WAIT_TIMEOUT)
			{
				CancelIo(dev->fh);
				numRead = -1;
			}
			else
			{
				numRead = (int)dev->ols.InternalHigh;
			}
		}
	}

	if(numRead > 0)
	{
		numRead--;
		memcpy(rbuf, lBuf + 1, numRead);
	}

	delete[] lBuf;

	return numRead;
}


int openI1D3writeHIDdevice(openI1D3hidIdevice* dev,	unsigned char* wbuf, int numToWrite, double timeout)
{
	int numWritten(0);

	unsigned char* lBuf;
	lBuf = new unsigned char[numToWrite + 1];
	if(!lBuf) return -1;
	memset(lBuf, 0x00, numToWrite + 1);
	memcpy(lBuf + 1, wbuf, numToWrite);

	if(WriteFile(dev->fh, lBuf, numToWrite + 1, (LPDWORD)&numWritten, &dev->ols) == 0)
	{ 
		if (GetLastError() != ERROR_IO_PENDING)
		{
			numWritten = -1; 
		}
		else
		{
			int res;
			res = WaitForSingleObject(dev->ols.hEvent, (int)(timeout * 1000.0 + 0.5));
			if (res == WAIT_FAILED)
			{
				numWritten = -1; 
			}
			else if (res == WAIT_TIMEOUT)
			{
				CancelIo(dev->fh);
				numWritten = -1; 
			}
			else
			{
				numWritten = (int)dev->ols.InternalHigh;
			}
		}
	}

	if(numWritten > 0)
	{
		numWritten--;
	}

	delete[] lBuf;

	return numWritten;
}

#endif //_openI1D3hidFuncs_h
