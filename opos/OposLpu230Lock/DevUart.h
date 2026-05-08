#pragma once

#include <list>

using namespace std;

class CDevUart
{

public:
	static const DWORD C_dwReadIntervalTimeout = 5;
	static const DWORD C_dwReadTotalTimeoutMultiplier = 5;  
	static const DWORD C_dwReadTotalTimeoutConstant = 100;  
	static const DWORD C_dwWriteTotalTimeoutMultiplier = 0;  
	static const DWORD C_dwWriteTotalTimeoutConstant = 0;  

private:
	enum{
		MAX_NUMBER_COM_PORT = 10,
		SIZE_PACKET = 65
	};

	enum : BYTE{
		CODE_FIRST_DEV_DETECT = '@',
		CODE_SECOND_DEV_DETECT = '$',

		CODE_XOR_RESPONSE = 'R'
	};


public:
	CDevUart(void);
	virtual ~CDevUart(void);
	
	int GetDevList( list<_tstring> & listDevPath );

	int GetComPortList( list<_tstring> & listDevPath );
	int GetcomPortListByRegistry( list<_tstring> & listDevPath );

	HANDLE Open( const _tstring & sPath );

	bool Config( 
		HANDLE hDev,
		DWORD BaudRate = CBR_9600,
		BYTE ByteSize = 8,
		DWORD fParity = 0,
		BYTE Parity = NOPARITY,
		BYTE StopBits = ONESTOPBIT
		);

	bool CDevUart::SetTimeOut(
		DWORD ReadIntervalTimeout,  
		DWORD ReadTotalTimeoutMultiplier,
		DWORD ReadTotalTimeoutConstant,  
		DWORD WriteTotalTimeoutMultiplier,
		DWORD WriteTotalTimeoutConstant
		);

	bool CDevUart::SetTimeOut(
		HANDLE hDev,
		DWORD ReadIntervalTimeout = C_dwReadIntervalTimeout,
		DWORD ReadTotalTimeoutMultiplier = C_dwReadTotalTimeoutMultiplier,
		DWORD ReadTotalTimeoutConstant = C_dwReadTotalTimeoutConstant,  
		DWORD WriteTotalTimeoutMultiplier = C_dwWriteTotalTimeoutMultiplier,
		DWORD WriteTotalTimeoutConstant = C_dwWriteTotalTimeoutConstant
		);

	bool WriteByte( HANDLE hDev, BYTE cData );

	bool Write( HANDLE hDev, BYTE *sData, unsigned int  nSize );

	bool ReadByte( HANDLE hDev, BYTE & cData );

	bool Read( HANDLE hDev, BYTE *sData, unsigned int  nSize );

	void Close( HANDLE hDev );

private:
	CDevUart( const CDevUart & );

	bool IsDevDetectOk( BYTE sCmd[], BYTE sResp[] );
};

