
#include "stdafx.h"
#include "EL_Support.h"
#include "stdlib.h"
#include <time.h>
#include <stdio.h>
#include "TCHAR.h"

//#include "Windows.h"
//#include "Setupapi.h"
//#include "tchar.h"
//#include "newdev.h"

////dynamic exported function prototype.
//GetNativeSystemInfo(WinXP or vista)
typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO);
//UpdateDriverForPlugAndPlayDevices(WinMe ,Win2000 or later)
//typedef BOOL (WINAPI *UPDATEDRVPNPDEVICE)(HWND,LPCTSTR,LPCTSTR,DWORD,PBOOL);

///////////////////////////////////////////////////////////////////////
//local function prototype
// psARel [Out] : pointer of starting pointer of adjusted relative path
// return : the pointer of last character of adjusted base path
// psBase [IN] : tail pointer of base path. this value must be absolute-value.
// psRel [IN] : relative path. this value must be relative-value.
LPTSTR EL_GetAdjPathPos( LPTSTR *ppsARel,LPTSTR psBase,LPTSTR psRel );


HANDLE EL_CreateOneAppMutex(LPCTSTR lpName,BOOL bOwnerShip)
{
    HANDLE hMutex;
    TCHAR *psName;

	if( lpName )
    	hMutex = CreateMutex(NULL, bOwnerShip, lpName);   // Create mutex with given name.......
    else{
    	//generates automatically unique mutex name.
    	psName=(TCHAR*)malloc( sizeof(TCHAR)*256 );//max name size is 256
    	if( psName==NULL )
    		return NULL;
    	//
		srand( (unsigned)time( NULL ) );
    	EL_CreateRandomName( psName,rand(),GetTickCount() );
    	hMutex = CreateMutex(NULL, bOwnerShip, psName);   // Create mutex with given name.......
    }

	switch(GetLastError()){
		case ERROR_SUCCESS:
			// Mutex created successfully. There is no instances running
        	break;
		case ERROR_ALREADY_EXISTS:
			// Mutex already exists so there is a running instance of our app.
			hMutex = NULL;
			break;
		default:
			// Failed to create mutex by unkniown reason
			break;
	}//end switch

    return hMutex;//return mutex handle
}


//psString buffer' size minmum 128 bytes
void EL_CreateRandomName( TCHAR *psName,int nSeed0,int nSeed1 )
{
	if( psName==NULL )
		return;
	//
	_stprintf( psName,_T("UNIQUE_NAME_REQ_%X_%X_"),nSeed0,nSeed1 );
}

//check given event is existing or not.......
BOOL EL_AvailableEvent( TCHAR *sEventName )
{
	HANDLE hEvent;
	DWORD dwResult;
	BOOL bResult;
	
	hEvent=OpenEvent( SYNCHRONIZE,FALSE,sEventName );

	if( hEvent ){
		CloseHandle( hEvent );//decrease reference counter	
		bResult=TRUE;
	}
	else{
		dwResult=GetLastError();
		if( dwResult==ERROR_FILE_NOT_FOUND ){
			//Not found event
			bResult=FALSE;
			
		}
		else{//opening event error
			bResult=FALSE;
		}
	}
	return bResult;
}

//check given mutex is existing or not.......
BOOL EL_AvailableMutex( TCHAR *sMutexName )
{
	HANDLE hMutex;
	DWORD dwResult;
	BOOL bResult;
	
	hMutex=OpenMutex( SYNCHRONIZE,FALSE,sMutexName );

	if( hMutex ){
		CloseHandle( hMutex );//decrease reference counter	
		bResult=TRUE;
	}
	else{
		dwResult=GetLastError();
		if( dwResult==ERROR_FILE_NOT_FOUND ){
			//Not found event
			bResult=FALSE;
			
		}
		else{//opening event error
			bResult=FALSE;
		}
	}
	return bResult;
}

