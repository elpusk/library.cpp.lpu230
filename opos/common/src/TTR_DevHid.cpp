#include "stdafx.h"
#include "TTR_DevHid.h"
#include "TTR_etc.h"

#pragma comment(lib, "Hid.lib")

///////////////////////////
//static member initial


CTTR_DeviceHid::CTTR_DeviceHid()
{
	m_listHids.clear();

	memset( &m_Capabilities, 0 , sizeof(HIDP_CAPS) );
}

CTTR_DeviceHid::CTTR_DeviceHid(CTTR_LogSystem *pLog):CTTR_Device(pLog)
{
	m_listHids.clear();
	memset( &m_Capabilities, 0 , sizeof(HIDP_CAPS) );
}

CTTR_DeviceHid::~CTTR_DeviceHid()
{
	RemoveDevList();
}


//Get device list
//return list and the number of device.
INT CTTR_DeviceHid::GetDevList( list<_tstring> & listDevPath )
{
	INT nDev = 0;	//Error
	_tstring sCurpath;
	HIDINFO *pHidInfo;

	//get current devie list.......
	RefreshDevList();

	listDevPath.begin();

	list<HIDINFO*>::iterator devitera=  m_listHids.begin();

	for( devitera =  m_listHids.begin(); devitera != m_listHids.end(); devitera++ ){

		pHidInfo = *devitera;

		if( pHidInfo ){

			_tstring sCurpath = pHidInfo->sPath;

			listDevPath.push_back( sCurpath );

			nDev++;
		}

	}//end for

	return nDev;
}


