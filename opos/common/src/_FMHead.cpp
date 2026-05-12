#include "stdafx.h"
#include "FMHead.h"
#include <stdlib.h>         /* For _MAX_PATH definition */
#include <stdio.h>
#include <malloc.h>


//allocate memory and reset data field of ROM file heaer
PROMHEADER CreateROMHeader()
{
	PROMHEADER pHeader;
	unsigned char *pTemp;
	unsigned long lSize;
	int i,j;
	unsigned char *psrc,*pdes;
	
	pHeader=(PROMHEADER)malloc(sizeof(ROMHEADER));
	if( pHeader!=NULL ){
		memset( pHeader,0x00,sizeof(ROMHEADER) );
		
		for( i=0; i<4; i++)
			pHeader->sPreTag[i]=0xa5;
		
		lSize=sizeof(ROMHEADER);
		//convert little endian to Big-endian
		psrc=(unsigned char *)&lSize;
		pdes=(unsigned char *)&pHeader->lHSize;
		pdes=pdes+sizeof(unsigned long)-1;
		for( i=0,j=0; i<sizeof(unsigned long); i++,j-- )
			*(pdes+j)=*(psrc+i);
		
		pTemp=pHeader->sHMajV;
		pTemp=pTemp+3; *pTemp='1';	//set major version
		pTemp=pHeader->sHMinV;
		*pTemp='0';	//set min version
	}
	return pHeader;
}

//Delete allocate memory by CreateROMHeader()
void DeleteROMHeader( PROMHEADER pHeader )
{
	if( pHeader )
		free(pHeader);
}

//Set model name ro ROM-header
BOOL SetROMModelName( PROMHEADER pHeader,const char *psModelName )
{
	int i;
	if( pHeader==NULL || psModelName==NULL )
		return FALSE;

	for( i=0; i<16; i++ ){
		if( *(psModelName+i)==NULL )
			break;//exit for
		else
			*(pHeader->sBINModelName+i)=*(psModelName+i);
	}//end for
	return TRUE;
}

//Set version ro ROM-header
BOOL SetROMVersion( PROMHEADER pHeader,const char *psVersion )
{
	int i;
	if( pHeader==NULL || psVersion==NULL )
		return FALSE;

	for( i=0; i<16; i++ ){
		if( *(psVersion+i)==NULL )
			break;//exit for
		else
			*(pHeader->sBINV+i)=*(psVersion+i);
	}//end for
	return TRUE;
}

//Set Description to ROM-header
BOOL SetROMDescription( PROMHEADER pHeader,const char *psDes )
{
	int i;
	if( pHeader==NULL || psDes==NULL )
		return FALSE;

	for( i=0; i<16; i++ ){
		if( *(psDes+i)==NULL )
			break;//exit for
		else
			*(pHeader->sBINDesc+i)=*(psDes+i);
	}//end for
	return TRUE;
}

//Set CRC to ROM-header
BOOL SetROMSizeCRC( PROMHEADER pHeader,HANDLE hPureBIN )
{
	unsigned long lSize;
	int i,j;
	unsigned char *psrc,*pdes;
	unsigned char sReadbuffer[4];
	unsigned char cCRC;
	BOOL bResult;
	unsigned long dwBytesToRead,dwBytesRead;
	
	if( pHeader==NULL || hPureBIN==NULL )
		return FALSE;
	//Set pure BIN size
	lSize=GetFileSize( hPureBIN,NULL );
	//convert little endian to Big-endian
	psrc=(unsigned char *)&lSize;
	pdes=(unsigned char *)&pHeader->lBINSize;
	pdes=pdes+sizeof(unsigned long)-1;
	for( i=0,j=0; i<sizeof(unsigned long); i++,j-- )
		*(pdes+j)=*(psrc+i);
	
	//Set CRC code
	SetFilePointer(hPureBIN,0,NULL,FILE_BEGIN);
	dwBytesToRead=4;
	cCRC=0x00;
	
	while(1){
		memset( sReadbuffer,0x00,4 );
		// Attempt a synchronous read operation. 
		bResult=ReadFile(hPureBIN,sReadbuffer, dwBytesToRead, &dwBytesRead, NULL) ; 
		// Check for end of file. 
		if( bResult&&dwBytesRead==0 ){ 
			// we're at the end of the file
			break;//exit while
		}
		else{
			cCRC=sReadbuffer[0]^sReadbuffer[1]^sReadbuffer[2]^sReadbuffer[3];
		}
	}//end while
	
	pHeader->lBINCheckSum=(unsigned long)cCRC;
	return TRUE;
}

