#include "stdafx.h"
#include "TTR_Dev.h"

///////////////////////
//static member initial
INT CTTR_Device::m_nMaxDevCnt = 30;	// on searching, maximum counter number

////{a5dcbf10-6530-11d2-901f-00c04fb951ed}
CONST GUID CTTR_Device::CLASS_GUID_USB = { 0xa5dcbf10, 0x6530, 0x11d2, { 0x90, 0x1f, 0x00, 0xc0, 0x4f, 0xb9, 0x51, 0xed } };


CTTR_Device::CTTR_Device()
{
	InitializeCriticalSectionAndSpinCount( &m_CSSafe,TTR_DEF_SPIN_COUNT );

	m_sPath.clear();	//device symbloic link

	memset( &m_DevGuid, 0, sizeof(GUID) );	//device Guid

	m_pLog = NULL;

	m_bEnableLog = FALSE;

	m_bOverlap = FALSE;

	m_hDev = INVALID_HANDLE_VALUE;

	memset( &m_OverLap, 0, sizeof(OVERLAPPED) );
}

CTTR_Device::CTTR_Device(CTTR_LogSystem *pLog)
{
	CTTR_Device();

	m_pLog = pLog;
}

CTTR_Device::~CTTR_Device()
{
	DeleteCriticalSection(&m_CSSafe);//remove critical section

	Close();
}

//Get device list
//return list and the number of device.
INT CTTR_Device::GetDevList( list<_tstring> & listDevPath )
{
	INT nDev = CTTR_Device::NOT_IMPLEMENT;	//Error

	return nDev;
}


BOOL	CTTR_Device::Open( _tstring & szDevPath, BOOL bOverlap /*= FALSE*/ )
{
	return Open( (LPCTSTR)szDevPath.c_str(), bOverlap );
}

BOOL	CTTR_Device::Open( LPCTSTR szDevPath,BOOL bOverlap /*=FALSE*/ )
{
	HANDLE hDev = NULL;
	DWORD dwFlagsAndAttributes;

	if( szDevPath == NULL )
		return FALSE;
	//
	
	if( bOverlap )
		dwFlagsAndAttributes = FILE_FLAG_OVERLAPPED;
	else
		dwFlagsAndAttributes = 0;
	//
	
	hDev = CreateFile(
		szDevPath, 
		GENERIC_READ | GENERIC_WRITE, 
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		(LPSECURITY_ATTRIBUTES)NULL, 
		OPEN_EXISTING, 
		dwFlagsAndAttributes, 
		NULL
		);

	if( hDev==INVALID_HANDLE_VALUE )
		return FALSE;

	//

	if( bOverlap ){
		//m_OverLap

		memset( &m_OverLap, 0, sizeof(OVERLAPPED) );

		m_OverLap.hEvent = CreateEvent( NULL,TRUE,FALSE,NULL );

		if( m_OverLap.hEvent == NULL ){
			CloseHandle( hDev );
			return FALSE;
		}

	}

	m_bOverlap = bOverlap;
	m_hDev = hDev;

	m_sPath = szDevPath;
		
	return TRUE;

}

////////////////
//STATIC MEMBER
HANDLE	CTTR_Device::Open( LPCTSTR szDevPath, DWORD dwFlagsAndAttributes )
{
	HANDLE hDev = NULL;

	if( szDevPath == NULL )
		return hDev;
	//
	hDev = CreateFile(
		szDevPath, 
		GENERIC_READ | GENERIC_WRITE, 
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		(LPSECURITY_ATTRIBUTES)NULL, 
		OPEN_EXISTING, 
		dwFlagsAndAttributes, 
		NULL
		);

	if( hDev==INVALID_HANDLE_VALUE )
		return hDev;

	//
	return hDev;

}

VOID	CTTR_Device::Close()
{
	if( m_hDev != INVALID_HANDLE_VALUE ){
		CloseHandle( m_hDev );

		m_hDev = INVALID_HANDLE_VALUE;

		if( m_bOverlap ){
			if( m_OverLap.hEvent ){
				CloseHandle( m_OverLap.hEvent );
				memset( &m_OverLap, 0, sizeof(OVERLAPPED) );
			}
		}
		
	}
}

