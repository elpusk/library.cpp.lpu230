#include "stdafx.h"
#include "TTR_etc.h"
#include "tchar.h"



#if !defined(SM_SERVERR2)
#define SM_SERVERR2						89
#endif

////dynamic exported function prototype.
//GetNativeSystemInfo(WinXP or vista)
typedef VOID (WINAPI *PGNSI)(LPSYSTEM_INFO);

// 
// pMmf[in,out]
// return value == pMmf->pMem
// faile return NULL
PVOID	TTR_CreateFileMem( LPTTR_MMF pMmf )
{
	HANDLE hMapObj=NULL;
	LPVOID pvShared=NULL;
	BOOL bFirst;
	LPTSTR lpMemName=NULL;
	//

	if( pMmf==NULL )
		return NULL;
	if( pMmf->dwSize==0 )
		return NULL;
	if( pMmf->sName[0]!=NULL )
		lpMemName = pMmf->sName;

		
	// Create a named file mapping object.
	hMapObj = CreateFileMapping(
				INVALID_HANDLE_VALUE,  // Use paging file
				NULL,                 // No security attributes
				PAGE_READWRITE,       // Read/Write access
				0,                    // Mem Size: high 32 bits
				pMmf->dwSize,          // Mem Size: low 32 bits
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

			pMmf->hMapObj=hMapObj;//save map handle for release mapfile
			pMmf->pMem=pvShared;

			if( bFirst ){
				pMmf->bIsCreate=TRUE;
				memset(pvShared,0,pMmf->dwSize);
			}
			else
				pMmf->bIsCreate=FALSE;

			return pvShared;
		}
		else{
			CloseHandle(hMapObj);
			return NULL;
		}
		

	}
	else
		return NULL;

}


// 
// pMmf[in,out]
// pMmf->sName will be auto generated.
// return value == pMmf->pMem
// faile return NULL
PVOID	TTR_CreateFileMemAutoName( LPTTR_MMF pMmf,INT nSeed )
{
	INT i =0;
	HANDLE hMapObj=NULL;
	LPVOID pvShared=NULL;
	BOOL bFirst;
	LPTSTR lpMemName=NULL;

	if( pMmf==NULL )
		return NULL;
	if( pMmf->dwSize==0 )
		return NULL;

	lpMemName = pMmf->sName;

	//
	TTR_CreateRandomName( lpMemName,nSeed,GetTickCount() );

	do{
		// Create a named file mapping object.
		hMapObj = CreateFileMapping(
					(HANDLE) 0xFFFFFFFF,  // Use paging file
					NULL,                 // No security attributes
					PAGE_READWRITE,       // Read/Write access
					0,                    // Mem Size: high 32 bits
					pMmf->dwSize,          // Mem Size: low 32 bits
					lpMemName);   // Name of map object
		if( hMapObj!=NULL ){
			// Determine if this is the first create of the file mapping.
			bFirst=(ERROR_ALREADY_EXISTS != GetLastError());

			if( bFirst ){
				// Now get a pointer to the file-mapped shared memory.
				pvShared = MapViewOfFile(
							hMapObj,        // File Map obj to view
							FILE_MAP_WRITE,   // Read/Write access
							0,                // high: map from beginning
							0,                // low:
							0);               // default: map entire file
				if( pvShared!=NULL ){

					pMmf->hMapObj=hMapObj;	//save map handle for release mapfile
					pMmf->pMem=pvShared;

					memset(pvShared,0,pMmf->dwSize);
					pMmf->bIsCreate=TRUE;	//always true.

					return pvShared;
				}
				else{
					CloseHandle(hMapObj);//Error
					return NULL;
				}
			}
			else{
				//here! already exist same type named-object.
				CloseHandle(hMapObj);//....... retry
			}
			

		}
		else{//Error
			return NULL;
		}

		i++;
		TTR_CreateRandomName( lpMemName,nSeed+i*700,GetTickCount() );

	}while( i<7 );//end do-while

	return NULL;
}


