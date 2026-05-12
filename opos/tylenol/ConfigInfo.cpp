#include "StdAfx.h"
#include "ConfigInfo.h"


CConfigInfo::CConfigInfo(void)
{
	//unsigned char cId[] = { '@', ':', ';' };
	unsigned char cId[] = { 0, 0, 0 };
	unsigned char sCKey[]=	{ 0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f };
	unsigned char sKey[]=	{ 0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37 };

	//set default
	m_nBuzzer = 26666;
	m_nWdt = 500;

	m_cKeymap = 0;

	for( INT32 i=0; i< THE_NUMBER_OF_TRACKS; i++ ){


		m_cId[i] = cId[i];
		m_cEnableEnter[i] = 1;
		m_cEnableEncryption[i] = 0;

		memcpy( m_sChangeKey[i], sCKey, THE_SIZE_OF_KEYS );
		memcpy( m_sEncryptionKey[i], sKey, THE_SIZE_OF_KEYS );

	}//end for

}

CConfigInfo::~CConfigInfo(void)
{
}

void CConfigInfo::setKeymap( unsigned char cKeymap)
{
	if( cKeymap )
		m_cKeymap = 1;
	else
		m_cKeymap = 0;
}

void CConfigInfo::setId(INT32 nIndex, unsigned char cId)
{
	if( nIndex >= THE_NUMBER_OF_TRACKS )
		return;
	//
	m_cId[nIndex] = cId;
}

void CConfigInfo::setEnableEnter(INT32 nIndex, unsigned char cEnableEnter)
{
	if( nIndex >= THE_NUMBER_OF_TRACKS )
		return;
	//
	if( cEnableEnter )
		m_cEnableEnter[nIndex] = 1;
	else
		m_cEnableEnter[nIndex] = 0;
}

void CConfigInfo::setEnableEncryption(INT32 nIndex, unsigned char cEnableEncryption)
{
	if( nIndex >= THE_NUMBER_OF_TRACKS )
		return;
	//
	if( cEnableEncryption )
		m_cEnableEncryption[nIndex] = 1;
	else
		m_cEnableEncryption[nIndex] = 0;
}

BOOL CConfigInfo::SetChangeKey( INT32 nIndex, LPCTSTR sChangeKey )
{
	if( sChangeKey == NULL )
		return FALSE;
	//
	INT32 nLen = _tcslen( sChangeKey );

	if( nLen <= 0 )
		return FALSE;
	//
	if( nLen>255 )
		return FALSE;

	TCHAR seps[]   = _T(" ,");
	TCHAR sKey[256];
	TCHAR *token;
	unsigned char sHex[THE_SIZE_OF_KEYS];
	INT32 i = 0;

	memset( sHex, 0, THE_SIZE_OF_KEYS );

	_tcscpy( sKey,sChangeKey );

	// Establish string and get the first token:
	token = _tcstok( sKey, seps ); // C4996

	// Note: strtok is deprecated; consider using strtok_s instead
	 while( token != NULL ){

		// While there are tokens in "string"
		//printf( " %s\n", token );

		if( !ChangeStringToHexToken( &(sHex[i]), token ) )
			return FALSE;

		i++;

		// Get next token: 
		token = _tcstok( NULL, seps ); // C4996

	}//end while

	//
	//CConfigInfo::m_sChangeKey

	memcpy( m_sChangeKey[nIndex], sHex, THE_SIZE_OF_KEYS );

	return TRUE;
}

BOOL CConfigInfo::SetEncryptionKey( INT32 nIndex, LPCTSTR EncryptionKey )
{
	if( EncryptionKey == NULL )
		return FALSE;
	//
	INT32 nLen = _tcslen( EncryptionKey );

	if( nLen <= 0 )
		return FALSE;
	//
	if( nLen>255 )
		return FALSE;

	TCHAR seps[]   = _T(" ,");
	TCHAR sKey[256];
	TCHAR *token;
	unsigned char sHex[THE_SIZE_OF_KEYS];
	INT32 i = 0;

	memset( sHex, 0, THE_SIZE_OF_KEYS );

	_tcscpy( sKey,EncryptionKey );

	// Establish string and get the first token:
	token = _tcstok( sKey, seps ); // C4996

	// Note: strtok is deprecated; consider using strtok_s instead
	 while( token != NULL ){

		// While there are tokens in "string"
		//printf( " %s\n", token );

		if( !ChangeStringToHexToken( &(sHex[i]), token ) )
			return FALSE;

		i++;

		// Get next token: 
		token = _tcstok( NULL, seps ); // C4996

	}//end while

	//
	//CConfigInfo::m_sChangeKey

	memcpy( m_sEncryptionKey[nIndex], sHex, THE_SIZE_OF_KEYS );

	return TRUE;
}


BOOL CConfigInfo::ChangeStringToHexToken( unsigned char *pcHex, const TCHAR *sHex )
{
	if( pcHex == NULL || sHex == NULL )
		return FALSE;
	//
	unsigned char cH,cL;
	INT32 nLen = _tcslen( sHex );

	if( nLen != 1 && nLen != 2 )
		return FALSE;

	////////////////////////////////////////////////
	//high
	INT32 i =0 ;
	cH = (unsigned char)sHex[i];

	if( cH >= '0' && cH <= '9' ){
		cH = cH - '0';
	}
	else if( cH >= 'a' && cH <= 'f' ){
		cH = cH - 'a' + 10;
	}
	else if( cH >= 'A' && cH <= 'F' ){
		cH = cH - 'A' + 10;
	}
	else
		return FALSE;

	cH = cH << 4;

	////////////////////////////////////////////////
	//low
	i++;
	cL = (unsigned char)sHex[i];

	if( cL >= '0' && cL <= '9' ){
		cL = cL - '0';
	}
	else if( cL >= 'a' && cL <= 'f' ){
		cL = cL - 'a' + 10;
	}
	else if( cL >= 'A' && cL <= 'F' ){
		cL = cL - 'A' + 10;
	}
	else
		return FALSE;

	//////////////////////////////////////////////
	*pcHex = cH | cL;

	return TRUE;
}