////////////////
//STATIC MEMBER
VOID	CTTR_Device::Close( HANDLE hDev )
{
	if( hDev != INVALID_HANDLE_VALUE )
		CloseHandle( hDev );
}


BOOL	CTTR_Device::Write(
			LPCVOID lpBuffer,
			DWORD nNumberOfBytesToWrite,
			LPDWORD lpNumberOfBytesWritten /*=NULL*/,
			LPOVERLAPPED lpOverlapped /*=NULL*/
		)
{
	BOOL bResult = TRUE;

	if( lpBuffer==NULL )
		return FALSE;

	if( m_bOverlap ){

		if( lpOverlapped == NULL ){
			lpOverlapped = &m_OverLap;
		}

	}
	else{
		lpOverlapped = NULL;//forcing
	}

	bResult = WriteFile( m_hDev,lpBuffer,nNumberOfBytesToWrite,lpNumberOfBytesWritten,lpOverlapped );

	return bResult;
}


BOOL	CTTR_Device::Read(
			LPVOID lpBuffer,
			DWORD nNumberOfBytesToRead,
			LPDWORD lpNumberOfBytesRead,
			LPOVERLAPPED lpOverlapped /*=NULL*/
		)
{
	BOOL bResult = TRUE;

	if( lpBuffer==NULL )
		return FALSE;

	if( m_bOverlap ){

		if( lpOverlapped == NULL ){
			lpOverlapped = &m_OverLap;
		}

	}
	else{
		lpOverlapped = NULL;
	}

	bResult = ReadFile( m_hDev,lpBuffer,nNumberOfBytesToRead,lpNumberOfBytesRead,lpOverlapped );

	return bResult;
}


BOOL	CTTR_Device::Write(
			LPCVOID lpBuffer,
			DWORD nNumberOfBytesToWrite,
			LPDWORD lpNumberOfBytesWritten/*= NULL*/,
			HANDLE hCancelEvent /*= NULL*/,
			UINT32 dwTimeOut /*= 0*/
		)
{
	BOOL bResult = TRUE;
	HANDLE hEvent;
	HANDLE hEvents[2];
	DWORD dwResult;
	DWORD NumberOfBytesWritten = 0;
	OVERLAPPED OverLap;

	if( !m_bOverlap ){
		return FALSE;
	}

	if( lpNumberOfBytesWritten == NULL ){
		lpNumberOfBytesWritten = &NumberOfBytesWritten;
	}

	//reset overlap structure
	hEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
	if( hEvent == NULL )
		return FALSE;

	memset( &OverLap,0,sizeof(OverLap) );
	OverLap.hEvent = hEvent;

	hEvents[0] = hEvent;	hEvents[1] = hCancelEvent;

	bResult = WriteFile( m_hDev,lpBuffer,nNumberOfBytesToWrite,lpNumberOfBytesWritten,&OverLap );

	if( !bResult ){

		dwResult = GetLastError();
		if(  dwResult == ERROR_IO_PENDING ){
			//

			if( hCancelEvent ){
				dwResult = WaitForMultipleObjects( 2, hEvents, FALSE, dwTimeOut );

				switch( dwResult ){
					case WAIT_OBJECT_0://IO event
						bResult = GetOverlappedResult( m_hDev, &OverLap, lpNumberOfBytesWritten, TRUE );
						break;
					case WAIT_OBJECT_0+1://cancel event
						CancelIo( m_hDev );	//cancel peding io
						bResult = FALSE;
						break;
					default:
						CancelIo( m_hDev );	//cancel peding io
						bResult = FALSE;
						break;

				}//end switch
			}
			else{
				dwResult = WaitForSingleObject( OverLap.hEvent, dwTimeOut );

				switch( dwResult ){
					case WAIT_OBJECT_0://IO event
						bResult = GetOverlappedResult( m_hDev, &OverLap, lpNumberOfBytesWritten, TRUE );
						break;
					default:
						CancelIo( m_hDev );	//cancel peding io
						bResult = FALSE;
						break;

				}//end switch
			}

		}
		else{
			//error
			bResult = FALSE;
		}

	}

	//
	CloseHandle( OverLap.hEvent );
	//
	return bResult;
}

