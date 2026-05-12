
#if !defined(__TTRL_ETC_HEADER_20091103__)
#define __TTRL_ETC_HEADER_20091103__

/////////////////////////////
//TTR serise etc header file
/////////////////////////////

//////////////////
//1. include file
//////////////////
#include "TTR_tchar.h"
#include <list>

///////////////////////////////
//2. define constatns and type
///////////////////////////////
#define	TTR_ETC_NAME_MAX_SIZE			128

//TTR_GetCurOSVersion  return value
#define	TTR_CUR_OS_VERSION_INVALIED		-1
#define	TTR_CUR_OS_VERSION_WIN95		10
#define	TTR_CUR_OS_VERSION_WIN98		20
#define	TTR_CUR_OS_VERSION_WINNT		30
#define	TTR_CUR_OS_VERSION_WINME		40
#define	TTR_CUR_OS_VERSION_WIN2000		50
#define	TTR_CUR_OS_VERSION_WINXP		60
#define	TTR_CUR_OS_VERSION_WIN2003		70
#define	TTR_CUR_OS_VERSION_WINVISTA		80
#define	TTR_CUR_OS_VERSION_WIN2008		90
#define	TTR_CUR_OS_VERSION_WIN7			100
#define	TTR_CUR_OS_VERSION_WIN2008R2	110



////////////////////////////////////////////
//Specifies packing alignment for structure
#pragma pack(push,8)

typedef	struct _TTR_MMF{	//entity of circlur command -queue.
	
	BOOL bIsCreate;				// [out]TRUE - Create new map, FALSE - Open already map
	HANDLE hMapObj;				// [out] file mapping object.
	LPVOID pMem;				// [out] pointer to the file-mapped shared memory.
	UINT32 dwSize;				// [in] the size of memory 
	TCHAR sName[TTR_ETC_NAME_MAX_SIZE];		// [in] Name of map object

} TTR_MMF, *PTTR_MMF,*LPTTR_MMF;

#pragma pack(pop)


// 
// pMmf[in,out]
// return value == pMmf->pMem
// faile return NULL
PVOID	TTR_CreateFileMem( LPTTR_MMF pMmf );

// 
// pMmf[in,out]
// pMmf->sName will be auto generated.
// return value == pMmf->pMem
// faile return NULL
PVOID	TTR_CreateFileMemAutoName( LPTTR_MMF pMmf,INT nSeed );

// pMmf[in]
VOID	TTR_DeleteFileMem( LPTTR_MMF pMmf );

VOID	TTR_CreateRandomName( LPTSTR psName,INT nSeed0,INT nSeed1 );

//Create unique name event object by random.
HANDLE	TTR_CreateUniqueEvent( LPTSTR psOutEventName,INT nSeed );

//Create unique name mutex object by random.
HANDLE	TTR_CreateUniqueMutex( LPTSTR psOutMutexName,INT nSeed );

//first try open-mutex(increase reference count). and fail try create-mutex(increase reference count).
HANDLE TTR_CreateMutex(	BOOL bInheritHandle,//// inheritance option for openning
						BOOL bInitialOwner,	// initial owner
						LPCTSTR lpName		// object name
						);

//Create Mutex for single instance
HANDLE TTR_CreateOneAppMutex(LPCTSTR lpName,BOOL bOwnerShip);

//check given event is existing or not.......
BOOL TTR_AvailableEvent( LPCTSTR sEventName );

//check given mutex is existing or not.......
BOOL TTR_AvailableMutex( LPCTSTR sMutexName );

INT TTR_ConvertToDigit( LPCTSTR szNumber, INT nType);

//Get abs-path from base path and relative path
VOID TTR_GetAbsPathFromRelPath( LPTSTR psOutAbs,INT nOutAbs, LPCTSTR psInBase, LPCTSTR psInRel);

//Get abs-path from base path and relative path in given Module as base
VOID TTR_GetAbsPathFromRelPathInModule( LPTSTR psOutAbs,INT nOutAbs, HMODULE hModule, LPCTSTR psInRel);

//Exist or absent file ?
BOOL TTR_IsExistFile( LPCTSTR psFilePath );

//get positive integer or zero from inf file's key
//if return negative, may be error
INT TTR_GetPosZeroIntegerFromInfKey( LPCTSTR psSection,LPCTSTR psKey,LPCTSTR psInfFile);

//get string value from inf file's key
BOOL TTR_GetStringFromInfKey( LPTSTR psVal,LPCTSTR psSection,LPCTSTR psKey,LPCTSTR psInfFile);

//return Cutted digit as nWith
//if  nSrc = 234 & nWidth = 2, return 34
INT TTR_GetCuttedDigit( INT nSrc,INT nWidth );
/////////////////////////////////////////

//Create path with multi level
BOOL TTR_CreatePath(_tstring &sPath);

//find file or folder. with subfolder or not
BOOL TTR_IsExistRecursive( _tstring sFindFileName,_tstring sPathFind,BOOL bRecursive,BOOL bIsFolder );

VOID TTR_GetFoundList( std::list<_tstring> & m_listFoundFile,_tstring sFindFileName,_tstring sPathFind,BOOL bRecursive,BOOL bIsFolder );

// Unicode version of GetProcAddress() windows API
FARPROC TTR_GetProcAddress( HMODULE hModule, LPCTSTR lpctProcName );

//Get current OS version
INT TTR_GetCurOSVersion();

//Get current Default language of system
INT TTR_GetOSDefaultLanguage();

//time delay  unit : 100nsec
//constant sytle : 1 sec = 10000000LL
VOID TTR_Delay( LONGLONG llTime );

//make multi zero-string with given zero-strings
//return the numebr of data(BYTE) in multistring(including NULL )
INT TTR_MakeMultiString( INT nCnt, LPTSTR szMultiStr, ... );

#endif	//__TTRL_ETC_HEADER_20091103__