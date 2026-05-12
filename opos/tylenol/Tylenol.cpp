#include "StdAfx.h"
#include "Tylenol.h"
#include "TTR_DevHid.h"
#include "ttr_tchar.h"


#ifdef	_UNICODE
#define	_tcout		wcout

#else
#define	_tstring	string
#endif

CTTR_DeviceHid CTylenol::m_HidDevice;

vector< _tstring > CTylenol::m_vDevPath;

bool CTylenol::m_bIsCuiMode = false;

CTylenol::CTylenol(void)
{
}


CTylenol::~CTylenol(void)
{
}

CTylenol::DevListType & CTylenol::GetDeviceList()
{

	m_vDevPath.clear();

	TCHAR sPath[512];
	bool bResult = false;
	list<_tstring> DevPathList;
	INT32 nDev = m_HidDevice.GetDevList(DevPathList);

	if( nDev > 0 ){

		list<_tstring>::iterator dev = DevPathList.begin();

		for( dev = DevPathList.begin(); dev != DevPathList.end(); dev++ ){

			_tcscpy( sPath, (*dev).c_str() );

			if( CTTR_DeviceHid::IsWantUSBDevice( sPath, 0x134b, 0x0206, 1) ){//lpu237
				//find ok
				m_vDevPath.push_back( *dev );
			}
			else if (CTTR_DeviceHid::IsWantUSBDevice(sPath, 0x134b, 0x0214, 0)) {//lpu238
				//find ok
				m_vDevPath.push_back(*dev);
			}

		}//end for

	}

	return m_vDevPath;
}

CTylenol::DevListType & CTylenol::GetDeviceList( int nVid, int nPid, int nInf/*= -1*/ )
{
	static vector< _tstring > vDevPath;

	vDevPath.clear();

	TCHAR sPath[512];
	bool bResult = false;
	list<_tstring> DevPathList;
	INT32 nDev = m_HidDevice.GetDevList(DevPathList);

	if( nDev > 0 ){

		list<_tstring>::iterator dev = DevPathList.begin();

		for( dev = DevPathList.begin(); dev != DevPathList.end(); dev++ ){

			_tcscpy( sPath, (*dev).c_str() );

			if( CTTR_DeviceHid::IsWantUSBDevice( sPath, nVid, nPid, nInf ) ){
				//find ok
				vDevPath.push_back( *dev );
			}
		}//end for

	}

	return vDevPath;
}

void CTylenol::OpenDevice( const _tstring & sDevPath )
{
	if( !m_HidDevice.Open( sDevPath.c_str() ) ){
		throw ErrorOpen;
	}
}


void CTylenol::OpenDevice()
{
	bool bResult = false;

	list<_tstring> DevPathList;
	TCHAR sPath[512];
	INT32 nDev;

	nDev = m_HidDevice.GetDevList(DevPathList);

	if( nDev > 0 ){

		list<_tstring>::iterator dev = DevPathList.begin();

		for( dev = DevPathList.begin(); dev != DevPathList.end(); dev++ ){

			_tcscpy( sPath, (*dev).c_str() );

			if( CTTR_DeviceHid::IsWantUSBDevice( sPath, 0x134b, 0x0206, 1) ){
				//find ok
				if( m_HidDevice.Open( *dev ) ){
					bResult = true;
				}
				else{
					break;
				}
			}
			else if (CTTR_DeviceHid::IsWantUSBDevice(sPath, 0x134b, 0x0214, 0)) {
				//find ok
				if (m_HidDevice.Open(*dev)) {
					bResult = true;
				}
				else {
					break;
				}
			}

		}//end for

		if( !bResult )
			throw ErrorOpen;

	}
	else{
		throw ErrorNoDevice;
	}

}

void CTylenol::EnterConfigMode()
{
	try{
		SendRequest( Cmd_EnterCS, 0, 0, NULL );
	}
	catch( ... ){
		throw ErrorCmdEnter;
	}
}

void CTylenol::LeaveConfigMode()
{
	try{
		SendRequest( Cmd_LeaveCS, 0, 0, NULL );
	}
	catch( ... ){
		throw ErrorCmdLeave;
	}
}

void CTylenol::ApplyConfigMode()
{
	try{
		SendRequest( Cmd_Apply, 0, 0, NULL );
	}
	catch( ... ){
		throw ErrorCmdApply;
	}
}

void CTylenol::GotoBootLoaderConfigMode()
{
	try{
		SendRequest( Cmd_GotoBootLoader, 0, 0, NULL );
	}
	catch( ... )	{
		throw ErrorCmdGotoBoot;	//chang error code.......
	}
}

void CTylenol::CloseDevice()
{

	m_HidDevice.Close();

	if( m_bIsCuiMode )
		_tcout << _T(" * SUCCESS : CLOSE DEVICE.") <<endl;

}


