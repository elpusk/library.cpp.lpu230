//////////////////////////////////////////////////////////////
// ELCrcChk.cpp : implementation file

#include "stdafx.h"
#include "ELCrcChk.h"
#include "Windows.h"
#include <malloc.h>


//internal funation prototype

unsigned short EL_ISO14443TypeBUpdateCrc( BYTE ch,unsigned short*lpwCrc );


///////////////////////////////
//Get Check-code of given file
//sFileName[IN] : file name
//pdwChkCode[OUT] : check code
//return : Processing result
BOOL EL_GetDWORD_ChkCode( unsigned long *pdwChkCode,const TCHAR *sFileName )
{
	HANDLE hFile;
	unsigned long dwBuffer;
	unsigned long dwReadCnt;
	BOOL bResult;
	unsigned long dwChkCode;


	//check parameters
	if( pdwChkCode==NULL || sFileName==NULL )
		return FALSE;

	dwReadCnt=dwChkCode=dwBuffer=0;
	
	//Open file
	hFile=CreateFile(
		sFileName,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if( hFile==INVALID_HANDLE_VALUE ){
		return FALSE;//open failure
	}

	/////////////////////
	//calculate Check code
	do{
		//clear reading buffer
		dwBuffer=0;
		//read 4-bytes from file
		bResult=ReadFile( 
			hFile,//file handle
			&dwBuffer, // data buffer
			sizeof(unsigned long),  // number of bytes to read
			&dwReadCnt, // number of bytes read
			NULL
		);
		if( !bResult ){
			//File Reading Error
			CloseHandle( hFile );	//return file-handle to syste,
			return FALSE;
		}

		dwChkCode=dwChkCode^dwBuffer;//XOR operation

	}while( dwReadCnt==sizeof(unsigned long) );//read to end of file
	
	CloseHandle( hFile );	//return file-handle to syste,
	//
	//save return check-code
	*pdwChkCode=dwChkCode;

	return TRUE;
}


BOOL EL_GetFileVersion( TCHAR *sVersion,const TCHAR *lpDestFile )
{
	unsigned long dwHdlDest;
	unsigned long dwSizeDest;
	unsigned long dwDestLS, dwDestMS;
	TCHAR *pDestData;
	VS_FIXEDFILEINFO *pvsInfo;
	UINT uLen;
	BOOL dwRet;
	DWORD dwDestLS1, dwDestLS2, dwDestMS1, dwDestMS2;
	BOOL bResult=FALSE;

	//Get fileversion buffer size
	dwSizeDest = GetFileVersionInfoSize(lpDestFile, &dwHdlDest);

	if(dwSizeDest){
		//allcate memory for file version info
		pDestData =(TCHAR *)malloc(dwSizeDest+sizeof(TCHAR) );
		if( pDestData==NULL )
			return FALSE;

		//get file information
		if( GetFileVersionInfo(lpDestFile, dwHdlDest, dwSizeDest, pDestData) ){
			dwRet = VerQueryValue(pDestData,_T("\\"), (void**)&pvsInfo, &uLen);
			if(dwRet){
				dwDestMS = pvsInfo->dwFileVersionMS; // Major version
				dwDestLS = pvsInfo->dwFileVersionLS; // Minor version

				dwDestMS1 = (dwDestMS / 65536);
				dwDestMS2 = (dwDestMS % 65536);
				dwDestLS1 = (dwDestLS / 65536);
				dwDestLS2 = (dwDestLS % 65536);

				wsprintf(sVersion, _T("%d.%d.%d.%d"), dwDestMS1, dwDestMS2, 
				dwDestLS1, dwDestLS2);
				bResult=TRUE;
			}
		}

		free( pDestData);
	}

	return bResult;
}

//////////////////////////////
//generates BCC code from byte stream
//pBuffer[IN] : byte stream pointer
//nSize[IN] : sizeof data
//return one-byte BCC code
BYTE EL_MakeBCC( LPBYTE pBuffer, int nSize )
{
	BYTE bcc = 0;
	int i;
	//
	for(i = 0; i < nSize; i++)
		bcc ^= pBuffer[i];
	//
	return bcc;
}


unsigned short EL_ISO14443TypeBUpdateCrc( BYTE ch,unsigned short*lpwCrc )
{
	ch = (ch^(BYTE)((*lpwCrc) & 0x00FF));
	
	ch = (ch^(ch<<4));
	
	*lpwCrc = (*lpwCrc >> 8)^((USHORT)ch <<	8)^((USHORT)ch<<3)^((USHORT)ch>>4);
	
	return(*lpwCrc);
}

void EL_ISO14443TypeBGetCrc( BYTE *Data, int Length,BYTE *TransmitFirst,BYTE *TransmitSecond )
{
	BYTE chBlock;
	unsigned short wCrc;
	wCrc = 0xFFFF; // ISO 3309
	
	do{
		chBlock = *Data++;
		EL_ISO14443TypeBUpdateCrc(chBlock, &wCrc);
		
	}while (--Length);//end do-while
	
	wCrc = ~wCrc; // ISO 3309
	*TransmitFirst = (BYTE) (wCrc & 0xFF);
	*TransmitSecond = (BYTE) ((wCrc >> 8) & 0xFF);
	return;
}