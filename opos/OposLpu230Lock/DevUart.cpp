#include "StdAfx.h"
#include "DevUart.h"

#include <cstring>
#include <string>


CDevUart::CDevUart(void)
{
}


CDevUart::~CDevUart(void)
{
}

int CDevUart::GetDevList( list<_tstring> & listDevPath )
{
	int nDev = 0;
	HANDLE hDev = NULL;

	listDevPath.clear();

	list<_tstring> listCom;

	//GetComPortList( listCom );
	GetcomPortListByRegistry( listCom );

	list<_tstring>::iterator iter = listCom.begin();

	for( ; iter != listCom.end(); ++iter ){

		hDev = Open( *iter );

		if( hDev != INVALID_HANDLE_VALUE ){
			//success open com port.

			//test lpu230 connection port.
			if( Config( hDev ) ){
				//
				if( SetTimeOut( hDev ) ){

					BYTE sCommand[SIZE_PACKET];

					::memset( sCommand,0, sizeof(sCommand) );
					sCommand[0] = CODE_FIRST_DEV_DETECT;
					sCommand[1] = CODE_SECOND_DEV_DETECT;

					//send command
					if( Write( hDev, sCommand, SIZE_PACKET ) ){

						BYTE sResponse[] = { 0, 0 };

						//get response
						if( Read( hDev, sResponse, sizeof(sResponse) ) ){

							if( IsDevDetectOk( sCommand, sResponse ) ){
								listDevPath.push_back( *iter );
								nDev++;
							}
						}
					}
				}
			}

			Close( hDev );
		}
	}//end for

	return nDev;
}

int CDevUart::GetComPortList( list<_tstring> & listDevPath )
{
	int nDev = 0;
	int i = 0;

	_tstring sDevPath;
	HANDLE hDev = NULL;

	listDevPath.clear();

	for( i = 1; i<=MAX_NUMBER_COM_PORT; i++ ){

		//make COM path
		sDevPath = _T("COM") + to_tstring( (unsigned long long)i );

		hDev = Open( sDevPath );

		if( hDev != INVALID_HANDLE_VALUE ){

			listDevPath.push_back( sDevPath );
			nDev++;

			Close( hDev );
		}

	}//end for

	return nDev;
}

int CDevUart::GetcomPortListByRegistry( list<_tstring> & listDevPath )
{
	HKEY hKey;
    TCHAR szValue[256];
	TCHAR szValueName[256];
    DWORD dwValue, dwType,dwValueName;
	DWORD dwIndex;
	long lResult;
	int nDev = 0;

	listDevPath.clear();

    lResult=RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("HARDWARE\\DEVICEMAP\\SERIALCOMM"), NULL, KEY_ALL_ACCESS, &hKey);
	if( lResult!=ERROR_SUCCESS )
		return nDev;

	dwIndex=0;

	do{
		dwValue=256;
		dwValueName=256;

		lResult=::RegEnumValue( hKey,dwIndex,szValueName,&dwValueName,NULL,&dwType,(unsigned char *)szValue, &dwValue );

		if( lResult==ERROR_SUCCESS ){
			dwIndex++;
			listDevPath.push_back( _tstring(szValue) );
			nDev++;
		}
	}while( lResult==ERROR_SUCCESS );


	::RegCloseKey(hKey);

	return nDev;
}