int EL_ConvertToDigit(const TCHAR *szNumber, int nType)
{
	const TCHAR* szTemp = szNumber;
	int nData = 0;
	int nPow = 1;
	int ii, nLen;

	nLen = _tcslen(szNumber);

	for ( ii = (nLen-1); ii >= 0; ii-- ){
		switch ( nType ){
			case 2:
				if ( szTemp[ii] < _T('0') || szTemp[ii] > _T('1') )
					return 0; // 잘못된 2진수
				nData += (szTemp[ii] - _T('0')) * nPow;
				nPow *= 2;
				break;
			case 8:
				if ( szTemp[ii] < _T('0') || szTemp[ii] > _T('8') )
					return 0; // 잘못된 8진수
				nData += (szTemp[ii] - _T('0')) * nPow;
				nPow *= 8;
				break;
			case 16:
				if ( (szTemp[ii]  < _T('0') && szTemp[ii] > _T('9')) &&
					 ((szTemp[ii] < _T('A') && szTemp[ii] > _T('F')) ||
					  (szTemp[ii] < _T('a') && szTemp[ii] > _T('f')))){
					return 0; // 잘못된 16진수
				}
				if ( szTemp[ii] >= _T('0') && szTemp[ii] <= _T('9') ){
					nData += (szTemp[ii] - _T('0')) * nPow;
				}
				if ( szTemp[ii] >= _T('A') && szTemp[ii] <= _T('F') ){
					nData += (szTemp[ii] - _T('A') + 10) * nPow;
				}
				if ( szTemp[ii] >= _T('a') && szTemp[ii] <= _T('f') ){
					nData += (szTemp[ii] - _T('a') + 10) * nPow;
				}

				nPow *= 16;
				break;
		}
	}
	return nData;
}

