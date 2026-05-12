
#if !defined(__TTRL_DEVICE_OBJECT__HEADER_20091130__)
#define __TTRL_DEVICE_OBJECT__HEADER_20091130__

///////////////////////////////////////////////
//////////////////////////////////////////////	
#include "TTR_def.h"
#include "TTR_tchar.h"
#include "windows.h"
#include "TTR_LogGen.h"
#include <list>
#include <iostream>
#include "Dbt.h"


using namespace std;


class CTTR_Device 
{
	/////////////////////
	//internal type part 
public:
	static CONST INT NOT_IMPLEMENT = -1;

	/*
	enum ENUM_DevEvent{
		DE_PLUGIN_WANTED_DEV					= 1,
		DE_PLUGOUT_WANTED_DEV					= 2,
		DE_PLUGIN_THE_OTHER_DEV					= 3,
		DE_PLUGOUT_THE_OTHER_DEV				= 4,
		DE_WILL_BE_PLUGOUT_WANTED_DEV			= 6,
		DE_WILL_BE_PLUGOUT_THE_OTHER_DEV		= 8,
		DE_NO_NEED_EVENT						= 100
	};
	*/
	enum ENUM_DevEvent{
		DE_PLUGIN_WANTED_DEV					= 100,
		DE_PLUGOUT_WANTED_DEV					= 200,
		DE_PLUGIN_THE_OTHER_DEV					= 300,
		DE_PLUGOUT_THE_OTHER_DEV				= 400,
		DE_WILL_BE_PLUGOUT_WANTED_DEV			= 600,
		DE_WILL_BE_PLUGOUT_THE_OTHER_DEV		= 800,
		DE_PLUGOUT_RETURN_NOTIFY_HANDLE		= 900,
		DE_NO_NEED_EVENT						= 1000
	};


	static CONST GUID CLASS_GUID_USB;			//USB class Guid

protected:
	static CONST INT FIND_ALL_DEVICES = -1;
private:



	///////////////
	//method part
public:
	CTTR_Device();
	CTTR_Device(CTTR_LogSystem *pLog);
	virtual ~CTTR_Device();
	//

	//Get device list
	//return list and the number of device.
	virtual INT GetDevList( list<_tstring> & listDevPath );

	virtual BOOL	Open( LPCTSTR szDevPath, BOOL bOverlap = FALSE );
	virtual BOOL	Open( _tstring & szDevPath, BOOL bOverlap = FALSE );

	virtual VOID	Close();

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


	//dwFlagsAndAttributes is CreateFile' dwFlagsAndAttributes
	static HANDLE	Open( LPCTSTR szDevPath, DWORD dwFlagsAndAttributes );
	static VOID		Close( HANDLE hDev );

	static BOOL	Write(
			HANDLE hDev,
			LPCVOID lpBuffer,
			DWORD nNumberOfBytesToWrite,
			LPDWORD lpNumberOfBytesWritten = NULL,
			LPOVERLAPPED lpOverlapped = NULL
			)
	{
		return WriteFile( hDev,lpBuffer,nNumberOfBytesToWrite,lpNumberOfBytesWritten,lpOverlapped );
	}

	static BOOL	Read(
			HANDLE hDev,
			LPVOID lpBuffer,
			DWORD nNumberOfBytesToRead,
			LPDWORD lpNumberOfBytesRead,
			LPOVERLAPPED lpOverlapped = NULL
			)
	{
			return ReadFile( hDev,lpBuffer,nNumberOfBytesToRead,lpNumberOfBytesRead,lpOverlapped );
	}


	//register user device for device-change-notification
	static HDEVNOTIFY RegisterEventDevice( HANDLE hTargetWnd,CONST GUID & InterfaceClassGuid );

	//register user device-handle for device-change-notification
	static HDEVNOTIFY RegisterEventDeviceHandle( HANDLE hTargetWnd,HANDLE hDev );

	//unregister device of RegisterEventDevice()
	static BOOL UnRegisterEventDevice( HDEVNOTIFY hNotify );

	//this function is called on WM_DEVICECHANGE message handler
	static CTTR_Device::ENUM_DevEvent OnDeviceChangeMsgHandler(
		UINT nEventType,
		DWORD dwData,
		CONST GUID & ClassGuid
		);

	//this function is called on WM_DEVICECHANGE message handler . return will be removed handle.
	static CTTR_Device::ENUM_DevEvent OnDeviceChangeMsgHandler( 
		UINT nEventType,
		DWORD dwData,
		HANDLE hWantedDev
		);

	//this function is called on WM_DEVICECHANGE message handler . return will be removed handle.
	static CTTR_Device::ENUM_DevEvent OnDeviceChangeMsgHandler( 
		UINT nEventType,
		DWORD dwData,
		INT nDev,
		CONST HANDLE *hWantedDevs
		);

	HANDLE GetHandle(){	return m_hDev;	}

protected:
private:

	////////////////////////
	//member variables part
public:
protected:
	CTTR_LogSystem *m_pLog;	//default is NULL , loggoing system pointer
	BOOL m_bEnableLog;		//default is FALSE


	CRITICAL_SECTION m_CSSafe;		//critical section in a processor

	_tstring m_sPath;	//device symbloic link

	GUID m_DevGuid;	//device Guid

	BOOL m_bOverlap;//overlap?

	HANDLE m_hDev;	//device handle

	OVERLAPPED m_OverLap;

	static INT m_nMaxDevCnt;	// on searching, maximum counter number

private:

};
/////////////////////////////////////////

#endif	//__TTRL_DEVICE_OBJECT__HEADER_20091130__