//pHidInfos[OUT] : return found device infomation
//return the number of detected devices.
//if nVID==0 && nPID==0 , find all HID deivce 
INT CTTR_DeviceHid::Find( 
						 list<HIDINFO*> & listDev,
						 INT nVID/*=0*/,INT nPID/*=0*/,
						 INT nInterface/*=-1*/,
						 INT nRev/*=-1*/
						 )
{
	HIDD_ATTRIBUTES Attributes;
	DWORD DeviceUsage;
	SP_DEVICE_INTERFACE_DATA devInfoData;
	INT nMemberIndex = 0;
	BOOL lResult;
	GUID HidGuid;
	HIDP_CAPS Capabilities;
	HANDLE hHid = INVALID_HANDLE_VALUE;
	PSP_DEVICE_INTERFACE_DETAIL_DATA pDetailData=NULL;
	HDEVINFO hDevInfo;
	DWORD dwLength=0;
	DWORD dwRequiredSize;
	INT nDetected=0;
	BOOL bDected = FALSE;
	INT i=0;
	INT nCurInterface=0;
	HIDINFO *pHidInfo = NULL;

	list<HIDINFO*>::iterator devitor = listDev.begin();

	CTTR_DeviceHid::ENUM_FindMode nMatchMode = FM_ALL;		//-1 : all devices
							//0 : matching VID.
							//1	: matching VID & PID.
							//2 : matching VID & PID & INTERFACE
							//3 : matching VID & PID & REV
							//4 : matching VID & PID & INTERFACE & REV

	/////////////////////
	//set matching mode.
	if( nVID <= FIND_ALL_DEVICES ){
		nMatchMode = FM_ALL;//-1 : all device
	}
	else if( nPID <= FIND_ALL_DEVICES ){
		nMatchMode = FM_VID;//0 : matching VID.
	}
	else{
		if( nInterface <= FIND_ALL_DEVICES ){

			if( nRev <= FIND_ALL_DEVICES ){
				nMatchMode = FM_VID_PID;//1	: matching VID & PID.
			}
			else{
				nMatchMode = FM_VID_PID_REV;//3 : matching VID & PID & REV
			}
		}
		else{

			if( nRev <= FIND_ALL_DEVICES ){
				nMatchMode = FM_VID_PID_INF;//2 : matching VID & PID & INTERFACE
			}
			else{
				nMatchMode = FM_VID_PID_INF_REV;//4 : matching VID & PID & INTERFACE & REV
			}
		}
	}

	//Get HID GUID
	HidD_GetHidGuid(&HidGuid);	
	
	
	//Get device information set of installed HID-Devices
	hDevInfo=SetupDiGetClassDevs( &HidGuid,NULL,NULL,DIGCF_PRESENT|DIGCF_INTERFACEDEVICE );
	
	devInfoData.cbSize = sizeof(devInfoData);

	//find wanted device.
	nMemberIndex = 0;

	for( nMemberIndex = 0 ; TRUE ;nMemberIndex++ ){
		// 1.
		lResult=SetupDiEnumDeviceInterfaces(
			hDevInfo, 0, &HidGuid, nMemberIndex, &devInfoData
			);

		if( lResult == FALSE ){
			break;//exit for;//NO more device.
		}

		// 2.
		lResult = SetupDiGetDeviceInterfaceDetail(
			hDevInfo, &devInfoData, NULL, 0, &dwLength, NULL
			);

		pDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(dwLength);
		pDetailData -> cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

		lResult = SetupDiGetDeviceInterfaceDetail(
			hDevInfo, &devInfoData, pDetailData, dwLength, &dwRequiredSize, NULL
			);

		if( lResult == FALSE ){
			break;//exit for
		}

		// 3.
		hHid = CreateFile(
			pDetailData->DevicePath,
			0,//GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ|FILE_SHARE_WRITE,
			(LPSECURITY_ATTRIBUTES)NULL,
			OPEN_EXISTING, 0, NULL
			);

		if( hHid == INVALID_HANDLE_VALUE ){
			//Free the memory used by the detailData structure (no longer needed).
			free(pDetailData);
			continue;
		}

		// 4.
		Attributes.Size = sizeof(Attributes);
		Attributes.VendorID = GetVidOnDevicePath( pDetailData->DevicePath );
		Attributes.ProductID = GetPidOnDevicePath( pDetailData->DevicePath );
		Attributes.VersionNumber = 0;
		
		//5.
		bDected = FALSE;

		if( Attributes.VendorID != 0 ){
			switch( nMatchMode ){
				case FM_ALL://-1 : all device
					bDected = TRUE;
					break;
				case FM_VID://0 : matching VID.
					if( Attributes.VendorID == (USHORT)nVID )
						bDected = TRUE;
					break;
				case FM_VID_PID://1	: matching VID & PID.
					if( Attributes.VendorID == (USHORT)nVID 
						&& Attributes.ProductID == (USHORT)nPID
						)
						bDected = TRUE;
					break;
				case FM_VID_PID_INF://2 : matching VID & PID & INTERFACE
					nCurInterface = GetInterfaceNumberOnDevicePath( pDetailData->DevicePath );

					if( Attributes.VendorID == (USHORT)nVID 
						&& Attributes.ProductID == (USHORT)nPID
						&& nCurInterface == nInterface
						)
						bDected = TRUE;
					break;
				case FM_VID_PID_REV://3 : matching VID & PID & REV
					if( Attributes.VendorID == (USHORT)nVID 
						&& Attributes.ProductID == (USHORT)nPID
						&& Attributes.VersionNumber == (USHORT)nRev
						)
						bDected = TRUE;
					break;
				case FM_VID_PID_INF_REV://4 : matching VID & PID & INTERFACE & REV
					nCurInterface = GetInterfaceNumberOnDevicePath( pDetailData->DevicePath );

					if( Attributes.VendorID == (USHORT)nVID 
						&& Attributes.ProductID == (USHORT)nPID
						&& Attributes.VersionNumber == (USHORT)nRev
						&& nCurInterface == nInterface
						)
						bDected = TRUE;
					break;

			}//end switch
		}

		if( bDected ){
			pHidInfo = new HIDINFO();
			
			if( pHidInfo ){//copy path
				_tcscpy( pHidInfo->sPath,pDetailData->DevicePath );
				::memset( &(pHidInfo->HidCaps),0,sizeof(HIDP_CAPS) );
				pHidInfo->VID=Attributes.VendorID;
				pHidInfo->PID=Attributes.ProductID;

				listDev.push_back( pHidInfo );
				nDetected++;
			}
		}//if( bDected )

		CloseHandle(hHid);

		//Free the memory used by the detailData structure (no longer needed).
		free(pDetailData);
	}//end for

	//Free the memory reserved for hDevInfo by SetupDiClassDevs.
	SetupDiDestroyDeviceInfoList(hDevInfo);

	return nDetected;
}


//Get PID on HID-device path
INT CTTR_DeviceHid::GetPidOnDevicePath( LPCTSTR psDevPath )
{
	INT nInterface=0;
	TCHAR *pdest=NULL;
	INT i;
	TCHAR sPid[8];
	LPTSTR psSrc=NULL;


	if( psDevPath==NULL )
		return -1;	//Not found interface.

	psSrc=_tcslwr( _tcsdup( psDevPath ) );

	pdest=_tcsstr( psSrc, _T("pid_") );	//pid_XXXX ==> xxxx hexcimal is product ID.

	if( pdest==NULL ){
		if( psSrc )
			free( psSrc );

		return -1;	//Not found interface.
	}
	//
	
	memset( sPid,0,8*sizeof(TCHAR) );//reset buffer.
	pdest= &pdest[4];	//increase pointer number to interface number position.

	//get interface number string
	for( i=0; i<4; i++ ){
		sPid[i]=pdest[i];
	}//end for

	if( i==0 ){
		if( psSrc )
			free( psSrc );
		return -1;	//Not found interface.
	}

	//convert string to digit.
	nInterface=TTR_ConvertToDigit( (LPCTSTR)sPid, 16);

	if( psSrc )
		free( psSrc );

	return nInterface;
}