int EL_GetCurOSVersion()
{
	OSVERSIONINFOEX osvi;
	SYSTEM_INFO si;
	PGNSI pGNSI;
	BOOL bOsVersionInfoEx;

	HKEY hKey;
	char szProductType[80];
	DWORD dwBufLen=80;
	LONG lRet;
	int nResult=EL_CUR_OS_VERSION_INVALIED;

	// Try calling GetVersionEx using the OSVERSIONINFOEX structure.
	// If that fails, try using the OSVERSIONINFO structure.

	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	if( !(bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi)) ){
		osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
		if( !GetVersionEx( (OSVERSIONINFO *) &osvi) ) 
			return nResult;
	}


	switch (osvi.dwPlatformId){
		// Test for the Windows NT product family.
		case VER_PLATFORM_WIN32_NT:
			// Test for the specific product.

			if( osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 1 ){

				if( osvi.wProductType == VER_NT_WORKSTATION )
					nResult=EL_CUR_OS_VERSION_WIN7;
				else
					nResult=EL_CUR_OS_VERSION_WIN2008R2;
			}

			if( osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 0 ){
				/*
				if( osvi.wProductType == VER_NT_WORKSTATION )
					TraceOnly( 0,"Microsoft Windows Vista ");
				else
					TraceOnly( 0,"Windows Server \"Longhorn\" " );
				*/
				//
				if( osvi.wProductType == VER_NT_WORKSTATION )
					nResult=EL_CUR_OS_VERSION_WINVISTA;
				else
					nResult=EL_CUR_OS_VERSION_WIN2008;
			}


			if( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2 ){
				// Use GetProcAddress to avoid load issues on Windows 2000
				pGNSI = (PGNSI) GetProcAddress(
						GetModuleHandle(_T("kernel32.dll")), 
						(LPCSTR)"GetNativeSystemInfo");
				if(NULL != pGNSI)
					pGNSI(&si);

				if( GetSystemMetrics(SM_SERVERR2) ){
					//TraceOnly( 0,"Microsoft Windows Server 2003 \"R2\" ");
					nResult=EL_CUR_OS_VERSION_WIN2003;
				}
				else if( osvi.wProductType == VER_NT_WORKSTATION && si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64){
					//TraceOnly( 0,"Microsoft Windows XP Professional x64 Edition ");
					nResult=EL_CUR_OS_VERSION_WINXP;
				}
				else{
					//TraceOnly( 0,"Microsoft Windows Server 2003, ");
					nResult=EL_CUR_OS_VERSION_WIN2003;
				}
			}

			if( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1 ){
				//TraceOnly( 0,"Microsoft Windows XP ");
				nResult=EL_CUR_OS_VERSION_WINXP;
			}

			if( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0 ){
				//TraceOnly( 0,"Microsoft Windows 2000 ");
				nResult=EL_CUR_OS_VERSION_WIN2000;
			}

			if( osvi.dwMajorVersion <= 4 ){
				//TraceOnly( 0,"Microsoft Windows NT ");
				nResult=EL_CUR_OS_VERSION_WINNT;
			}

			// Test for specific product on Windows NT 4.0 SP6 and later.
			if( bOsVersionInfoEx ){
				// Test for the workstation type.
				if( osvi.wProductType == VER_NT_WORKSTATION && si.wProcessorArchitecture!=PROCESSOR_ARCHITECTURE_AMD64){
					/*
					if( osvi.dwMajorVersion == 4 )
						TraceOnly( 0,"Workstation 4.0 " );
					else if( osvi.wSuiteMask & VER_SUITE_PERSONAL )
						TraceOnly( 0,"Home Edition " );
					else
						TraceOnly( 0,"Professional " );
					*/
			}// Test for the server type.
			else if( osvi.wProductType == VER_NT_SERVER || osvi.wProductType == VER_NT_DOMAIN_CONTROLLER ){
				if(osvi.dwMajorVersion==5 && osvi.dwMinorVersion==2){
					if ( si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_IA64 ){
						/*
						if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
							TraceOnly( 0,"Datacenter Edition for Itanium-based Systems" );
						else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
							TraceOnly( 0,"Enterprise Edition for Itanium-based Systems" );
						*/
					}
					else if ( si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64 ){
						/*
						if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
							TraceOnly( 0,"Datacenter x64 Edition " );
						else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
							TraceOnly( 0,"Enterprise x64 Edition " );
						else
							TraceOnly( 0,"Standard x64 Edition " );
						*/
					}
					else{
						/*
						if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
							TraceOnly( 0,"Datacenter Edition " );
						else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
							TraceOnly( 0,"Enterprise Edition " );
						else if ( osvi.wSuiteMask == VER_SUITE_BLADE )
							TraceOnly( 0,"Web Edition " );
						else
							TraceOnly( 0,"Standard Edition " );
						*/
					}
				}
				else if(osvi.dwMajorVersion==5 && osvi.dwMinorVersion==0){
					/*
					if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
						TraceOnly( 0,"Datacenter Server " );
					else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
						TraceOnly( 0,"Advanced Server " );
					else
						TraceOnly( 0,"Server " );
					*/
	            }
				else{  // Windows NT 4.0 
					/*
					if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
						TraceOnly( 0,"Server 4.0, Enterprise Edition " );
					else
						TraceOnly( 0,"Server 4.0 " );
					*/
				}
			}
		}
		else{// Test for specific product on Windows NT 4.0 SP5 and earlier

			lRet = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
					_T("SYSTEM\\CurrentControlSet\\Control\\ProductOptions"),
					0, KEY_QUERY_VALUE, &hKey );
			if( lRet != ERROR_SUCCESS )
				return nResult;

			lRet = RegQueryValueEx( hKey, _T("ProductType"), NULL, NULL,
					(LPBYTE) szProductType, &dwBufLen);
			RegCloseKey( hKey );

			if( (lRet != ERROR_SUCCESS) || (dwBufLen > 80) )
				return nResult;
			/*
			if( lstrcmpi( "WINNT", szProductType) == 0 )
				TraceOnly( 0,"Workstation " );
			if( lstrcmpi( "LANMANNT", szProductType) == 0 )
				TraceOnly( 0,"Server " );
			if( lstrcmpi( "SERVERNT", szProductType) == 0 )
				TraceOnly( 0,"Advanced Server " );
			TraceOnly( 0,"%d.%d ", osvi.dwMajorVersion, osvi.dwMinorVersion );
			*/
		}

		// Display service pack (if any) and build number.
		if( osvi.dwMajorVersion == 4 && lstrcmpi( osvi.szCSDVersion, _T("Service Pack 6") ) == 0 ){ 

			// Test for SP6 versus SP6a.
			lRet = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
					_T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Hotfix\\Q246009"),
					0, KEY_QUERY_VALUE, &hKey );
			/*
			if( lRet == ERROR_SUCCESS )
				TraceOnly( 0,"Service Pack 6a (Build %d)\n",osvi.dwBuildNumber & 0xFFFF );         
			else{ // Windows NT 4.0 prior to SP6a
				TraceOnly( 0,"%s (Build %d)\n",osvi.szCSDVersion,osvi.dwBuildNumber & 0xFFFF);
			}
			*/

			RegCloseKey( hKey );
		}
		else{ // not Windows NT 4.0 
			//TraceOnly( 0,"%s (Build %d)\n",osvi.szCSDVersion,osvi.dwBuildNumber & 0xFFFF);
		}
		break;
		// Test for the Windows Me/98/95.
		case VER_PLATFORM_WIN32_WINDOWS:

			if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 0){
				//TraceOnly ( 0,"Microsoft Windows 95 ");
				nResult=EL_CUR_OS_VERSION_WIN95;
				/*
				if (osvi.szCSDVersion[1]=='C' || osvi.szCSDVersion[1]=='B')
					TraceOnly( 0,"OSR2 " );
				*/
			} 

			if(osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 10){
				//TraceOnly( 0,"Microsoft Windows 98 ");
				nResult=EL_CUR_OS_VERSION_WIN98;
				/*
				if( osvi.szCSDVersion[1]=='A' || osvi.szCSDVersion[1]=='B')
					TraceOnly( 0,"SE " );
				*/
			}

			if(osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 90){
				//TraceOnly( 0,"Microsoft Windows Millennium Edition\n");
				nResult=EL_CUR_OS_VERSION_WINME;
			} 
			break;
		case VER_PLATFORM_WIN32s:
			//TraceOnly( 0,"Microsoft Win32s\n");
			break;
	}//end switch
	return nResult; 
}