bool CDevUart::IsDevDetectOk( BYTE sCmd[], BYTE sResp[] )
{
	if( 	((CODE_XOR_RESPONSE^sCmd[0]) == sResp[0] )
		&&
		((CODE_XOR_RESPONSE^sCmd[1]) == sResp[1] )
		){
		return true;
	}
	else
		return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//
/////////////////////////////////////////////////////////////////////////////////////////////
 
HANDLE CDevUart::Open( const _tstring & sPath )
{
	/*
	HANDLE	hDev = CreateFile(
				sPath.c_str(),
				GENERIC_READ | GENERIC_WRITE, 
				FILE_SHARE_READ | FILE_SHARE_WRITE,
				(LPSECURITY_ATTRIBUTES)NULL, 
				OPEN_EXISTING, 
				FILE_FLAG_OVERLAPPED, 
				NULL
				);
				*/

	HANDLE	hDev = CreateFile(
				sPath.c_str(),
				GENERIC_READ | GENERIC_WRITE, 
				0,
				0,
				OPEN_EXISTING, 
				0, 
				NULL
				);

	if( hDev != INVALID_HANDLE_VALUE ){

	}

	return hDev;

}

bool CDevUart::Config( 
	HANDLE hDev,
	DWORD BaudRate /*= CBR_9600*/,
	BYTE ByteSize /*= 8*/,
	DWORD fParity /*= 0*/,
	BYTE Parity /*= NOPARITY*/,
	BYTE StopBits /*= ONESTOPBIT*/
	)
{
	DCB dcb;

	bool bPortReady;

	if( ::GetCommState( hDev, &dcb ) )
		bPortReady = true;
	else
		bPortReady = false;

	if( !bPortReady ){
		return false;
	}

	dcb.BaudRate          = BaudRate;  
	dcb.ByteSize          = ByteSize;  
	dcb.Parity            = Parity ;  
	dcb.StopBits          = StopBits;  
	dcb.fBinary           = true;  
	dcb.fDsrSensitivity   = false;  
	dcb.fParity           = fParity;  
	dcb.fOutX             = false;  
	dcb.fInX              = false;  
	dcb.fNull             = false;  
	dcb.fAbortOnError     = true;  
	dcb.fOutxCtsFlow      = false;  
	dcb.fOutxDsrFlow      = false;  
	dcb.fDtrControl       = DTR_CONTROL_DISABLE;  
	dcb.fDsrSensitivity   = false;  
	dcb.fRtsControl       = RTS_CONTROL_DISABLE;  
	dcb.fOutxCtsFlow      = false;  
	dcb.fOutxCtsFlow      = false;  
  
	if( ::SetCommState(hDev, &dcb) )
		bPortReady = true;
	else
		false;

	return bPortReady;
}

 ////////////
bool CDevUart::SetTimeOut(
	HANDLE hDev,
	DWORD ReadIntervalTimeout /*= C_dwReadIntervalTimeout */,
	DWORD ReadTotalTimeoutMultiplier /*= C_dwReadTotalTimeoutMultiplier */,
	DWORD ReadTotalTimeoutConstant /*= C_dwReadTotalTimeoutConstant */,  
	DWORD WriteTotalTimeoutMultiplier /*= C_dwWriteTotalTimeoutMultiplier */,
	DWORD WriteTotalTimeoutConstant /*= C_dwWriteTotalTimeoutConstant */
	)
{
	COMMTIMEOUTS CommTimeouts;
	bool bPortReady;
	
	if( ::GetCommTimeouts( hDev, &CommTimeouts ) )
		bPortReady = true;
	else
		bPortReady = false;

	if( !bPortReady ){
		return false;
	}

    CommTimeouts.ReadIntervalTimeout          = ReadIntervalTimeout;  
    CommTimeouts.ReadTotalTimeoutConstant     = ReadTotalTimeoutConstant;  
    CommTimeouts.ReadTotalTimeoutMultiplier   = ReadTotalTimeoutMultiplier;  
    CommTimeouts.WriteTotalTimeoutConstant    = WriteTotalTimeoutConstant;  
    CommTimeouts.WriteTotalTimeoutMultiplier  = WriteTotalTimeoutMultiplier;  
      
    if( ::SetCommTimeouts(hDev, &CommTimeouts) )
		bPortReady = true;
	else
		bPortReady = false;

	return bPortReady;
}

bool CDevUart::WriteByte( HANDLE hDev, BYTE cData )
{
	DWORD dwWritten = 0;

	if( ::WriteFile(hDev, &cData, 1, &dwWritten, NULL) == 0 )
		return false;
	else
		return true;
}

bool CDevUart::Write( HANDLE hDev, BYTE *sData, unsigned int  nSize )
{
	DWORD dwWritten = 0;

	if( ::WriteFile(hDev, sData, nSize, &dwWritten, NULL) ){

		if( nSize == dwWritten )
			return true;
	}

	return false;
}
  
bool CDevUart::ReadByte( HANDLE hDev, BYTE & cData )
{  
	BYTE rx;  
	cData = 0;  
	DWORD dwBytesTransferred=0;  
  
	if( ::ReadFile( hDev, &rx, 1, &dwBytesTransferred, 0) ){  

		if(dwBytesTransferred == 1){
			cData = rx;
			return true;
		}
	}

	return false;
}

bool CDevUart::Read( HANDLE hDev, BYTE *sData, unsigned int  nSize )
{
	DWORD dwBytesTransferred = 0;

	if( ::ReadFile( hDev, sData, nSize, &dwBytesTransferred, 0 ) ){

		if( dwBytesTransferred == nSize )
			return true;
	}

	return false;
}

  
void CDevUart::Close( HANDLE hDev )
{  
    ::CloseHandle( hDev );  
}  