BOOL	CTTR_Device::Read(
			LPVOID lpBuffer,
			DWORD nNumberOfBytesToRead,
			LPDWORD lpNumberOfBytesRead,
			HANDLE hCancelEvent /*= NULL*/,
			UINT32 dwTimeOut /*= 0*/
		)
{
	BOOL bResult = TRUE;
	HANDLE hEvent;
	HANDLE hEvents[2];
	DWORD dwResult;
	DWORD NumberOfBytesRead = 0;
	OVERLAPPED OverLap;

	if( !m_bOverlap ){
		return FALSE;
	}

	if( lpNumberOfBytesRead == NULL ){
		lpNumberOfBytesRead = &NumberOfBytesRead;
	}

	//reset overlap structure
	hEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
	if( hEvent == NULL )
		return FALSE;

	memset( &OverLap,0,sizeof(OverLap) );
	OverLap.hEvent = hEvent;

	hEvents[0] = hEvent;	hEvents[1] = hCancelEvent;

	bResult = ReadFile( m_hDev,lpBuffer,nNumberOfBytesToRead,lpNumberOfBytesRead,&OverLap );


	if( !bResult ){

		dwResult = GetLastError();
		if(  dwResult == ERROR_IO_PENDING ){
			//

			if( hCancelEvent ){
				dwResult = WaitForMultipleObjects( 2, hEvents, FALSE, dwTimeOut );

				switch( dwResult ){
					case WAIT_OBJECT_0://IO event
						bResult = GetOverlappedResult( m_hDev, &OverLap, lpNumberOfBytesRead, TRUE );
						break;
					case WAIT_OBJECT_0+1://cancel event
						CancelIo( m_hDev );	//cancel peding io
						bResult = FALSE;
						break;
					default:
						CancelIo( m_hDev );	//cancel peding io
						bResult = FALSE;
						break;

				}//end switch
			}
			else{
				dwResult = WaitForSingleObject( OverLap.hEvent, dwTimeOut );

				switch( dwResult ){
					case WAIT_OBJECT_0://IO event
						bResult = GetOverlappedResult( m_hDev, &OverLap, lpNumberOfBytesRead, TRUE );
						break;
					default:
						CancelIo( m_hDev );	//cancel peding io
						bResult = FALSE;
						break;

				}//end switch
			}

		}
		else{
			//error
			bResult = FALSE;
		}

	}

	//
	CloseHandle( OverLap.hEvent );
	//
	return bResult;
}

//register user device for device-change-notification
HDEVNOTIFY CTTR_Device::RegisterEventDevice( HANDLE hTargetWnd,CONST GUID & InterfaceClassGuid )
{
	////////////////////////////////////
	// interface class.
	HDEVNOTIFY diNotifyHandle;
	DEV_BROADCAST_DEVICEINTERFACE broadcastInterface;

	ZeroMemory(&broadcastInterface, sizeof(broadcastInterface));

	broadcastInterface.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
	broadcastInterface.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
	broadcastInterface.dbcc_classguid = InterfaceClassGuid;

	
	diNotifyHandle = RegisterDeviceNotification(
		hTargetWnd,
		&broadcastInterface,
		DEVICE_NOTIFY_WINDOW_HANDLE
		);

	return diNotifyHandle;
}