int EL_GetOSDefaultLanguage()
{
	long lDefLanInfo;
	int nLanguageCode=0;


	if( !SystemParametersInfo( SPI_GETDEFAULTINPUTLANG,0,&lDefLanInfo,0 ) ){
		return EL_CUR_OS_DEF_LANG_INVALIED;	//invalied language code
	}
	//
	lDefLanInfo=lDefLanInfo&0x0000FFFF;//make upper word

	if( 0x0412==lDefLanInfo || lDefLanInfo==0x0812 ){//Korean OS
		nLanguageCode=EL_CUR_OS_DEF_LANG_KOREAN;
	}
	else{//the other OS
		nLanguageCode=EL_CUR_OS_DEF_LANG_USA_ENGLISH;//considered by U.S language
	}
	return nLanguageCode;
}


//current modul path to current path of program
BOOL EL_SetModulPathToCurPath( HMODULE hModule )
{
	TCHAR Path_buffer[_MAX_PATH];
	TCHAR Drive[_MAX_DRIVE];
	TCHAR Dir[_MAX_DIR];

	if( GetModuleFileName(hModule, Path_buffer,_MAX_PATH )>=0 ){
		_tsplitpath( Path_buffer,Drive,Dir,NULL,NULL );
		_tmakepath( Path_buffer,Drive,Dir,NULL,NULL );

		return SetCurrentDirectory(Path_buffer);
	}
	else
		return FALSE;
}

