// artestDlg.h : header file
//

#if !defined(__FIRMWARE_HEADER__)
#define __FIRMWARE_HEADER__

#include <Windows.h>

////////////////////////////////////////
//the definition of ROM file heaer V1.0
////////////////////////////////////////
typedef	struct TagROMHEADER{
	unsigned char sPreTag[4];			//Header pretag. constant 0xa5a5a5a5
	unsigned char sHMajV[4];			//Header format major version.
	unsigned char sHMinV[4];			//Header format minor version.
	unsigned long lHSize;				//the size of Header = sizeof(ROMHEADER)
										//big endian format--;
	unsigned char sBINModelName[16];	//the model name of BIN file
	unsigned char sBINV[16];			//the version of BIN file
	unsigned char sBINDesc[16];			//the description of BIN file
	unsigned long lBINSize;				//the pure size of BIN part
										//big endian format--;
	unsigned long lBINCheckSum;			//the 8bits-checksum of BIN part
										//big endian format--;
}ROMHEADER, *LPROMHEADER,*PROMHEADER;


////////////////////////////////////////
//the support function of ROM file heaer
////////////////////////////////////////

//allocate memory and reset data field of ROM file heaer
PROMHEADER CreateROMHeader();

//Delete allocate memory by CreateROMHeader()
void DeleteROMHeader( PROMHEADER pHeader );

//Set model name ro ROM-header
BOOL SetROMModelName( PROMHEADER pHeader,const char *psModelName );

//Set version ro ROM-header
BOOL SetROMVersion( PROMHEADER pHeader,const char *psVersion );

//Set Description to ROM-header
BOOL SetROMDescription( PROMHEADER pHeader,const char *psDes );

//Set CRC to ROM-header
BOOL SetROMSizeCRC( PROMHEADER pHeader,HANDLE hPureBIN );

//Generate ROM file
BOOL GenerateROMFile( const char *pROMFileName,PROMHEADER pHeader,HANDLE hPureBIN );

//Check pretag
BOOL IsCorrectROMFormat( HANDLE hROM );

//get header from ROM file
BOOL GetROMHeader( PROMHEADER pHeader,HANDLE hROM );

//get Model name from ROM header
BOOL GetROMModelName( char *psModelName,PROMHEADER pHeader );

//get Model name from ROM version
BOOL GetROMVersion( char *psVersion,PROMHEADER pHeader );

//get Model name from ROM description
BOOL GetROMDescription( char *psDes,PROMHEADER pHeader );

//get pure BIN-part-size  from ROM header
unsigned long GetPureBINSize( PROMHEADER pHeader );

//Check CRC of ROM file
BOOL IsCorrectROMCRC( PROMHEADER pHeader,HANDLE hROM );


#endif // !defined(__FIRMWARE_HEADER__)