//Generate ROM file
BOOL GenerateROMFile( const char *pROMFileName,PROMHEADER pHeader,HANDLE hPureBIN )
{
	HANDLE hRom;
	BOOL bResult;
	unsigned long dwNumberOfBytesToWrite;// number of bytes to write
	unsigned long dwNumberOfBytesWritten;// number of bytes written
	unsigned char sReadbuffer[128];
	unsigned long dwBytesToRead,dwBytesRead;

	if( pROMFileName==NULL || pHeader==NULL )
		return FALSE;
	//Create target file
	hRom=CreateFile(
		(LPCTSTR)pROMFileName,
		GENERIC_WRITE,
		0,// share mode
		NULL, // SD
		CREATE_NEW,// how to create
		FILE_ATTRIBUTE_NORMAL,                 // file attributes
		NULL                        // handle to template file
	);
	if( hRom==INVALID_HANDLE_VALUE )
		return FALSE;
	//Save header
	dwNumberOfBytesToWrite=sizeof(ROMHEADER);

	bResult=WriteFile( hRom,pHeader,dwNumberOfBytesToWrite,&dwNumberOfBytesWritten,NULL );
	if( !bResult  ){
		CloseHandle( hRom );
		DeleteFile( (LPCTSTR)pROMFileName );
		return FALSE;
	}

	SetFilePointer(hPureBIN,0,NULL,FILE_BEGIN);
	dwBytesToRead=128;
	dwNumberOfBytesToWrite=128;

	while(1){
		memset( sReadbuffer,0x00,128 );
		// Attempt a synchronous read operation. 
		bResult=ReadFile(hPureBIN,sReadbuffer, dwBytesToRead, &dwBytesRead, NULL) ; 
		// Check for end of file. 
		if( bResult&&dwBytesRead==0 ){ 
			// we're at the end of the file
			break;//exit while
		}
		else{
			dwNumberOfBytesToWrite=dwBytesRead;
			bResult=WriteFile( hRom,sReadbuffer,dwNumberOfBytesToWrite,&dwNumberOfBytesWritten,NULL );
			if( !bResult ){
				FlushFileBuffers( hRom );
				CloseHandle( hRom );
				DeleteFile( (LPCTSTR)pROMFileName );
				return FALSE;
			}
		}
	}//end while

	FlushFileBuffers( hRom );
	CloseHandle( hRom );
	return TRUE;	
}


//Check pretag
BOOL IsCorrectROMFormat( HANDLE hROM )
{
	BOOL bResult;
	unsigned long dwBytesToRead,dwBytesRead;
	unsigned char sReadbuffer[10];

	SetFilePointer(hROM,0,NULL,FILE_BEGIN);

	dwBytesToRead=4;
	bResult=ReadFile(hROM,sReadbuffer, dwBytesToRead, &dwBytesRead, NULL) ; 
	if( !bResult )
		return FALSE;
	//
	if( sReadbuffer[0]==0xa5 &&
		sReadbuffer[1]==0xa5 &&
		sReadbuffer[2]==0xa5 &&
		sReadbuffer[3]==0xa5 
		)
		return TRUE;
	else
		return FALSE;
	
}

//get header from ROM file
BOOL GetROMHeader( PROMHEADER pHeader,HANDLE hROM )
{
	unsigned long dwBytesToRead,dwBytesRead;
	BOOL bResult;

	if( pHeader==NULL )
		return FALSE;
	//
	SetFilePointer(hROM,0,NULL,FILE_BEGIN);
	
	dwBytesToRead=sizeof(ROMHEADER);
	bResult=ReadFile(hROM,pHeader, dwBytesToRead, &dwBytesRead, NULL);
	if( bResult ){
		return TRUE;
	}
	else
		return FALSE;
}

//get Model name from ROM header
BOOL GetROMModelName( char *psModelName,PROMHEADER pHeader )
{
	if( pHeader==NULL )
		return FALSE;
		
	memset( psModelName,0x00,16 );
	memcpy( psModelName,pHeader->sBINModelName,16 );
	return TRUE;
}

//get ROM version from ROM header
BOOL GetROMVersion( char *psVersion,PROMHEADER pHeader )
{
	if( pHeader==NULL )
		return FALSE;
		
	memset( psVersion,0x00,16 );
	memcpy( psVersion,pHeader->sBINV,16 );
	return TRUE;
}

//get Model name from ROM description
BOOL GetROMDescription( char *psDes,PROMHEADER pHeader )
{
	if( pHeader==NULL )
		return FALSE;

	memset( psDes,0x00,16 );
	memcpy( psDes,pHeader->sBINDesc,16 );
	return TRUE;
}

//get pure BIN-part-size  from ROM header
unsigned long GetPureBINSize( PROMHEADER pHeader )
{
	unsigned char *psrc,*pdes;
	unsigned long lSize=0;
	int i,j;
	
	if( pHeader==NULL )
		return 0;//Error
		
	//convert Big-endian to little-endian
	psrc=(unsigned char *)&pHeader->lBINSize;
	pdes=(unsigned char *)&lSize;
	pdes=pdes+sizeof(unsigned long)-1;
	for( i=0,j=0; i<sizeof(unsigned long); i++,j-- )
		*(pdes+j)=*(psrc+i);
	
	return lSize;

}

//Check CRC of ROM file
BOOL IsCorrectROMCRC( PROMHEADER pHeader,HANDLE hROM )
{
	unsigned char cCRC;
	unsigned char sReadbuffer[4];
	BOOL bResult;
	unsigned long dwBytesToRead,dwBytesRead;
	
	if( pHeader==NULL )
		return FALSE;
		
	//calculate CRC code
	SetFilePointer(hROM,sizeof(ROMHEADER),NULL,FILE_BEGIN);
	dwBytesToRead=4;
	cCRC=0x00;
	
	while(1){
		memset( sReadbuffer,0x00,4 );
		// Attempt a synchronous read operation. 
		bResult=ReadFile(hROM,sReadbuffer, dwBytesToRead, &dwBytesRead, NULL) ; 
		// Check for end of file. 
		if( bResult&&dwBytesRead==0 ){ 
			// we're at the end of the file
			break;//exit while
		}
		else{
			cCRC=sReadbuffer[0]^sReadbuffer[1]^sReadbuffer[2]^sReadbuffer[3];
		}
	}//end while
	
	if( ((unsigned long)cCRC)==pHeader->lBINCheckSum )
		return TRUE;
	else
		return FALSE;
}