//get Cuurent full path
//OUT : psFullPath - this string end of '\' character.
BOOL EL_GetCurFullPath( TCHAR *psFullPath,HMODULE hModule )
{
	TCHAR Path_buffer[_MAX_PATH];
	TCHAR Drive[_MAX_DRIVE];
	TCHAR Dir[_MAX_DIR];

	if( psFullPath==NULL )
		return FALSE;
	//
	if( GetModuleFileName(hModule, Path_buffer,_MAX_PATH )>=0 ){
		_tsplitpath( Path_buffer,Drive,Dir,NULL,NULL );
		_tmakepath( psFullPath,Drive,Dir,NULL,NULL );

		return TRUE;
	}
	else
		return FALSE;
}

// checks if or not psPath is .\?????
BOOL EL_IsThisPathCurrent( TCHAR *psPath )
{
	TCHAR Drive[_MAX_DRIVE];
	TCHAR Dir[_MAX_DIR];
   	TCHAR Fname[_MAX_FNAME];
	TCHAR Ext[_MAX_EXT];

	if( psPath==NULL )
		return FALSE;
	//
	_tsplitpath( psPath,Drive,Dir,Fname,Ext );

	//
	if( _tcslen( Drive )==0 ){

		//Driver letter is NULL
		if( _tcscmp( Dir,_T(".\\") )==0 ){
			//path is ".\"
			return TRUE;
		}
	}

	//
	return FALSE;

}

//check if or not the given path is relative path.
BOOL EL_IsThisRelativePath( const TCHAR *psPath )
{
	if( psPath==NULL )
		return FALSE;
	//
	if( psPath[0]==_T('.') )
		return TRUE;
	else
		return FALSE;
}

LPVOID EL_CreateShareFileMem( HANDLE *phOutMap,DWORD dwSize,LPCTSTR lpMemName )
{
	HANDLE hMapObj=NULL;
	LPVOID pvShared=NULL;
	BOOL bFirst;
	//
	if( dwSize==0 )
		return NULL;
	if( lpMemName==NULL )
		return NULL;
	if( phOutMap==NULL )
		return NULL;
		
	// Create a named file mapping object.
	hMapObj = CreateFileMapping(
				(HANDLE) 0xFFFFFFFF,  // Use paging file
				NULL,                 // No security attributes
				PAGE_READWRITE,       // Read/Write access
				0,                    // Mem Size: high 32 bits
				dwSize,          // Mem Size: low 32 bits
				lpMemName);   // Name of map object
	if( hMapObj!=NULL ){
		// Determine if this is the first create of the file mapping.
		bFirst=(ERROR_ALREADY_EXISTS != GetLastError());

		// Now get a pointer to the file-mapped shared memory.
		pvShared = MapViewOfFile(
					hMapObj,        // File Map obj to view
					FILE_MAP_WRITE,   // Read/Write access
					0,                // high: map from beginning
					0,                // low:
					0);               // default: map entire file
		if( pvShared!=NULL ){

			if(bFirst){
				// If this is the first attaching process, init the
				// shared memory.
				memset(pvShared,0,dwSize);
			}
		}
		
		*phOutMap=hMapObj;//save map handle for release mapfile
		return pvShared;
	}
	else
		return NULL;
}

//Open share file-map memory
LPVOID EL_OpenShareFileMem( HANDLE *phOutMap,LPCTSTR lpMemName )
{
	HANDLE hMapObj=NULL;
	LPVOID pvShared=NULL;
	//
	if( lpMemName==NULL )
		return NULL;
	if( phOutMap==NULL )
		return NULL;
		
	// Open a named file mapping object.
	hMapObj = OpenFileMapping(
			FILE_MAP_ALL_ACCESS,// access mode
			TRUE,				// inherit flag
			lpMemName			// object name
			);
	if( hMapObj==NULL )
		return NULL;
	//
	// Now get a pointer to the file-mapped shared memory.
	pvShared = MapViewOfFile(
				hMapObj,        // File Map obj to view
				FILE_MAP_WRITE,   // Read/Write access
				0,                // high: map from beginning
				0,                // low:
				0);               // default: map entire file
	if( pvShared==NULL ){
		CloseHandle(hMapObj);
		return NULL;
	}
	
	*phOutMap=hMapObj;//save map handle for release mapfile
	return pvShared;
}	