//send request & get response
void CTylenol::SendRequest( const MSR_HostCmd Cmd, const unsigned char cSub,INT32 nData, const unsigned char *sData, unsigned char *sResp /*= NULL*/  )
{
	bool bResult = false;

	INT32 nBuf;
	INT32 nPacket;
	INT32 nRemainder;
	INT32 nOffset = 0;

	unsigned char sBuf[512];
	unsigned char sPacket[65];

	memset( sBuf,0,512 );

	sBuf[0] = 0;	//report ID
	sBuf[1] = (unsigned char)Cmd;
	sBuf[2] = cSub;
	sBuf[3] = (unsigned char)nData;

	if( nData > 0 )
		memcpy( &sBuf[4], sData,nData );

	if( nData >= sizeof(UINT32)*2 ){
		UINT32 dwOff,dwSize;
		memcpy( &dwOff, sData, sizeof(dwOff) );
		memcpy( &dwSize, &sData[sizeof(dwOff)], sizeof(dwSize) );
		//_tout << _T("(offset,size)=(")<< dwOff << _T(",") << dwSize << _T(")") <<endl;
	}

	/////////////////////////////////////////////////////////
	//the first send
	nRemainder = nData + 3 + 1;

	memset( sPacket, 0 , 65 );

	if( nRemainder > 65 ){
		memcpy( sPacket, sBuf, 65 );
		nPacket = 65;
		nOffset += nPacket;
		nRemainder -= 65;
		
	}
	else{
		memcpy( sPacket, sBuf, 65 );
		nPacket = nRemainder;
		nOffset += nPacket;
		nRemainder = 0;

	}

	if( !m_HidDevice.Write( sPacket, 65,(LPDWORD)&nPacket,(LPOVERLAPPED)NULL ) ){

		if( m_bIsCuiMode )
			_tcout << _T(" * ERROR : SEND ENTER COMMAND.") <<endl;
		throw ErrorSendCmd;
	}

	/////////////////////////////////////////////////////////
	//the partial send
	while( nRemainder > 0 ){

		memset( sPacket, 0 , 65 );

		if( nRemainder > 64 ){
			memcpy( &sPacket[1], &sBuf[nOffset], 64 );
			nPacket = 64;
			nOffset += nPacket;
			nRemainder -= 64;
		}
		else{
			memcpy( &sPacket[1],  &sBuf[nOffset], nRemainder );
			nPacket = nRemainder;
			nOffset += nPacket;
			nRemainder = 0;
		}

		if( !m_HidDevice.Write( sPacket, 65,(LPDWORD)&nPacket,(LPOVERLAPPED)NULL ) ){

			if( m_bIsCuiMode )
				_tcout << _T(" * ERROR : SEND ENTER COMMAND.") <<endl;
			throw ErrorSendCmd;
		}


	}//end while


	/////////////////////////////////////////////////////////
	//getting response.
	nBuf = 1+3+76+37+104;
	memset( sBuf,0,nBuf );

	if( !m_HidDevice.Read( sBuf, (DWORD)nBuf, (LPDWORD)&nBuf,(LPOVERLAPPED)NULL ) ){
		
		if( m_bIsCuiMode )
			_tcout << _T(" * ERROR : GET RESPONSE OF ENTER COMMAND.") <<endl;
		throw ErrorGetResponse;
	}
	else{

		if( (sBuf[1] == MSR_RESP_PREFIX && sBuf[2] == Resp_Good) ||
			(sBuf[1] == MSR_RESP_PREFIX && sBuf[2] == Resp_GoodNegative)
			){
			bResult = true;

			if( sResp ){
				memcpy( sResp, &sBuf[1], nBuf-1 );
			}
		}
		else{
			throw ErrorUnknownResponse;
		}
	}

}

void CTylenol::SendRequest( const MSR_HOST_PACKET & Req,  PMSR_HOST_PACKET Resp /*= NULL*/  )
{
	SendRequest( 
		static_cast<MSR_HostCmd>(Req.cCmd),
		Req.cSub, static_cast<INT32>(Req.cLen),
		static_cast<const unsigned char*>(Req.sData),
		reinterpret_cast<unsigned char*>(Resp)
		);
}

void CTylenol::GotoBoot( const _tstring & sDevPath )
{
	OpenDevice( sDevPath );

	EnterConfigMode();

	GotoBootLoaderConfigMode();

	if( m_bIsCuiMode ){
		_tcout << _T(" * SUCCESS : Start Bootloader.")<<endl;
		_tcout << _T("*************************************************************************") <<endl;
		_tcout << _T(" * NOTICE : Please waits for detecting drive with label \"CRP2 ENABLD\".)") <<endl;
		_tcout << _T("*************************************************************************") <<endl;
	}

	CloseDevice();

	if( m_bIsCuiMode )
		_tcout << "the end of program." <<endl;
}

void CTylenol::GotoBoot()
{
	OpenDevice();

	EnterConfigMode();

	GotoBootLoaderConfigMode();

	if( m_bIsCuiMode ){
		_tcout << _T(" * SUCCESS : Start Bootloader.")<<endl;
		_tcout << _T("*************************************************************************") <<endl;
		_tcout << _T(" * NOTICE : Please waits for detecting drive with label \"CRP2 ENABLD\".)") <<endl;
		_tcout << _T("*************************************************************************") <<endl;
	}

	CloseDevice();

	if( m_bIsCuiMode )
		_tcout << "the end of program." <<endl;
}

void CTylenol::PrintError( enumErrorCode code )
{
	_tstring sMsg;

	switch( code ){
		case ErrorOpen:
			sMsg = _T("ERROR : ErrorOpen ");
			break;
		case ErrorNoDevice:
			sMsg = _T("ERROR :ErrorNoDevice ");
			break;
		case 	ErrorSendCmd:
			sMsg = _T("ERROR : ErrorSendCmd ");
			break;
		case 	ErrorGetResponse:
			sMsg = _T("ERROR : ErrorGetResponse ");
			break;
		case 	ErrorUnknownResponse:
			sMsg = _T("ERROR : ErrorUnknownResponse ");
			break;
		case 	ErrorCmdGotoBoot:
			sMsg = _T("ERROR : ErrorCmdGotoBoot ");
			break;
		case 	ErrorCmdApply:
			sMsg = _T("ERROR : ErrorCmdApply ");
			break;
		case 	ErrorCmdLeave:
			sMsg = _T("ERROR : ErrorCmdLeave ");
			break;
		case 	ErrorCmdEnter:
			sMsg = _T("ERROR : ErrorCmdEnter");
			break;
		default:
			break;
	}//end switch

	_tout << sMsg << endl;

}