//Get VID on HID-device path
INT CTTR_DeviceHid::GetVidOnDevicePath( LPCTSTR psDevPath )
{
	INT nInterface=0;
	TCHAR *pdest=NULL;
	INT i;
	TCHAR sVid[8];
	LPTSTR psSrc=NULL;


	if( psDevPath==NULL )
		return 0;	//Not found interface.

	psSrc=_tcslwr( _tcsdup( psDevPath ) );

	pdest=_tcsstr( psSrc, _T("vid_") );	//vid_XXXX ==> xxxx hexcimal is vender ID.

	if( pdest==NULL ){
		if( psSrc )
			free( psSrc );

		return 0;	//Not found interface.
	}
	//
	
	memset( sVid,0,8*sizeof(TCHAR) );//reset buffer.
	pdest=&pdest[4];	//increase pointer number to interface number position.

	//get interface number string
	for( i=0; i<4; i++ ){
		sVid[i]=pdest[i];
	}//end for

	if( i==0 ){
		if( psSrc )
			free( psSrc );
		return 0;	//Not found interface.
	}

	//convert string to digit.
	nInterface=TTR_ConvertToDigit( (LPCTSTR)sVid, 16);

	if( psSrc )
		free( psSrc );

	return nInterface;
}

//Get interface number on HID-device path
INT CTTR_DeviceHid::GetInterfaceNumberOnDevicePath( LPCTSTR psDevPath )
{
	INT nInterface=0;
	TCHAR *pdest=NULL;
	INT i;
	TCHAR sInterface[5];
	LPTSTR psSrc=NULL;


	if( psDevPath==NULL )
		return -1;	//Not found interface.

	psSrc=_tcslwr( _tcsdup( psDevPath ) );

	pdest=_tcsstr( psSrc, _T("mi_") );	//mi_XX ==> xx is interface number( interface number is started from zero-base )

	if( pdest==NULL ){
		if( psSrc )
			free( psSrc );

		return -1;	//Not found interface.
	}
	//
	
	memset( sInterface,0,5*sizeof(TCHAR) );//reset buffer.
	pdest=pdest+3*sizeof(TCHAR);	//increase pointer number to interface number position.

	//get interface number string
	for( i=0; i<5; i++ ){
		if( pdest[i]<_T('0') || pdest[i]>_T('9') )
			break;//exit for
		//
		sInterface[i]=pdest[i];
	
	}//end for

	if( i==0 ){
		if( psSrc )
			free( psSrc );
		return -1;	//Not found interface.
	}

	//convert string to digit.
	nInterface=TTR_ConvertToDigit( (LPCTSTR)sInterface, 16);

	if( psSrc )
		free( psSrc );

	return nInterface;
}

BOOL CTTR_DeviceHid::GetCapabilities( PHIDP_CAPS pCapabilities,HANDLE hDeviceHandle )
{
	//Get the Capabilities structure for the device.
	PHIDP_PREPARSED_DATA	PreparsedData;
	BOOL bResult;

	/*
	API function: HidD_GetPreparsedData
	Returns: a pointer to a buffer containing the information about the device's capabilities.
	Requires: A handle returned by CreateFile.
	There's no need to access the buffer directly,
	but HidP_GetCaps and other API functions require a pointer to the buffer.
	*/

	bResult=HidD_GetPreparsedData(hDeviceHandle,&PreparsedData);

	if( !bResult )
		return bResult;

	/*
	API function: HidP_GetCaps
	Learn the device's capabilities.
	For standard devices such as joysticks, you can find out the specific
	capabilities of the device.
	For a custom device, the software will probably know what the device is capable of,
	and the call only verifies the information.
	Requires: the pointer to the buffer returned by HidD_GetPreparsedData.
	Returns: a Capabilities structure containing the information.
	*/
	
	if( HidP_GetCaps(PreparsedData,pCapabilities)==HIDP_STATUS_SUCCESS )
		bResult=TRUE;
	else
		bResult=FALSE;

	//the capabilities data is .......
	//pCapabilities->UsagePage : Usage Page
	//pCapabilities->InputReportByteLength : Input Report Byte Length
	//pCapabilities->OutputReportByteLength : Output Report Byte Length
	//pCapabilities->FeatureReportByteLength : Feature Report Byte Length
	//pCapabilities->NumberLinkCollectionNodes : Number of Link Collection Nodes
	//pCapabilities->NumberInputButtonCaps : Number of Input Button Caps
	//pCapabilities->NumberInputValueCaps : Number of InputValue Caps
	//pCapabilities->NumberInputDataIndices : Number of InputData Indices
	//pCapabilities->NumberOutputButtonCaps : Number of Output Button Caps
	//pCapabilities->NumberOutputValueCaps : Number of Output Value Caps
	//pCapabilities->NumberOutputDataIndices : Number of Output Data Indices
	//pCapabilities->NumberFeatureButtonCaps : Number of Feature Button Caps
	//pCapabilities->NumberFeatureValueCaps : Number of Feature Value Caps
	//pCapabilities->NumberFeatureDataIndices : Number of Feature Data Indices
	
	//No need for PreparsedData any more, so free the memory it's using.
	if( !HidD_FreePreparsedData(PreparsedData) )
		bResult=FALSE;
	//
	return bResult;
}