//this function is pair EL_CreateShareFileMem() and EL_OpenShareFileMem
BOOL EL_DeleteShareFileMem( HANDLE hMapObj,LPVOID pvShared )
{
	if( pvShared==NULL )
		return FALSE;
	if( hMapObj==INVALID_HANDLE_VALUE )
		return FALSE;
		
	// Unmap any shared memory from the process's address space.
	if( UnmapViewOfFile(pvShared)==FALSE )
		return FALSE;//
	// Close the process's handle to the file-mapping object.
	CloseHandle(hMapObj);

	return TRUE;
}


///////////////////////////////////////////////////////////////////////
//local function prototype
// psARel [Out] : pointer of starting pointer of adjusted relative path
// return : the pointer of last character of adjusted base path
// psBase [IN] : base path. this value must be absolute-value.
// psRel [IN] : relative path. this value must be relative-value.
LPTSTR EL_GetAdjPathPos( LPTSTR *ppsARel,LPTSTR psBase,LPTSTR psRel )
{
	int nCharSize=1;
	LPTSTR pAB=NULL;

	nCharSize=sizeof(TCHAR);

	if( *(psBase-nCharSize)==(_T(':')) ){
		if( *psRel==_T('.') ){
			if( *(psRel+nCharSize)==_T('.') )
				return NULL;//error
			else{
				pAB=psBase;
				*ppsARel=psRel;
				return pAB;
			}
		
		}
		else
			return NULL;
	}
	else{
		if( *psRel==_T('.') ){
			//
			if( *(psRel+nCharSize)==_T('.') ){
				psRel+=(nCharSize*3);

				do{
					psBase=psBase-nCharSize;
				}while( *psBase!=_T('\\') );
				
				pAB=EL_GetAdjPathPos( ppsARel,psBase,psRel );
				return pAB;
			}
			else{
				psRel+=(nCharSize*2);
				pAB=EL_GetAdjPathPos( ppsARel,psBase,psRel );
				return pAB;
			}
		}
		else{
			if( *psRel==_T('\\') ){
				psRel+=nCharSize;
			}
		
			*ppsARel=psRel;
			pAB=psBase;
			return pAB;
		}
	}
}


//get absolute path from base and relative paths.
// pAbs[OUT] : absolute path
// pBase[IN] : base absolute path
// pRel[IN] : relative path path
BOOL EL_GetAbsPath( LPTSTR pAbs,LPTSTR pBase,LPTSTR pRel )
{
	LPTSTR pEndBase=NULL;
	LPTSTR pStartRel=NULL;
	
	int nCharSize=1;
	int nCount=0;
	int nBaseCnt=0;
	int i=0;


	if( pAbs==NULL )
		return FALSE;
	//
	if( pBase==NULL )
		return FALSE;
	if( pRel==NULL ){
		_tcscpy( pAbs,pBase );
	}

	nCharSize=sizeof(TCHAR);
/*
	nBaseCnt=_tcsclen( pBase );
	nBaseCnt--;
	
	
	pEndBase=pBase+nBaseCnt*nCharSize;
*/	
	//find the end of string
	while( pBase[i]!=NULL ){
		i++;
	}//end while

	pEndBase=&pBase[i-1];

	pEndBase=EL_GetAdjPathPos( &pStartRel,pEndBase,pRel );
	if( pEndBase==NULL )
		return FALSE;
	/*
	nCount=(pEndBase-pBase)/nCharSize+1;
	
	_tcsncpy( pAbs,pBase,nCount );
	*(pAbs+nCount*nCharSize)=NULL;//make zero string
	*/
	memcpy( pAbs,pBase,(pEndBase-pBase)+nCharSize );
	*(pAbs+(pEndBase-pBase)+nCharSize)=NULL;//make zero string

	_tcscat( pAbs,pStartRel );
	return TRUE;
}

