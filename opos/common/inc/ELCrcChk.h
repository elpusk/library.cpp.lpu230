// ELCrcChk.cpp : header file
// Library: Use Version.lib
// last update : 2008.5.16


#if !defined(__EL_CRC_CHECK_HEADER_20070120__)
#define __EL_CRC_CHECK_HEADER_20070120__


////////////////////////////////////////
//the support function
////////////////////////////////////////

///////////////////////////////
//Get Check-code of given file
//sFileName[IN] : file name
//pdwChkCode[OUT] : check code
//return : Processing result
BOOL EL_GetDWORD_ChkCode( unsigned long *pdwChkCode,const TCHAR *sFileName );

///////////////////////////////
//Get version string of given file
//lpDestFile[IN] : file name
//sVersion[OUT] : version buffer
//return : Processing result
BOOL EL_GetFileVersion( TCHAR *sVersion,const TCHAR *lpDestFile );

//////////////////////////////
//generates BCC code from byte stream
//pBuffer[IN] : byte stream pointer
//nSize[IN] : sizeof data
//return one-byte BCC code
BYTE EL_MakeBCC( LPBYTE pBuffer, int nSize );


void EL_ISO14443TypeBGetCrc( BYTE *Data, int Length,BYTE *TransmitFirst,BYTE *TransmitSecond );

#endif // !defined(__EL_CRC_CHECK_HEADER_20070120__)