VOID CTTR_DeviceHid::RemoveDevList()
{
	HIDINFO * pHidInfo = NULL;

	list<HIDINFO*>::iterator devitor = m_listHids.begin();

	/////////////////////
	//remove device list.
	for( devitor = m_listHids.begin(); devitor != m_listHids.end(); devitor++ ){

			pHidInfo = *devitor;

			if( pHidInfo )
				delete( pHidInfo );
	}//end for

	m_listHids.clear();
}

VOID CTTR_DeviceHid::RefreshDevList()
{
	RemoveDevList();

	Find( m_listHids );

}

//Retrieves the size of the ring buffer the driver uses to store
//input reports.  The default is 8.
//error return negative value
INT  CTTR_DeviceHid::GetNumInputBuffers( HANDLE hHID )
{
	BOOL bResult;
	ULONG ulNumberBuffers=0; 
	INT nSize;

	bResult=HidD_GetNumInputBuffers( hHID,&ulNumberBuffers );

	if( bResult )
		nSize=(INT)ulNumberBuffers;
	else
		nSize=-1;//indicate error
	//
	return nSize;
}


//Sets the size of the ring buffer the driver uses to store input
//reports
//error return negative value
BOOL CTTR_DeviceHid::SetNumInputBuffers( HANDLE hHID,INT nNumberBuffers )
{
	return HidD_SetNumInputBuffers( hHID,(ULONG)nNumberBuffers );
}

BOOL CTTR_DeviceHid::Write(
		LPCVOID lpBuffer,
		DWORD nNumberOfBytesToWrite,
		LPDWORD lpNumberOfBytesWritten/*=NULL*/,
		LPOVERLAPPED lpOverlapped /*=NULL*/
	)
{
	HidD_FlushQueue( m_hDev );

	BOOL bResult = CTTR_Device::Write( lpBuffer,nNumberOfBytesToWrite,lpNumberOfBytesWritten,lpOverlapped );
	
	return bResult;

}

BOOL CTTR_DeviceHid::Read(
		LPVOID lpBuffer,
		DWORD nNumberOfBytesToRead,
		LPDWORD lpNumberOfBytesRead,
		LPOVERLAPPED lpOverlapped /*=NULL*/
	)
{
	BOOL 	bResult = ::ReadFile( m_hDev,lpBuffer,nNumberOfBytesToRead,lpNumberOfBytesRead,lpOverlapped );

	return bResult;
}

BOOL CTTR_DeviceHid::Write(
		LPCVOID lpBuffer,
		DWORD nNumberOfBytesToWrite,
		LPDWORD lpNumberOfBytesWritten /*=NULL*/,
		HANDLE hCancelEvent /*=NULL*/,
		UINT32 dwTimeOut /*=0*/
	)
{
	HidD_FlushQueue( m_hDev );

	BOOL bResult = CTTR_Device::Write( lpBuffer,nNumberOfBytesToWrite,lpNumberOfBytesWritten,hCancelEvent,dwTimeOut );
	
	return bResult;

}

BOOL CTTR_DeviceHid::Read(
		LPVOID lpBuffer,
		DWORD nNumberOfBytesToRead,
		LPDWORD lpNumberOfBytesRead,
		HANDLE hCancelEvent /*=NULL*/,
		UINT32 dwTimeOut /*=0*/
	)
{
	BOOL bResult = CTTR_Device::Read( lpBuffer,nNumberOfBytesToRead,lpNumberOfBytesRead,hCancelEvent,dwTimeOut );
	//HidD_FlushQueue( m_hDev );
	return bResult;
}