//check if or not file exsits.
BOOL EL_IsFileExistance( LPCTSTR lpFilePath )
{
	HANDLE hfile;

	if( lpFilePath==NULL )
		return FALSE;
	//
	hfile=CreateFile( lpFilePath,GENERIC_READ,FILE_SHARE_READ, NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL, NULL);
	if( hfile==INVALID_HANDLE_VALUE ){
		return FALSE;
	}
	CloseHandle(hfile);

	return TRUE;
}

//Time delay by event object.
void EL_DelayByEvent( DWORD dwTimeOut )
{
	HANDLE hDelayEvent=NULL;	
	
	hDelayEvent=CreateEvent( NULL,TRUE,FALSE,NULL );
	if( hDelayEvent==NULL ){
		return;//failure of creation event  
	}
	
	WaitForSingleObject( hDelayEvent,dwTimeOut );
	ResetEvent( hDelayEvent );	//clear event.
	
	CloseHandle( hDelayEvent );	//close handle
}

//first try open-mutex(increase reference count). and fail try create-mutex(increase reference count).
HANDLE EL_CreateMutex(	BOOL bInheritHandle,//// inheritance option for openning
						BOOL bInitialOwner,	// initial owner
						LPCTSTR lpName		// object name
						)
{
	HANDLE hMutexDup=NULL;
	HANDLE hNewMutex=NULL;
	
	//the first open-try
	hNewMutex=OpenMutex( MUTEX_ALL_ACCESS,bInheritHandle,lpName );
	if( hNewMutex==NULL ){
		if( GetLastError()==ERROR_FILE_NOT_FOUND ){
			
			//the second create-try
			hNewMutex=CreateMutex( NULL,bInitialOwner,lpName );
			if( hNewMutex==NULL )
				return hNewMutex;//CreateMutex-error
		}
		else
			return hNewMutex;//OpenMutex-error
	}
	else{
		/*
		DuplicateHandle(
			GetCurrentProcess(), 
			hNewMutex, 
			GetCurrentProcess(),
			&hMutexDup, 
			0,
			FALSE,
			DUPLICATE_SAME_ACCESS);
		*/
	}
	
	return hNewMutex;
}

//Create unique name event object by random.
HANDLE EL_CreateUniqueEvent( TCHAR *psOutEventName,int nSeed )
{
	HANDLE hEvent;
	DWORD dwResult;

	int i=0;

	if( psOutEventName==NULL )
		return NULL;
	//
	EL_CreateRandomName( psOutEventName,nSeed,GetTickCount() );

	do{
		hEvent=CreateEvent(NULL,FALSE,FALSE,(LPCTSTR)psOutEventName );
		if( hEvent==NULL ){//fail
			//already exist another type named-object.
		}
		else{//success
			dwResult=GetLastError();
			if( dwResult!=ERROR_ALREADY_EXISTS ){
				return hEvent;		
			}
			//here! already exist same type named-object.
			CloseHandle( hEvent );
		}
		
		i++;
		EL_CreateRandomName( psOutEventName,nSeed+i*1000,GetTickCount() );
		
	}while( i<6 );//end do-while

	return NULL;
	
}

//Create unique name mutex object by random.
HANDLE EL_CreateUniqueMutex( TCHAR *psOutMutexName,int nSeed )
{
	HANDLE hMutex;
	DWORD dwResult;

	int i=0;

	if( psOutMutexName==NULL )
		return NULL;
	//
	EL_CreateRandomName( psOutMutexName,nSeed,GetTickCount() );

	do{
		hMutex=CreateMutex(NULL,FALSE,(LPCTSTR)psOutMutexName );
		if( hMutex==NULL ){//fail
			//already exist another type named-object.
		}
		else{//success
			dwResult=GetLastError();
			if( dwResult!=ERROR_ALREADY_EXISTS ){
				return hMutex;		
			}
			//here! already exist same type named-object.
			CloseHandle( hMutex );
		}
		
		i++;
		EL_CreateRandomName( psOutMutexName,nSeed+i*1000,GetTickCount() );
		
	}while( i<6 );//end do-while

	return NULL;
	
}