// pMmf[in]
VOID	TTR_DeleteFileMem( LPTTR_MMF pMmf )
{
	if( pMmf==NULL )
		return;

	//
	if( pMmf->pMem ){

		if( UnmapViewOfFile(pMmf->pMem) ){
			CloseHandle(pMmf->hMapObj);
		}
	}

}


//psName buffer' size minmum 128 bytes
VOID	TTR_CreateRandomName( LPTSTR psName,INT nSeed0,INT nSeed1 )
{
	if( psName==NULL )
		return;
	//
	_stprintf( psName,_T("TTR_UNIQUE_NAME_IS_%X_%X_"),nSeed0,nSeed1 );
}

//Create unique name event object by random.
HANDLE TTR_CreateUniqueEvent( LPTSTR psOutEventName,INT nSeed )
{
	HANDLE hEvent;
	DWORD dwResult;

	INT i=0;

	if( psOutEventName==NULL )
		return NULL;
	//
	TTR_CreateRandomName( psOutEventName,nSeed,GetTickCount() );

	do{
		hEvent=CreateEvent(NULL,TRUE,FALSE,(LPCTSTR)psOutEventName );
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
		TTR_CreateRandomName( psOutEventName,nSeed+i*379,GetTickCount() );
		
	}while( i<50 );//end do-while

	return NULL;
	
}

//Create unique name mutex object by random.
HANDLE TTR_CreateUniqueMutex( LPTSTR psOutMutexName,INT nSeed )
{
	HANDLE hMutex;
	DWORD dwResult;

	INT i=0;

	if( psOutMutexName==NULL )
		return NULL;
	//
	TTR_CreateRandomName( psOutMutexName,nSeed,GetTickCount() );

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
		TTR_CreateRandomName( psOutMutexName,nSeed+i*438,GetTickCount() );
		
	}while( i<50 );//end do-while

	return NULL;
	
}

//first try open-mutex(increase reference count). and fail try create-mutex(increase reference count).
HANDLE TTR_CreateMutex(	BOOL bInheritHandle,// inheritance option for openning
						BOOL bInitialOwner,	// initial owner
						LPCTSTR lpName		// object name
						)
{
	HANDLE hNewMutex=NULL;
	
	if( lpName ){
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
	}
	else{
		hNewMutex=CreateMutex( NULL,bInitialOwner,lpName );
	}
	
	return hNewMutex;
}