//this function is called on WM_DEVICECHANGE message handler
CTTR_DeviceHid::ENUM_DevEvent CTTR_DeviceHid::OnDeviceChangeMsgHandler(
	UINT nEventType,
	DWORD_PTR dwData,
	CONST GUID & ClassGuid,
	INT nVID,
	INT nPID/*=1*/,
	INT nRev/*=1*/,
	INT nInf/*=1*/
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

					if( IsWantUSBDevice( (LPTSTR)(broadcastInterface->dbcc_name),nVID,nPID,nInf,nRev ) ){
						//"Plugin pinpad device"
						nReturn=DE_PLUGIN_WANTED_DEV;
					}
					else//"USB device(except pinpad)Plugin"
						nReturn=DE_PLUGIN_THE_OTHER_DEV;
				}
				else{
					if( IsWantUSBDevice( (LPTSTR)(broadcastInterface->dbcc_name),nVID,nPID,nInf,nRev ) ){
						//"Plugin pinpad device"
						nReturn=DE_PLUGIN_WANTED_DEV;
					}
					else//"USB device(except pinpad)Plugin"
						nReturn=DE_PLUGIN_THE_OTHER_DEV;
				}
			}
			break;
		case DBT_DEVICEREMOVECOMPLETE:

			broadcastHdr = (PDEV_BROADCAST_HDR) dwData;

			if( broadcastHdr->dbch_devicetype==DBT_DEVTYP_DEVICEINTERFACE ){

				broadcastInterface = (PDEV_BROADCAST_DEVICEINTERFACE) dwData;

				if( IsWantUSBDevice( (LPTSTR)(broadcastInterface->dbcc_name),nVID,nPID,nInf,nRev ) ){
					//"Plugout pinpad device"
					nReturn=DE_PLUGOUT_WANTED_DEV;
				}
				else//"USB device(except pinpad)Plugout"
					nReturn=DE_PLUGOUT_THE_OTHER_DEV;
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



UINT CTTR_DeviceHid::OnDeviceChangeMsgHandler( 
	UINT nEventType,
	DWORD_PTR dwData,
	INT nDev,
	CONST GUID & ClassGuid,
	CONST INT *pnVID,
	CONST INT *pnPID /*=NULL*/,
	CONST INT *pnRev /*=NULL*/,
	CONST INT *pnInf/*=NULL*/,
	HDEVNOTIFY *phNotify /*= NULL*/
	)
{
	PDEV_BROADCAST_HDR broadcastHdr;
	PDEV_BROADCAST_DEVICEINTERFACE broadcastInterface;
	UINT nReturn=DE_NO_NEED_EVENT;
	UINT nOffset = 0;
	INT nVID, nPID, nRev, nInf;
	INT i;

	if( phNotify )
		*phNotify = NULL;
	//
	if( pnVID==NULL )
		return nReturn;

	switch(nEventType){
		case DBT_DEVICEARRIVAL:
			broadcastHdr = (PDEV_BROADCAST_HDR) dwData;

			if( broadcastHdr->dbch_devicetype==DBT_DEVTYP_DEVICEINTERFACE ) {

				broadcastInterface = (PDEV_BROADCAST_DEVICEINTERFACE) dwData;

				if( broadcastInterface->dbcc_classguid==ClassGuid ){


					nReturn=DE_PLUGIN_THE_OTHER_DEV;

					for( i=0; i<nDev; i++ ){

						nVID = pnVID[i];
						//
						if( pnPID )	nPID = pnPID[i];
						else		nPID = -1;
						//
						if( pnRev )	nRev = pnRev[i];
						else		nRev = -1;
						//
						if( pnInf )	nInf = pnInf[i];
						else		nInf = -1;

						//
						if( IsWantUSBDevice( (LPTSTR)(broadcastInterface->dbcc_name),nVID,nPID,nInf,nRev ) ){
							nReturn=DE_PLUGIN_WANTED_DEV;
							nOffset = i;
							break;//exit for
						}

					}//end for
						
				}
				else{

					nReturn=DE_PLUGIN_THE_OTHER_DEV;

					for( i=0; i<nDev; i++ ){

						nVID = pnVID[i];
						//
						if( pnPID )	nPID = pnPID[i];
						else		nPID = -1;
						//
						if( pnRev )	nRev = pnRev[i];
						else		nRev = -1;
						//
						if( pnInf )	nInf = pnInf[i];
						else		nInf = -1;

						//
						if( IsWantUSBDevice( (LPTSTR)(broadcastInterface->dbcc_name),nVID,nPID,nInf,nRev ) ){
							nReturn=DE_PLUGIN_WANTED_DEV;
							nOffset = i;
							break;//exit for
						}

					}//end for
				}
			}
			break;
		case DBT_DEVICEREMOVECOMPLETE:

			broadcastHdr = (PDEV_BROADCAST_HDR) dwData;

			if( broadcastHdr->dbch_devicetype==DBT_DEVTYP_DEVICEINTERFACE ){

				broadcastInterface = (PDEV_BROADCAST_DEVICEINTERFACE) dwData;


				nReturn=DE_PLUGOUT_THE_OTHER_DEV;

				for( i=0; i<nDev; i++ ){
					nVID = pnVID[i];
					//
					if( pnPID )	nPID = pnPID[i];
					else		nPID = -1;
					//
					if( pnRev )	nRev = pnRev[i];
					else		nRev = -1;
					//
					if( pnInf )	nInf = pnInf[i];
					else		nInf = -1;

					if( IsWantUSBDevice( (LPTSTR)(broadcastInterface->dbcc_name),nVID,nPID,nInf,nRev ) ){
						nReturn=DE_PLUGOUT_WANTED_DEV;
						nOffset = i;
						break;//exit for
					}

				}//end for
					
			}
			else if( broadcastHdr->dbch_devicetype==DBT_DEVTYP_HANDLE ){
				//phDev
				PDEV_BROADCAST_HANDLE pDevBroadCastHandle = (PDEV_BROADCAST_HANDLE) dwData;

				if( phNotify ){
					*phNotify = pDevBroadCastHandle->dbch_hdevnotify;
					nReturn = DE_PLUGOUT_RETURN_NOTIFY_HANDLE;
					nOffset = 0;
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

	if( nReturn == DE_PLUGIN_WANTED_DEV || nReturn == DE_PLUGOUT_WANTED_DEV )
		nReturn += nOffset;

	return nReturn;
}




//psInf == NULL, psRev == NULL,  pass interface & revision.
BOOL CTTR_DeviceHid::ParsingUSBDevicePath(
	PTCHAR pDevicePath,
	PTCHAR psVID,
	PTCHAR psPID/*=NULL*/,
	PTCHAR psInf/*=NULL*/,
	PTCHAR psRev/*=NULL*/
	)
{
	TCHAR seps[]=_T("\\#&");
	TCHAR *token;
	INT32 i=0;
	//
	if( pDevicePath==NULL || psVID==NULL )
		return FALSE;

	/* Establish string and get the first token: */
	token = _tcstok( pDevicePath, seps );

	while( token != NULL ){
		/* While there are tokens in "pDevicePath" */
		switch( i ){
			case 1://may be USB device is "USB"
				//Perform a lowercase comparison of strings
				if( _tcsicmp( (LPCTSTR)token,_T("USB") )!=0 ){
					if( _tcsicmp( (LPCTSTR)token,_T("HID") )!=0 )//USB HID ?
						return FALSE;//this device is none-usb device.
				}
				break;
			case 2://may be Vid string(Vid_????)
				if( token[0]!=_T('V') && token[0]!=_T('v') )
					return FALSE;
				if( token[1]!=_T('I') && token[1]!=_T('i') )
					return FALSE;
				if( token[2]!=_T('D') && token[2]!=_T('d') )
					return FALSE;
				if( token[3]!=_T('_') )
					return FALSE;
				//
				if( _tcslen( token )!=8 )
					return FALSE;
				//Copy vender ID
				_tcscpy( psVID,&(token[4]) );

				if( psPID )
					break;
				else
					return TRUE;
			case 3://may be Pid string(Pid_????)
				if( token[0]!=_T('P') && token[0]!=_T('p') )
					return FALSE;
				if( token[1]!=_T('I') && token[1]!=_T('i') )
					return FALSE;
				if( token[2]!=_T('D') && token[2]!=_T('d') )
					return FALSE;
				if( token[3]!=_T('_') )
					return FALSE;
				//
				if( _tcslen( token )!=8 )
					return FALSE;
				//Copy product ID
				_tcscpy( psPID,&(token[4]) );

				if( psRev != NULL || psInf != NULL )
					break;
				else
					return TRUE;
			case 4:
				if( psRev ){
					//may be Rev string(Rev_????)
					if( token[0]!=_T('R') && token[0]!=_T('r') )
						return FALSE;
					if( token[1]!=_T('E') && token[1]!=_T('e') )
						return FALSE;
					if( token[2]!=_T('V') && token[2]!=_T('v') )
						return FALSE;
					if( token[3]!=_T('_') )
						return FALSE;
					//
					if( _tcslen( token )!=8 )
						return FALSE;
					//Copy revision string
					_tcscpy( psRev,&(token[4]) );

					if( psInf )
						break;
					else
						return TRUE;
				}
				else{//none rev & interface. 
					//may be Interface string(Mi_????)
					if( token[0]!=_T('M') && token[0]!=_T('m') )
						return FALSE;
					if( token[1]!=_T('I') && token[1]!=_T('i') )
						return FALSE;
					if( token[2]!=_T('_') )
						return FALSE;
					//
					if( _tcslen( token )!=5 )
						return FALSE;
					//Copy interface string
					_tcscpy( psInf,&(token[3]) );

					return TRUE;
				}
				break;
			case 5:
				//may be Interface string(Mi_????)
				if( token[0]!=_T('M') && token[0]!=_T('m') )
					return FALSE;
				if( token[1]!=_T('I') && token[1]!=_T('i') )
					return FALSE;
				if( token[2]!=_T('_') )
					return FALSE;
				//
				if( _tcslen( token )!=5 )
					return FALSE;
				//Copy interface string
				_tcscpy( psInf,&(token[3]) );

				return TRUE;
			default:
				break;
		}//end switch
		//m_ListReport.AddString( (LPCTSTR)token );
		//m_ListReport.SetCurSel( m_ListReport.GetCount()-1 );
		/* Get next token: */
		token = _tcstok( NULL, seps );
		i++;
	}
	return TRUE;
}

enum WantUSBDeviceMode{

	ModeVidPid,
	ModeVidPidInf,
	ModeVidPidRev,
	ModeVidPidInfRev

};

//checks whether the given VID & PID exsit in the given device path 
// if nPID<0 : No consider PID number.
// if nInf<0 : No consider interface number.
// if nRev<0 : No consider revision number.
BOOL CTTR_DeviceHid::IsWantUSBDevice(
									 PTCHAR psDevicePath,
									 INT nVID,
									 INT nPID /*= -1*/,
									 INT nInf/*= -1*/,
									 INT nRev/*= -1*/  
									 )

{
	TCHAR sVID[8];
	TCHAR sPID[8];
	TCHAR sInf[8];
	TCHAR sRev[8];
	PTCHAR psID;
	PTCHAR pssID[4] = { NULL, NULL, NULL, NULL };
	WORD wID[4] = { 0,0,0,0 };//wID[0] - PID, wID[1] - VID, wID[2] - REV or INF, wID[3] -  REV or INF.
	BYTE *pcID;
	BYTE cItem;
	INT32 i,j;
	TCHAR sDevPath[256];
	BOOL bResult = FALSE;
	enum WantUSBDeviceMode nMode = ModeVidPid;
	INT32 nLen;

	pssID[0] = sVID;	pssID[1] = sPID;

	_tcscpy( sDevPath,psDevicePath );

	//Get VID, PID from Path-string.
	if( nRev<0 ){
		if( nInf<0 ){
			bResult = ParsingUSBDevicePath( sDevPath,(PTCHAR)sVID,(PTCHAR)sPID,NULL,NULL );

			if( bResult ){
					//convert lower case
					_tcslwr( (PTCHAR)sVID );	_tcslwr( (PTCHAR)sPID );
					nMode = ModeVidPid;
			}
		}
		else{
			bResult = ParsingUSBDevicePath( sDevPath,(PTCHAR)sVID,(PTCHAR)sPID,(PTCHAR)sInf,NULL );
			if( bResult ){
					//convert lower case
					_tcslwr( (PTCHAR)sVID );	_tcslwr( (PTCHAR)sPID );	_tcslwr( (PTCHAR)sInf );
					nMode = ModeVidPidInf;
					pssID[2] = sInf;
			}
		}
	}
	else{
		if( nInf<0 ){
			bResult = ParsingUSBDevicePath( sDevPath,(PTCHAR)sVID,(PTCHAR)sPID, NULL, (PTCHAR)sRev );
			if( bResult ){
					//convert lower case
					_tcslwr( (PTCHAR)sVID );	_tcslwr( (PTCHAR)sPID );	_tcslwr( (PTCHAR)sRev );
					nMode = ModeVidPidRev;
					pssID[2] = sRev;
			}

		}
		else{
			bResult = ParsingUSBDevicePath( sDevPath,(PTCHAR)sVID,(PTCHAR)sPID,(PTCHAR)sInf, (PTCHAR)sRev );
			if( bResult ){
					//convert lower case
					_tcslwr( (PTCHAR)sVID );	_tcslwr( (PTCHAR)sPID );	_tcslwr( (PTCHAR)sInf );	_tcslwr( (PTCHAR)sRev );
					nMode = ModeVidPidInfRev;
					pssID[3] = sRev;	pssID[2] = sInf;
			}

		}
	}

	
	if( !bResult )
		return FALSE;

	
	//convert VID and PID string type to VID and PID word type.
	for( j=0; j<4; j++ ){

		pcID = (LPBYTE)(&(wID[j]));
		pcID++;
		psID = pssID[j];

		nLen = _tcsclen( psID );

		for( i=0; i<nLen; i++ ){
			//high nibble
			if( psID[i]>=_T('0') && psID[i]<=_T('9') ){
				cItem=(BYTE)( psID[i] - _T('0') );
				cItem=cItem<<4;
			}
			else if( psID[i]>=_T('a') && psID[i]<=_T('f') ){
				cItem=(BYTE)( psID[i] - _T('a') + 0x0A);
				cItem=cItem<<4;
			}
			else
				return FALSE;

			i++;
			//low nibble
			if( psID[i]>=_T('0') && psID[i]<=_T('9') ){
				cItem+=(BYTE)( psID[i] - _T('0') );
			}
			else if( psID[i]>=_T('a') && psID[i]<=_T('f') ){
				cItem+=(BYTE)( psID[i] - _T('a') + 0x0A );
			}
			else
				return FALSE;
			*pcID=cItem;
			pcID--;
		}//end for

		for( ; i<(sizeof(WORD)*2); i++ ){
			wID[j] = wID[j] >> 4;
		}//end for

		if( j == 1 ){

			if( nMode == ModeVidPid )
				break;	//exit for
		}
		else if( j == 2 ){
			if( nMode == ModeVidPidInf || nMode == ModeVidPidRev )
				break;//exit for

		}

	}//end for

	switch( nMode ){
		case ModeVidPid:

			if( nPID < 0 ){
				if( (WORD)nVID == wID[0] )
					return TRUE;	//matching VID.
			}
			else{
				if( (WORD)nVID == wID[0] &&  (WORD)nPID == wID[1] ){
					return TRUE;	//matching VID & PID.
				}
			}
			break;
		case ModeVidPidRev:
			if( (WORD)nVID == wID[0] &&  (WORD)nPID == wID[1] && (WORD)nRev == wID[2] ){
				return TRUE;	//matching VID & PID & Rev.
			}
			break;
		case ModeVidPidInf:
			if( (WORD)nVID == wID[0] &&  (WORD)nPID == wID[1] && (WORD)nInf == wID[2] ){
				return TRUE;	//matching VID & PID & interface.
			}
			break;
		case ModeVidPidInfRev:
			if( (WORD)nVID == wID[0] &&  (WORD)nPID == wID[1] && (WORD)nRev == wID[3] && (WORD)nInf == wID[2] ){
				return TRUE;	//matching VID & PID & Rev & interface.
			}
			break;

	}//end switch

	return FALSE;
}


//the first byte of lpBuffer is report ID.
//If the top-level collection includes report IDs,
//the caller must set the first byte of the buffer to a nonzero report ID;
//otherwise the caller must set the first byte to zero.
BOOL	CTTR_DeviceHid::WriteFeatureReport(
		HANDLE hHID,				//handle to HID device
		LPVOID ReportBuffer,		// data buffer
		ULONG  ReportBufferLength	// number of bytes written,excluding its report ID
	)
{
	return HidD_SetFeature( hHID,ReportBuffer,ReportBufferLength );
}

//the first byte of lpBuffer is report ID.
//If the top-level collection includes report IDs,
//the caller must set the first byte of the buffer to a nonzero report ID;
//otherwise the caller must set the first byte to zero.
BOOL	CTTR_DeviceHid::ReadFeatureReport(
		HANDLE hHID,				//handle to HID device
		LPVOID ReportBuffer ,		// data buffer
		ULONG  ReportBufferLength	// number of bytes read, excluding its report ID
	)
{
	return HidD_GetFeature( hHID,ReportBuffer,ReportBufferLength );
}

INT  CTTR_DeviceHid::GetNumInputBuffers()
{
	return GetNumInputBuffers( m_hDev );
}

static BOOL SetNumInputBuffers(	HANDLE hHID,INT nNumberBuffers );

BOOL CTTR_DeviceHid::SetNumInputBuffers( INT nNumberBuffers )
{
	return SetNumInputBuffers( m_hDev, nNumberBuffers );
}

