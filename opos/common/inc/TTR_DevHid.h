
#if !defined(__TTRL_DEVICE_HID_OBJECT__HEADER_20091217__)
#define __TTRL_DEVICE_HID_OBJECT__HEADER_20091217__

///////////////////////////////////////////////
//////////////////////////////////////////////	
#include "TTR_def.h"
#include "TTR_tchar.h"
#include "windows.h"
#include "TTR_LogGen.h"
#include <list>
#include <iostream>
#include "TTR_Dev.h"

extern "C"
{
#include <hidclass.h>
#include <hidsdi.h>
#include <setupapi.h>
#include <dbt.h>
}

using namespace std;


class CTTR_DeviceHid : public CTTR_Device
{
	/////////////////////
	//internal type part 
public:

protected:

	struct HIDINFO{
		USHORT VID;//vender ID
		USHORT PID;//product ID
		TCHAR sPath[_MAX_PATH];//device path
		HIDP_CAPS HidCaps;//HID capabilities
	};

	//device fine mode..
	enum ENUM_FindMode{
		FM_ALL = -1,		//all find usb device
		FM_VID = 0,			//find matched VID device.
		FM_VID_PID,			//find matched VID & PID device.
		FM_VID_PID_INF,		//find matched VID & PID & interface device.
		FM_VID_PID_REV,		//find matched VID & PID & revision device.
		FM_VID_PID_INF_REV	//find matched VID & PID * interface & revision device.
	};

private:


	///////////////
	//method part
public:
	CTTR_DeviceHid();
	CTTR_DeviceHid(CTTR_LogSystem *pLog);
	virtual ~CTTR_DeviceHid();
	//

	//Get device list
	//return list and the number of device.
	virtual INT GetDevList( list<_tstring> & listDevPath );

	virtual BOOL	Open( LPCTSTR szDevPath, BOOL bOverlap = FALSE )
	{
		BOOL bResult =  CTTR_Device::Open( szDevPath, bOverlap );

		if( bResult ){
			GetCapabilities( &m_Capabilities, m_hDev );
		}

		return bResult;
	}


	virtual BOOL	Open( _tstring & szDevPath, BOOL bOverlap = FALSE )
	{
		BOOL bResult =  CTTR_Device::Open( szDevPath, bOverlap );

		if( bResult ){
			GetCapabilities( &m_Capabilities, m_hDev );
		}

		return bResult;
	}


	virtual BOOL	Write(
			LPCVOID lpBuffer,
			DWORD nNumberOfBytesToWrite,
			LPDWORD lpNumberOfBytesWritten = NULL,
			LPOVERLAPPED lpOverlapped = NULL
		);

	virtual BOOL	Read(
			LPVOID lpBuffer,
			DWORD nNumberOfBytesToRead,
			LPDWORD lpNumberOfBytesRead,
			LPOVERLAPPED lpOverlapped = NULL
		);

	virtual BOOL	Write(
			LPCVOID lpBuffer,
			DWORD nNumberOfBytesToWrite,
			LPDWORD lpNumberOfBytesWritten = NULL,
			HANDLE hCancelEvent = NULL,
			UINT32 dwTimeOut = 0
		);

	virtual BOOL	Read(
			LPVOID lpBuffer,
			DWORD nNumberOfBytesToRead,
			LPDWORD lpNumberOfBytesRead,
			HANDLE hCancelEvent = NULL,
			UINT32 dwTimeOut = 0
		);

	//the first byte of lpBuffer is report ID.
	//If the top-level collection includes report IDs,
	//the caller must set the first byte of the buffer to a nonzero report ID;
	//otherwise the caller must set the first byte to zero.
	virtual BOOL	WriteFeatureReport(
			HANDLE hHID,				//handle to HID device
			LPVOID ReportBuffer,		// data buffer
			ULONG  ReportBufferLength	// number of bytes written,excluding its report ID
		);

	//the first byte of lpBuffer is report ID.
	//If the top-level collection includes report IDs,
	//the caller must set the first byte of the buffer to a nonzero report ID;
	//otherwise the caller must set the first byte to zero.
	virtual BOOL	ReadFeatureReport(
			HANDLE hHID,				//handle to HID device
			LPVOID ReportBuffer ,		// data buffer
			ULONG  ReportBufferLength	// number of bytes read, excluding its report ID
		);

	//this function is called on WM_DEVICECHANGE message handler
	static CTTR_DeviceHid::ENUM_DevEvent OnDeviceChangeMsgHandler( 
		UINT nEventType,
		DWORD_PTR dwData,
		CONST GUID & ClassGuid,
		INT nVID,
		INT nPID = -1,
		INT nRev = -1,
		INT nInf = -1
		);

	//this function is called on WM_DEVICECHANGE message handler
	static UINT OnDeviceChangeMsgHandler( 
		UINT nEventType,
		DWORD_PTR dwData,
		INT nDev,
		CONST GUID & ClassGuid,
		CONST INT *pnVID,
		CONST INT *pnPID = NULL,
		CONST INT *pnRev = NULL,
		CONST INT *pnInf = NULL,
		HDEVNOTIFY *phNotify = NULL
		);


	/////////////////////////
	//the first in this class

	static INT  GetNumInputBuffers( HANDLE hHID );

	INT  GetNumInputBuffers();
	
	static BOOL SetNumInputBuffers(	HANDLE hHID,INT nNumberBuffers );

	BOOL SetNumInputBuffers( INT nNumberBuffers );

	static BOOL IsWantUSBDevice(
		PTCHAR psDevicePath,
		INT nVID,
		INT nPID = -1,
		INT nInf = -1,
		INT nRev = -1
		);


protected:
	//remove device list
	VOID RemoveDevList();

	VOID RefreshDevList();


	INT Find( 
		list<HIDINFO*> & listDev,
		INT nVID = FIND_ALL_DEVICES,
		INT nPID = FIND_ALL_DEVICES,
		INT nInterface = FIND_ALL_DEVICES,
		INT nRev = FIND_ALL_DEVICES
		);

	INT GetPidOnDevicePath( LPCTSTR psDevPath );
	INT GetVidOnDevicePath( LPCTSTR psDevPath );
	INT GetInterfaceNumberOnDevicePath( LPCTSTR psDevPath );

	BOOL GetCapabilities( PHIDP_CAPS pCapabilities,HANDLE hDeviceHandle );

	static BOOL ParsingUSBDevicePath(
		PTCHAR pDevicePath,
		PTCHAR psVID,
		PTCHAR psPID = NULL,
		PTCHAR psInf = NULL,
		PTCHAR psRev = NULL
		);


private:


	////////////////////////
	//member variables part
public:
protected:
	list<HIDINFO*> m_listHids;	//Hid device list

	HIDP_CAPS m_Capabilities;

private:

};
/////////////////////////////////////////

#endif	//__TTRL_DEVICE_HID_OBJECT__HEADER_20091217__