HANDLE TTR_CreateOneAppMutex(LPCTSTR lpName,BOOL bOwnerShip)
{
    HANDLE hMutex = NULL;

	if( lpName )
    	hMutex = CreateMutex(NULL, bOwnerShip, lpName);   // Create mutex with given name.......
    else{
		return hMutex;//return mutex handle
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

//check given event is existing or not.......
BOOL TTR_AvailableEvent( LPCTSTR sEventName )
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
BOOL TTR_AvailableMutex( LPCTSTR sMutexName )
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

INT TTR_ConvertToDigit( LPCTSTR szNumber, INT nType)
{
	LPCTSTR szTemp = szNumber;
	INT nData = 0;
	INT nPow = 1;
	INT ii, nLen;

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


//Get abs-path from base path and relative path
VOID TTR_GetAbsPathFromRelPath( LPTSTR psOutAbs,INT nOutAbs, LPCTSTR psInBase, LPCTSTR psInRel)
{
	LPTSTR pCur;

	if( psOutAbs==NULL )
		return;

	//
	pCur = _tgetcwd(NULL, 0);	//save current dirct

	_tchdir( psInBase );	//change current directory
	_tfullpath( psOutAbs, psInRel, nOutAbs );

	_tchdir( pCur );			//recover current directory
	free( pCur );

}

//Get abs-path from base path and relative path in given Module as base
VOID TTR_GetAbsPathFromRelPathInModule( LPTSTR psOutAbs,INT nOutAbs, HMODULE hModule, LPCTSTR psInRel)
{
	TCHAR sModuleFN[_MAX_PATH];
	TCHAR Drive[_MAX_DRIVE];
	TCHAR Dir[_MAX_DIR];
   	TCHAR Fname[_MAX_FNAME];
	TCHAR Ext[_MAX_EXT];
	TCHAR sBaseFolder[_MAX_PATH];

	if( psOutAbs==NULL )
		return;
	if( hModule == NULL )
		return;

	//
	if( GetModuleFileName( hModule,sModuleFN,_MAX_PATH )>0 ){
		//generate realtive from folder to abs folder
		_tsplitpath( sModuleFN,Drive,Dir,Fname,Ext );
		_tmakepath( sBaseFolder,Drive,Dir,NULL,NULL );

		TTR_GetAbsPathFromRelPath( psOutAbs,nOutAbs, sBaseFolder, psInRel);
	}

}

//Exist or absent file ?
BOOL TTR_IsExistFile( LPCTSTR psFilePath )
{
	BOOL bResult = TRUE;
	HANDLE hfile;

	if( psFilePath ){

		hfile=CreateFile(psFilePath,GENERIC_READ,FILE_SHARE_READ, NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL, NULL);
		if( hfile==INVALID_HANDLE_VALUE ){

			bResult = FALSE;
		}
		else{
			CloseHandle(hfile);
		}

	}
	else
		bResult = FALSE;
	
	return bResult;
}


//get positive integer or zero from inf file's key
//if return negative, may be error
INT TTR_GetPosZeroIntegerFromInfKey( LPCTSTR psSection,LPCTSTR psKey,LPCTSTR psInfFile)
{
	INT nResult = -1;
	DWORD dwResult =0;
	TCHAR strBuf[_MAX_PATH];
#ifdef UNICODE 
	CHAR strTmp[2*(_MAX_PATH+1)];
#endif

	if( psSection!=NULL && psKey!=NULL && psInfFile!=NULL ){


		dwResult=GetPrivateProfileString( psSection,psKey,_T("-1"),strBuf,_MAX_PATH,psInfFile);
		if( dwResult>0 ){
		//
		#ifdef UNICODE
			wcstombs(strTmp, (const wchar_t *)strBuf, sizeof(strTmp)); 
			nResult=atoi( strTmp );
		#else
			nResult=atoi( strBuf );
		#endif
		}

	}

	return nResult;
}


//get string value from inf file's key
BOOL TTR_GetStringFromInfKey( LPTSTR psVal,LPCTSTR psSection,LPCTSTR psKey,LPCTSTR psInfFile)
{
	BOOL bResult = FALSE;
	DWORD dwResult =0;
	TCHAR strBuf[_MAX_PATH];

	if( psVal!=NULL && psSection!=NULL && psKey!=NULL && psInfFile!=NULL ){

		dwResult=GetPrivateProfileString( psSection,psKey,_T("-1"),strBuf,_MAX_PATH,psInfFile);
		if( dwResult>0 ){
			//
			_tcscpy( psVal,strBuf );
			bResult = TRUE;
		}
	}

	return bResult;
}


//return Cutted digit as nWith
//if  nSrc = 234 & nWidth = 2, return 34
INT TTR_GetCuttedDigit( INT nSrc,INT nWidth )
{
	INT nResult = 0;
	double dSrc;
	INT nTemp;
	double dDiv;
	INT nMul;
	INT i;
	double dRemover;
	BOOL bIsPositive;
	

	if( nSrc!=0 ){

		if( nWidth==0 ){
			nResult = nSrc;//no cut in width is zero case.......
		}
		else if( nWidth<17 && nWidth>0 ){
			
			dDiv = 1.0;
			nMul = 1;

			if( nSrc<0 ){
				bIsPositive = FALSE;

				nSrc = -nSrc;	//covert to positive
			}
			else
				bIsPositive = TRUE;

			//
			dRemover = dSrc = (double)nSrc;

			//generate cutter
			for( i=0; i<nWidth; i++ ){
				dDiv *= 10.0;
				nMul *= 10;
			}//end for
			 
			dRemover = dRemover / dDiv;
			nTemp = (INT)dRemover;

			nTemp *= nMul;
			dRemover = (double)nTemp;
				
			//cut hight digit
			dSrc = dSrc - dRemover;
			nResult = (INT)dSrc;

			if( !bIsPositive ){
				//recover sign
				nResult = -nResult;

			}

		}

	}

	return nResult;

}


BOOL TTR_CreatePath(_tstring &sPath)
{
	DWORD attr;
	INT pos;
	BOOL bResult = TRUE;

	// Check for trailing slash:
	pos = sPath.find_last_of(_T("\\"));
	if (sPath.length() == pos + 1){	// last character is "\"

		sPath.resize(pos);
	}

	// Look for existing object:
	attr = GetFileAttributes(sPath.c_str());
	if (0xFFFFFFFF == attr){	// doesn't exist yet - create it!

		pos = sPath.find_last_of(_T("\\"));
		if (0 < pos){
			// Create parent dirs:
			bResult = TTR_CreatePath(sPath.substr(0, pos));
		}
		// Create node:
		bResult = bResult && CreateDirectory(sPath.c_str(), NULL);
	}
	else if (FILE_ATTRIBUTE_DIRECTORY != attr){	// object already exists, but is not a dir
		SetLastError(ERROR_FILE_EXISTS);
		bResult = TRUE;
	}

	return bResult;
}


BOOL TTR_IsExistRecursive( _tstring sFindFileName,_tstring sPathFind,BOOL bRecursive,BOOL bIsFolder )
{
	BOOL next;
	HANDLE FileSearching;
	WIN32_FIND_DATA FindFileData;
	_tstring stemp;
	BOOL bFindOK = FALSE;

	// Add slash to path if it absent
	if(sPathFind[sPathFind.length()]!= _T('\\')){
		sPathFind+=_T("\\");
	}

	// search for any file
	stemp=sPathFind + _T("*.*");

	//open handle
	FileSearching=FindFirstFile(
		stemp.c_str(),    // pointer to name of file to search for
		&FindFileData     // pointer to returned information
		);

	if(next=(FileSearching  != INVALID_HANDLE_VALUE)){
		do{

			if( bRecursive ){
				//call self  if find directory(FILE_ATTRIBUTE_DIRECTORY)
				if((FILE_ATTRIBUTE_DIRECTORY & FindFileData.dwFileAttributes) &&
					_tcscmp(FindFileData.cFileName,_T(".")) &&
					_tcscmp(FindFileData.cFileName,_T(".."))){
					//
					bFindOK = TTR_IsExistRecursive (sFindFileName,sPathFind+FindFileData.cFileName,bRecursive,bIsFolder);
				}
			}

			/////////////////////////
			// to do.
			
			if( bIsFolder ){
				//search is folder
				if( FILE_ATTRIBUTE_DIRECTORY & FindFileData.dwFileAttributes ){

					if( _tcscmp( sFindFileName.c_str(),FindFileData.cFileName )==0 ){
						bFindOK = TRUE;
					}
				}
			}
			else{
				//file
				if( !(FILE_ATTRIBUTE_DIRECTORY & FindFileData.dwFileAttributes) ){
					if( _tcscmp( sFindFileName.c_str(),FindFileData.cFileName )==0 ){
						bFindOK = TRUE;
					}
				}
			}
			
			if( bFindOK ){
				FindClose(FileSearching);
				return bFindOK;
			}


			next=FindNextFile( FileSearching,&FindFileData );	// do search

			//do not freeze form till search continuing
			//Application->ProcessMessages();
		}while(next);

	}
	
	FindClose(FileSearching);
	//Search in the subdir - end
	return bFindOK;
}

VOID TTR_GetFoundList( std::list<_tstring> & m_listFoundFile,_tstring sFindFileName,_tstring sPathFind,BOOL bRecursive,BOOL bIsFolder )
{
	BOOL next;
	HANDLE FileSearching;
	WIN32_FIND_DATA FindFileData;
	_tstring stemp;

	// Add slash to path if it absent
	if(sPathFind[sPathFind.length()]!= _T('\\')){
		sPathFind+=_T("\\");
	}

	// search for any file
	stemp=sPathFind + sFindFileName;

	//open handle
	FileSearching=FindFirstFile(
		stemp.c_str(),    // pointer to name of file to search for
		&FindFileData     // pointer to returned information
		);

	if(next=(FileSearching  != INVALID_HANDLE_VALUE)){
		do{

			if( bRecursive ){
				//call self  if find directory(FILE_ATTRIBUTE_DIRECTORY)
				if((FILE_ATTRIBUTE_DIRECTORY & FindFileData.dwFileAttributes) &&
					_tcscmp(FindFileData.cFileName,_T(".")) &&
					_tcscmp(FindFileData.cFileName,_T(".."))){
					//
					TTR_GetFoundList(m_listFoundFile,sFindFileName,sPathFind+FindFileData.cFileName,bRecursive,bIsFolder);
				}
			}

			/////////////////////////
			// to do.
			
			if( bIsFolder ){
				//search is folder
				if( FILE_ATTRIBUTE_DIRECTORY & FindFileData.dwFileAttributes ){

					m_listFoundFile.push_back( FindFileData.cFileName );

				}
			}
			else{
				//file
				if( !(FILE_ATTRIBUTE_DIRECTORY & FindFileData.dwFileAttributes) ){

					m_listFoundFile.push_back( FindFileData.cFileName );
				}
			}
			

			next=FindNextFile( FileSearching,&FindFileData );	// do search

			//do not freeze form till search continuing
			//Application->ProcessMessages();
		}while(next);

	}
	
	FindClose(FileSearching);
	//Search in the subdir - end
}

FARPROC TTR_GetProcAddress( HMODULE hModule, LPCTSTR lpctProcName )
{
	INT nNeeded;
	FARPROC pFun = NULL;
	CHAR *lpszAnsiFn;


#if defined(_UNICODE) || defined(UNICODE)
	nNeeded = WideCharToMultiByte( CP_ACP,0,lpctProcName,-1,NULL,0,NULL,NULL );
	lpszAnsiFn = new CHAR[nNeeded];

	if( lpszAnsiFn ){
		WideCharToMultiByte( CP_ACP,0,lpctProcName,-1,lpszAnsiFn,nNeeded,NULL,NULL );
		pFun = GetProcAddress( hModule,lpszAnsiFn );
	}

	delete( lpszAnsiFn );
#else
	pFun = GetProcAddress( hModule,lpctProcName);
#endif

	return pFun;
}


INT TTR_GetCurOSVersion()
{
	OSVERSIONINFOEX osvi;
	SYSTEM_INFO si;
	PGNSI pGNSI;
	BOOL bOsVersionInfoEx;

	HKEY hKey;
	TCHAR szProductType[80];
	DWORD dwBufLen=80;
	LONG lRet;
	INT nResult=TTR_CUR_OS_VERSION_INVALIED;

	// Try calling GetVersionEx using the OSVERSIONINFOEX structure.
	// If that fails, try using the OSVERSIONINFO structure.

	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	if( !(bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi)) ){
		osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
		if( !GetVersionEx( (OSVERSIONINFO *) &osvi) ) 
			return nResult;
	}

	// Use GetProcAddress to avoid load issues on Windows 2000
	pGNSI = (PGNSI) TTR_GetProcAddress(	GetModuleHandle( _T("kernel32.dll")),_T("GetNativeSystemInfo") );
	if(NULL != pGNSI)
		pGNSI(&si);


	switch (osvi.dwPlatformId){
		// Test for the Windows NT product family.
		case VER_PLATFORM_WIN32_NT:
			// Test for the specific product.
			if( osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 0 ){

				if( osvi.dwMinorVersion == 0 ){

					if( osvi.wProductType == VER_NT_WORKSTATION )
						nResult=TTR_CUR_OS_VERSION_WINVISTA;
					else
						nResult=TTR_CUR_OS_VERSION_WIN2008;
				}
				else if( osvi.dwMinorVersion == 1 ){

					if( osvi.wProductType == VER_NT_WORKSTATION )
						nResult=TTR_CUR_OS_VERSION_WIN7;
					else
						nResult=TTR_CUR_OS_VERSION_WIN2008R2;
				}
			}
			else if( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2 ){

				if( GetSystemMetrics(SM_SERVERR2) )
					nResult=TTR_CUR_OS_VERSION_WIN2003; //Microsoft Windows Server 2003 R2
				else if( osvi.wProductType == VER_NT_WORKSTATION && si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64)
					nResult=TTR_CUR_OS_VERSION_WINXP;//Microsoft Windows XP Professional x64 Edition
				else
					nResult=TTR_CUR_OS_VERSION_WIN2003;//Microsoft Windows Server 2003
			}
			else if( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1 )
				nResult=TTR_CUR_OS_VERSION_WINXP;//Microsoft Windows XP
			else if( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0 )
				nResult=TTR_CUR_OS_VERSION_WIN2000;//Microsoft Windows 2000
			else if( osvi.dwMajorVersion <= 4 )
				nResult=TTR_CUR_OS_VERSION_WINNT;//Microsoft Windows NT

			// Test for specific product on Windows NT 4.0 SP6 and later.
			if( bOsVersionInfoEx && pGNSI ){
				// Test for the workstation type.
				if( osvi.wProductType == VER_NT_WORKSTATION && si.wProcessorArchitecture!=PROCESSOR_ARCHITECTURE_AMD64){
					/*
					if( osvi.dwMajorVersion == 4 ) TraceOnly( 0,"Workstation 4.0 " );
					else if( osvi.wSuiteMask & VER_SUITE_PERSONAL )	TraceOnly( 0,"Home Edition " );
					else TraceOnly( 0,"Professional " );
					*/
				}// Test for the server type.
				else if( osvi.wProductType == VER_NT_SERVER || osvi.wProductType == VER_NT_DOMAIN_CONTROLLER ){
					if(osvi.dwMajorVersion==5 && osvi.dwMinorVersion==2){
						if ( si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_IA64 ){
							/*
							if( osvi.wSuiteMask & VER_SUITE_DATACENTER )	TraceOnly( 0,"Datacenter Edition for Itanium-based Systems" );
							else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )	TraceOnly( 0,"Enterprise Edition for Itanium-based Systems" );
							*/
						}
						else if ( si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64 ){
							/*
							if( osvi.wSuiteMask & VER_SUITE_DATACENTER )	TraceOnly( 0,"Datacenter x64 Edition " );
							else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )	TraceOnly( 0,"Enterprise x64 Edition " );
							else TraceOnly( 0,"Standard x64 Edition " );
							*/
						}
						else{
							/*
							if( osvi.wSuiteMask & VER_SUITE_DATACENTER )	TraceOnly( 0,"Datacenter Edition " );
							else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )	TraceOnly( 0,"Enterprise Edition " );
							else if ( osvi.wSuiteMask == VER_SUITE_BLADE )	TraceOnly( 0,"Web Edition " );
							else	TraceOnly( 0,"Standard Edition " );
							*/
						}
					}
					else if(osvi.dwMajorVersion==5 && osvi.dwMinorVersion==0){
						/*
						if( osvi.wSuiteMask & VER_SUITE_DATACENTER )	TraceOnly( 0,"Datacenter Server " );
						else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )	TraceOnly( 0,"Advanced Server " );
						else	TraceOnly( 0,"Server " );
						*/
					}
					else{  // Windows NT 4.0 
						/*
						if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )	TraceOnly( 0,"Server 4.0, Enterprise Edition " );
						else	TraceOnly( 0,"Server 4.0 " );
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
				if( lstrcmpi( "WINNT", szProductType) == 0 )	TraceOnly( 0,"Workstation " );
				if( lstrcmpi( "LANMANNT", szProductType) == 0 )	TraceOnly( 0,"Server " );
				if( lstrcmpi( "SERVERNT", szProductType) == 0 )	TraceOnly( 0,"Advanced Server " );
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
				if( lRet == ERROR_SUCCESS )	TraceOnly( 0,"Service Pack 6a (Build %d)\n",osvi.dwBuildNumber & 0xFFFF );         
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
				nResult=TTR_CUR_OS_VERSION_WIN95;
				/*
				if (osvi.szCSDVersion[1]=='C' || osvi.szCSDVersion[1]=='B')
					TraceOnly( 0,"OSR2 " );
				*/
			} 

			if(osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 10){
				//TraceOnly( 0,"Microsoft Windows 98 ");
				nResult=TTR_CUR_OS_VERSION_WIN98;
				/*
				if( osvi.szCSDVersion[1]=='A' || osvi.szCSDVersion[1]=='B')
					TraceOnly( 0,"SE " );
				*/
			}

			if(osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 90){
				//TraceOnly( 0,"Microsoft Windows Millennium Edition\n");
				nResult=TTR_CUR_OS_VERSION_WINME;
			} 
			break;
		case VER_PLATFORM_WIN32s:
			//TraceOnly( 0,"Microsoft Win32s\n");
			break;
	}//end switch
	return nResult; 
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// return Locale Identifier Constants and Strings
// see : ms-help://MS.W7SDK.1033/MS.W7SDKCOM.1033/intl/intl/language_identifier_constants_and_strings.htm
INT TTR_GetOSDefaultLanguage()
{
	LONG lDefLanInfo;
	INT nLanguageCode=0;


	if( !SystemParametersInfo( SPI_GETDEFAULTINPUTLANG,0,&lDefLanInfo,0 ) ){
		return 0;	//invalied language code
	}
	//
	lDefLanInfo=lDefLanInfo&0x0000FFFF;//make upper word

	if( lDefLanInfo==0x0812 ){//Korean OS
		//0x0812 is only Windows 95, Windows NT 4.0
		nLanguageCode=LANG_KOREAN;//( 0x0412 == LANG_KOREAN )
	}
	else
		nLanguageCode = lDefLanInfo;


	return nLanguageCode;
}


//time delay  unit : 100nsec
//constant sytle : 1 sec = 10000000LL
VOID TTR_Delay( LONGLONG llTime )
{
	HANDLE hTimer = NULL;
	LARGE_INTEGER liDueTime;

	liDueTime.QuadPart = -llTime;

	// Create an unnamed waitable timer.
	hTimer = CreateWaitableTimer(NULL, TRUE, NULL);
    if (NULL == hTimer)
		return;

	// Set a timer to wait for 10 seconds.
	if (!SetWaitableTimer(hTimer, &liDueTime, 0, NULL, NULL, 0))
		return;

	// Wait for the timer.
	WaitForSingleObject(hTimer, INFINITE);

	CloseHandle( hTimer );
}

//make multi zero-string with given zero-strings
//return the numebr of data(BYTE) in multistring(including NULL )
INT TTR_MakeMultiString( INT nCnt,LPTSTR szMultiStr, ... )
{
	LPTSTR pStr = NULL;
	LPTSTR szMultiOut = szMultiStr;
	INT nNumStr = 0;
	INT i;

	va_list argptr;
	va_start(argptr, nCnt);

	//pass second argument
	va_arg( argptr, LPTSTR );

	for( i=0; i<nCnt; i++ ){
		pStr = (LPTSTR)va_arg( argptr, LPCTSTR );

		if( pStr ){

			_tcscpy( szMultiOut, pStr );
			szMultiOut[_tcslen(pStr)] = NULL;

			szMultiOut = szMultiOut + _tcslen(pStr) +1;

			nNumStr = nNumStr + (_tcslen(pStr) +1) * sizeof(TCHAR);
		}
	}//end for

	if( nNumStr >0 ){
		szMultiOut[0] = NULL;	//make multi Zero string
		nNumStr = nNumStr + sizeof(TCHAR);
	}

	return nNumStr;

}