//register user device-handle for device-change-notification
HDEVNOTIFY CTTR_Device::RegisterEventDeviceHandle( HANDLE hTargetWnd,HANDLE hDev )
{
	////////////////////////////////////
	// interface class.
	HDEVNOTIFY diNotifyHandle;
	DEV_BROADCAST_HANDLE filter;

	ZeroMemory(&filter, sizeof(DEV_BROADCAST_HANDLE));

	filter.dbch_size = sizeof(DEV_BROADCAST_HANDLE);
	filter.dbch_devicetype = DBT_DEVTYP_HANDLE;
	filter.dbch_handle = hDev;
	//filter.dbch_hdevnotify = hDevnotify;

	diNotifyHandle = RegisterDeviceNotification(
		hTargetWnd,
		&filter,
		DEVICE_NOTIFY_WINDOW_HANDLE
		);

	DWORD dwResult = GetLastError();
	return diNotifyHandle;
}

//unregister device of RegisterEventDevice()
BOOL CTTR_Device::UnRegisterEventDevice( HDEVNOTIFY hNotify )
{
	return UnregisterDeviceNotification( hNotify );

}



//this function is called on WM_DEVICECHANGE message handler
CTTR_Device::ENUM_DevEvent CTTR_Device::OnDeviceChangeMsgHandler( 
	UINT nEventType,
	DWORD dwData,
	CONST GUID & ClassGuid
	)
{
	PDEV_BROADCAST_HDR broadcastHdr;
	PDEV_BROADCAST_DEVICEINTERFACE broadcastInterface;
	ENUM_DevEvent nReturn=DE_NO_NEED_EVENT;

	switch(nEventType){
		case DBT_DEVICEARRIVAL:
			broadcastHdr = (PDEV_BROADCAST_HDR) dwData;

			if( broadcastHdr->dbch_devicetype==DBT_DEVTYP_DEVICEINTERFACE ) {
				broadcastInterface = (PDEV_BROADCAST_DEVICEINTERFACE) dwData;

				if( broadcastInterface->dbcc_classguid==ClassGuid ){

					nReturn=DE_PLUGIN_WANTED_DEV;
				}
				else{
					nReturn=DE_PLUGIN_THE_OTHER_DEV;
				}
			}
			break;
		case DBT_DEVICEREMOVECOMPLETE:
			broadcastHdr = (PDEV_BROADCAST_HDR) dwData;
			if( broadcastHdr->dbch_devicetype==DBT_DEVTYP_DEVICEINTERFACE ){

				broadcastInterface = (PDEV_BROADCAST_DEVICEINTERFACE) dwData;

				if( broadcastInterface->dbcc_classguid==ClassGuid ){

					nReturn=DE_PLUGOUT_WANTED_DEV;
				}
				else{
					nReturn=DE_PLUGOUT_THE_OTHER_DEV;
				}
					
			}
			break;
		case DBT_DEVICEQUERYREMOVE:break;//Permission to remove a device is requested. Any application can deny this request and cancel the removal.
		case DBT_DEVICEQUERYREMOVEFAILED:break;//Request to remove a device has been canceled.
		case DBT_DEVICEREMOVEPENDING:break;//Device is about to be removed. Cannot be denied.
		case DBT_DEVICETYPESPECIFIC:break;//Device-specific event.
		case DBT_CONFIGCHANGED:break;//Current configuration has changed.
		case DBT_DEVNODES_CHANGED:break;//Device node has changed. 
		default:break;
	}//end switch

	return nReturn;
}


