//EL_Support.h
//this file contains support function
//
//last update 2008.06.05 - EL_GetAbsPath() - fix bug.......
//last update 2008.06.09 - EL_CreateMutex() - fix bug.......
//////////////////////////////////////////////////////////


#if !defined(__EL_SUPPORT_HEADER_20070712__)
#define __EL_SUPPORT_HEADER_20070712__


#if !defined(SM_SERVERR2)
#define SM_SERVERR2						89
#endif

////////////////////////////////////////////////
//1. include file
////////////////////////////////////////////////


////////////////////////////////////////////////
//2. define constatns and type
////////////////////////////////////////////////
//EL_GetCurOSVersion  return value
#define	EL_CUR_OS_VERSION_INVALIED		-1
#define	EL_CUR_OS_VERSION_WIN95			100
#define	EL_CUR_OS_VERSION_WIN98			200
#define	EL_CUR_OS_VERSION_WINNT			300
#define	EL_CUR_OS_VERSION_WINME			400
#define	EL_CUR_OS_VERSION_WIN2000		500
#define	EL_CUR_OS_VERSION_WINXP			600
#define	EL_CUR_OS_VERSION_WIN2003		700
#define	EL_CUR_OS_VERSION_WINVISTA		800

#define	EL_CUR_OS_VERSION_WIN2008		900
#define	EL_CUR_OS_VERSION_WIN2008R2		1000
#define	EL_CUR_OS_VERSION_WIN7			1100


//EL_GetOSDefaultLanguage return value
#define	EL_CUR_OS_DEF_LANG_INVALIED		-1
#define	EL_CUR_OS_DEF_LANG_KOREAN		1
#define	EL_CUR_OS_DEF_LANG_USA_ENGLISH	2

////////////////////////////////////////////////
//3. the definition of data structure
////////////////////////////////////////////////

////////////////////////////////////////////////
//4. the definition of supported function prototype
////////////////////////////////////////////////

//Create mutex fotr single-instances
//lpName can be NULL. if this parameter is NULL,
//function generates this the name of mutex automatically.
HANDLE EL_CreateOneAppMutex(LPCTSTR lpName,BOOL bOwnerShip);

//generate random name
//psString buffer' size minmum 128 bytes
//nSeed0 and nSeed1 is seed variable. 
//this parameters can be a value.
void EL_CreateRandomName( TCHAR *psName,int nSeed0,int nSeed1 );

//check given event(named event) is existing or not.......
BOOL EL_AvailableEvent( TCHAR *sEventName );

//check given mutex is existing or not.......
BOOL EL_AvailableMutex( TCHAR *sMutexName );

//convert string to digit
//nType = 2,8,16
int EL_ConvertToDigit(const TCHAR *szNumber, int nType);

//Get current OS version
int EL_GetCurOSVersion();

//Get current Default language of system
int EL_GetOSDefaultLanguage();

//current modul path to current path of program
BOOL EL_SetModulPathToCurPath( HMODULE hModule );

//get Cuurent full path
//OUT : psFullPath - this string end of '\' character.
BOOL EL_GetCurFullPath( TCHAR *psFullPath,HMODULE hModule );

// checks if or not psPath is .\?????
BOOL EL_IsThisPathCurrent( TCHAR *psPath );

//check if or not the given path is relative path.
BOOL EL_IsThisRelativePath( const TCHAR *psPath );

//create filemap memory
LPVOID EL_CreateShareFileMem( HANDLE *phOutMap,DWORD dwSize,LPCTSTR lpMemName );

//open filemap memory
LPVOID EL_OpenShareFileMem( HANDLE *phOutMap,LPCTSTR lpMemName );

//this function is pair EL_CreateShareFileMem() and EL_OpenShareFileMem
//delete filemap memory
BOOL EL_DeleteShareFileMem( HANDLE hMapObj,LPVOID pvShared );

//get absolute path from base and relative paths.
// pAbs[OUT] : absolute path
// pBase[IN] : base absolute path
// pRel[IN] : relative path path
BOOL EL_GetAbsPath( LPTSTR pAbs,LPTSTR pBase,LPTSTR pRel );

//check if or not file exsits.
BOOL EL_IsFileExistance( LPCTSTR lpFilePath );

//Time delay by event object.
void EL_DelayByEvent( DWORD dwTimeOut );

//first try open-mutex(increase reference count). and fail try create-mutex(increase reference count).
HANDLE EL_CreateMutex(	BOOL bInheritHandle,//// inheritance option for openning
						BOOL bInitialOwner,	// initial owner
						LPCTSTR lpName		// object name
						);

//Create unique name event object by random.
HANDLE EL_CreateUniqueEvent( TCHAR *psOutEventName,int nSeed );

//Create unique name mutex object by random.
HANDLE EL_CreateUniqueMutex( TCHAR *psOutMutexName,int nSeed );

////////////////////////////////////////////////
#endif//__EL_SUPPORT_HEADER_20070712__

//the enf of file