//this function is called on WM_DEVICECHANGE message handler . return will be removed handle.
CTTR_Device::ENUM_DevEvent CTTR_Device::OnDeviceChangeMsgHandler( 
	UINT nEventType,
	DWORD dwData,
	HANDLE hWantedDev
	)
{
	PDEV_BROADCAST_HDR broadcastHdr;
	ENUM_DevEvent nReturn=DE_NO_NEED_EVENT;
	PDEV_BROADCAST_HANDLE pDBHandle;

	switch(nEventType){
		case DBT_DEVICEARRIVAL:
			broadcastHdr = (PDEV_BROADCAST_HDR) dwData;

			if( broadcastHdr->dbch_devicetype==DBT_DEVTYP_HANDLE ) {

				pDBHandle = (PDEV_BROADCAST_HANDLE) broadcastHdr;

				if( broadcastHdr->dbch_devicetype==DBT_DEVTYP_HANDLE ){

					if( pDBHandle->dbch_handle==hWantedDev ){

						nReturn=DE_PLUGIN_WANTED_DEV;
					}
					else
						nReturn=DE_PLUGIN_THE_OTHER_DEV;
				}
			}
			break;
		case DBT_DEVICEREMOVECOMPLETE:
			broadcastHdr = (PDEV_BROADCAST_HDR) dwData;

			if( broadcastHdr->dbch_devicetype==DBT_DEVTYP_HANDLE ){

				pDBHandle = (PDEV_BROADCAST_HANDLE) broadcastHdr;

				if( pDBHandle->dbch_handle==hWantedDev ){

					nReturn=DE_PLUGOUT_WANTED_DEV;
				}
				else{
					nReturn=DE_PLUGOUT_THE_OTHER_DEV;
				}
			}
			break;
		case DBT_DEVICEQUERYREMOVE:break;//Permission to remove a device is requested. Any application can deny this request and cancel the removal.
		case DBT_DEVICEQUERYREMOVEFAILED:break;//Request to remove a device has been canceled.
		case DBT_DEVICEREMOVEPENDING:break;//Device is about to be removed. Cannot be denied.
		case DBT_DEVICETYPESPECIFIC:break;//Device-specific event.
		case DBT_CONFIGCHANGED:break;//Current configuration has changed.
		case DBT_DEVNODES_CHANGED:break;//Device node has changed. 
		default:break;
	}//end switch

	return nReturn;
}


//this function is called on WM_DEVICECHANGE message handler . return will be removed handle.
CTTR_Device::ENUM_DevEvent CTTR_Device::OnDeviceChangeMsgHandler( 
	UINT nEventType,
	DWORD dwData,
	INT nDev,
	CONST HANDLE *hWantedDevs
	)
{
	PDEV_BROADCAST_HDR broadcastHdr;
	ENUM_DevEvent nReturn=DE_NO_NEED_EVENT;
	PDEV_BROADCAST_HANDLE pDBHandle;
	INT i;

	if( nDev<= 0 )
		return nReturn;

	switch(nEventType){
		case DBT_DEVICEARRIVAL:
			broadcastHdr = (PDEV_BROADCAST_HDR) dwData;

			if( broadcastHdr->dbch_devicetype==DBT_DEVTYP_HANDLE ) {

				pDBHandle = (PDEV_BROADCAST_HANDLE) broadcastHdr;

				if( broadcastHdr->dbch_devicetype==DBT_DEVTYP_HANDLE ){

					nReturn=DE_PLUGIN_THE_OTHER_DEV;

					for( i=0; i<nDev; i++ ){

						if( pDBHandle->dbch_handle == hWantedDevs[i] ){

							nReturn=DE_PLUGIN_WANTED_DEV;
						}
					}//end for
						
				}
			}
			break;
		case DBT_DEVICEREMOVECOMPLETE:
			broadcastHdr = (PDEV_BROADCAST_HDR) dwData;

			if( broadcastHdr->dbch_devicetype==DBT_DEVTYP_HANDLE ){

				pDBHandle = (PDEV_BROADCAST_HANDLE) broadcastHdr;

				nReturn=DE_PLUGOUT_THE_OTHER_DEV;

				for( i=0; i<nDev; i++ ){

					if( pDBHandle->dbch_handle==hWantedDevs[i] ){

						nReturn=DE_PLUGOUT_WANTED_DEV;
					}

				}//end for

			}
			break;
		case DBT_DEVICEQUERYREMOVE:break;//Permission to remove a device is requested. Any application can deny this request and cancel the removal.
		case DBT_DEVICEQUERYREMOVEFAILED:break;//Request to remove a device has been canceled.
		case DBT_DEVICEREMOVEPENDING:break;//Device is about to be removed. Cannot be denied.
		case DBT_DEVICETYPESPECIFIC:break;//Device-specific event.
		case DBT_CONFIGCHANGED:break;//Current configuration has changed.
		case DBT_DEVNODES_CHANGED:break;//Device node has changed. 
		default:break;
	}//end switch

	return nReturn;
}
