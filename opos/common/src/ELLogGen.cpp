///////////////////////////////////////////
//ELLogGen.cpp
//Log generate body
//////////////////////////////////////////
#include "stdafx.h"
#include <stdio.h>
#include <time.h>
#include "ELLogGen.h"
#include "EL_Support.h"		//20090911

//Search available log file
BOOL SetCurLogFileName( PLOGGENCTL pLogCtl );

void TraceTimeTemp( const TCHAR *filename,const TCHAR *pszFormat, ...)
{
	struct tm *dt;
	va_list args;
	time_t t;

	FILE *pstream;
	//
	if( filename==NULL )
		return;
	//
	pstream = _tfopen( filename, _T("a+") );

	if( pstream!=NULL ){
		va_start(args, pszFormat);
		t = time(NULL);
		dt = localtime(&t);
		//
		_ftprintf( pstream, _T("[%02d-%02d %02d:%02d:%02d] "), dt->tm_mon+1,
						dt->tm_mday, dt->tm_hour, dt->tm_min, dt->tm_sec);
		_vftprintf( pstream,pszFormat,args );
		_flushall();
		fclose( pstream );
	}
}

void TraceTemp( const TCHAR *filename,const TCHAR *pszFormat, ...)
{
	va_list args;

	FILE *pstream;
	//
	if( filename==NULL )
		return;
	//
	pstream = _tfopen( filename, _T("a+") );

	if( pstream!=NULL ){
		va_start(args, pszFormat);
		_vftprintf( pstream,pszFormat,args );
		_flushall();
		fclose( pstream );
	}
}

void Trace( PLOGGENCTL pLogCtl,long lLogLev,const TCHAR *pszFormat, ...)
{
	static BOOL bOpened=FALSE;
	static FILE *pstream=NULL;
	static BOOL bFirst=TRUE;
	static DWORD dwStTick=0;
	
	struct tm *dt;
	va_list args;
	time_t t;
	TCHAR sDir[_MAX_DIR];
	TCHAR sDrive[_MAX_DRIVE];
	TCHAR sFileName[_MAX_FNAME];
	TCHAR sFullFilePath[_MAX_PATH];
	
	if( pLogCtl==NULL )
		return;
	if( !pLogCtl->bEnable )
		return;

	//check log level 
	if( lLogLev>=pLogCtl->lBaseLevel )
		return;
	/////////////////
	if( !bOpened ){
		//generate full log path name
		_tsplitpath( pLogCtl->sLogFolder,sDrive,sDir,NULL,NULL );
		_stprintf( sFileName,_T("%s%d"),pLogCtl->sFileBaseName,pLogCtl->lCurIndex );
		_tmakepath( sFullFilePath,sDrive,sDir,sFileName,pLogCtl->sFileBaseExt );
		
		pstream = _tfopen( sFullFilePath, _T("a+") );
		if( pstream==NULL ){
			pLogCtl->pLogFileStream=NULL;
			return;
		}
		else{
			pLogCtl->pLogFileStream=pstream;
			bOpened=TRUE;
		}
	}
	//////
	va_start(args, pszFormat);
	
	if( pLogCtl->bEnableTimeStemp ){
		t = time(NULL);
		dt = localtime(&t);
		_ftprintf( pstream, _T("[%02d-%02d %02d:%02d:%02d] "), dt->tm_mon+1,
						dt->tm_mday, dt->tm_hour, dt->tm_min, dt->tm_sec);
	}
	
	if( pLogCtl->bEnableTimeTick ){
		if( bFirst ){
			dwStTick=GetTickCount();
			_ftprintf( pstream, _T("[%09u] "),0);
			bFirst=FALSE;
		}
		else
			_ftprintf( pstream, _T("[%09u] "),GetTickCount()-dwStTick);
	}
	
	_vftprintf( pstream,pszFormat,args );
	fflush( pstream );
	/////
	if( pLogCtl->bOnlyOpenGen ){
		fclose( pstream );
		pLogCtl->pLogFileStream=NULL;
		bOpened=FALSE;
	}
	
}

//Trace without time mark( date,time,timetick and so on....... )
void TraceNoTimeMark( PLOGGENCTL pLogCtl,long lLogLev,const TCHAR *pszFormat, ...)
{
	static BOOL bOpened=FALSE;
	static FILE *pstream=NULL;
	static BOOL bFirst=TRUE;
	static DWORD dwStTick=0;
	
	va_list args;
	TCHAR sDir[_MAX_DIR];
	TCHAR sDrive[_MAX_DRIVE];
	TCHAR sFileName[_MAX_FNAME];
	TCHAR sFullFilePath[_MAX_PATH];
	
	if( pLogCtl==NULL )
		return;
	if( !pLogCtl->bEnable )
		return;

	//check log level 
	if( lLogLev>=pLogCtl->lBaseLevel )
		return;
	/////////////////
	if( !bOpened ){
		//generate full log path name
		_tsplitpath( pLogCtl->sLogFolder,sDrive,sDir,NULL,NULL );
		_stprintf( sFileName,_T("%s%d"),pLogCtl->sFileBaseName,pLogCtl->lCurIndex );
		_tmakepath( sFullFilePath,sDrive,sDir,sFileName,pLogCtl->sFileBaseExt );
		
		pstream = _tfopen( sFullFilePath, _T("a+") );
		if( pstream==NULL ){
			pLogCtl->pLogFileStream=NULL;
			return;
		}
		else{
			pLogCtl->pLogFileStream=pstream;
			bOpened=TRUE;
		}
	}
	//////
	va_start(args, pszFormat);
	
	_vftprintf( pstream,pszFormat,args );
	fflush( pstream );
	/////
	if( pLogCtl->bOnlyOpenGen ){
		fclose( pstream );
		pLogCtl->pLogFileStream=NULL;
		bOpened=FALSE;
	}
	
}

//Create Log control(allocate memory)
PLOGGENCTL CreateLogControl()
{
	PLOGGENCTL pLogCtl;
	
	pLogCtl=(PLOGGENCTL)malloc( sizeof(LOGGENCTL) );
	if( pLogCtl==NULL )
		return pLogCtl;
	//
	memset( pLogCtl,0x00,sizeof(LOGGENCTL) );
	
	//set default
	pLogCtl->bEnable=FALSE;
	pLogCtl->lBaseLevel=0;			//log generate base level
	pLogCtl->bEnableTimeStemp=TRUE;	//generate time stemp
	pLogCtl->bEnableTimeTick=FALSE;	//generate time tick
	pLogCtl->bOnlyOpenGen=FALSE;	//open at generating log and close.
	
	SetLogDirectory( pLogCtl,_T(".\\") );
	_tcscpy( pLogCtl->sFileBaseName,_T("ELlog") );
	_tcscpy( pLogCtl->sFileBaseExt,_T(".txt") );
	pLogCtl->pLogFileStream=NULL;
	pLogCtl->lCurIndex=0;				//log file current index number
	pLogCtl->lMaxLog=DEF_EL_LOG_NUMBER;	//the number of maximum log files.
	pLogCtl->hModule=NULL;	//20090911
	
	//
	return pLogCtl;
}

//Create Log control(allocate memory) //20090911
PLOGGENCTL CreateLogControlWithModule( HMODULE hModule )
{
	PLOGGENCTL pLogCtl;
	
	pLogCtl=(PLOGGENCTL)malloc( sizeof(LOGGENCTL) );
	if( pLogCtl==NULL )
		return pLogCtl;
	//
	memset( pLogCtl,0x00,sizeof(LOGGENCTL) );
	
	//set default
	pLogCtl->bEnable=FALSE;
	pLogCtl->lBaseLevel=0;			//log generate base level
	pLogCtl->bEnableTimeStemp=TRUE;	//generate time stemp
	pLogCtl->bEnableTimeTick=FALSE;	//generate time tick
	pLogCtl->bOnlyOpenGen=FALSE;	//open at generating log and close.
	
	SetLogDirectory( pLogCtl,_T(".\\") );
	_tcscpy(	pLogCtl->sFileBaseName,_T("ELlog") );
	_tcscpy(	pLogCtl->sFileBaseExt,_T(".txt") );
	pLogCtl->pLogFileStream=NULL;
	pLogCtl->lCurIndex=0;				//log file current index number
	pLogCtl->lMaxLog=DEF_EL_LOG_NUMBER;	//the number of maximum log files.
	pLogCtl->hModule=hModule;	
	
	//
	return pLogCtl;
}

//Delete Log control(free memory)
void DeleteLogControl(PLOGGENCTL pLogCtl)
{
	if( pLogCtl==NULL )
		return;
	//
	if( pLogCtl->pLogFileStream!=NULL )
		fclose( pLogCtl->pLogFileStream );
	//
	free( pLogCtl );
}

//Initial Log control structure by ini file
void IniLogControlByIniFile( PLOGGENCTL pLogCtl,const TCHAR *Inifilename )
{
	HANDLE hfile;
	DWORD dwResult;
	TCHAR strEnable[_MAX_PATH];
#ifdef UNICODE 
	CHAR strTmp[2*(_MAX_PATH+1)];
#endif
	TCHAR strExten[_MAX_PATH];
	TCHAR *psData;
	int nEnablelog;
	int nLevel;
	int i,nSize;
	
	if( Inifilename==NULL )
		return;
	//exist ini-file
	hfile=CreateFile(Inifilename,GENERIC_READ,FILE_SHARE_READ, NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL, NULL);
	if( hfile==INVALID_HANDLE_VALUE ){
		pLogCtl->bEnable=FALSE;//disable log
		return;
	}
	CloseHandle(hfile);
	//
	dwResult=GetPrivateProfileString(_T("LogSetting"),_T("logenable"),_T("100"),strEnable,_MAX_PATH,Inifilename);
	if( dwResult==0 ){
		pLogCtl->bEnable=FALSE;//disable log
		return;	//if GetPrivateProfileString is failed
	}
	//
#ifdef UNICODE
	wcstombs(strTmp, (const wchar_t *)strEnable, sizeof(strTmp)); 
	nEnablelog=atoi( strTmp );
#else
	nEnablelog=atoi( strEnable );
#endif

	//
	if( nEnablelog==1 ){
		//setting log level
		memset( strEnable,0,sizeof(TCHAR)*_MAX_PATH );
		dwResult=GetPrivateProfileString(_T("LogSetting"),_T("loglevel"),_T("100"),strEnable,_MAX_PATH,Inifilename);
		if( dwResult==0 )
			pLogCtl->lBaseLevel=0;//default level
		else{
			#ifdef UNICODE
				wcstombs(strTmp, (const wchar_t *)strEnable, sizeof(strTmp)); 
				nLevel=atoi( strTmp );
			#else
				nLevel=atoi( strEnable );
			#endif

			if( nLevel<0 || nLevel>MAX_EL_LOG_LEVEL )
				nLevel=0;//default level
		}
		pLogCtl->lBaseLevel=nLevel;
		pLogCtl->bEnable=TRUE;//enable log
		
		//setting time stemp
		memset( strEnable,0,sizeof(TCHAR)*_MAX_PATH );
		dwResult=GetPrivateProfileString(_T("LogSetting"),_T("logtimestemp"),_T("100"),strEnable,_MAX_PATH,Inifilename);
		if( dwResult==0 )
			pLogCtl->bEnableTimeStemp=TRUE;//default
		else{
			#ifdef UNICODE
				wcstombs(strTmp, (const wchar_t *)strEnable, sizeof(strTmp)); 
				nLevel=atoi( strTmp );
			#else
				nLevel=atoi( strEnable );
			#endif

			if( nLevel!=0 && nLevel!=1 )
				nLevel=0;//default level
			//
			if( nLevel==1 )
				pLogCtl->bEnableTimeStemp=TRUE;
			else
				pLogCtl->bEnableTimeStemp=FALSE;
		}
		
		//setting time tick
		memset( strEnable,0,sizeof(TCHAR)*_MAX_PATH );
		dwResult=GetPrivateProfileString(_T("LogSetting"),_T("logtimetick"),_T("100"),strEnable,_MAX_PATH,Inifilename);
		if( dwResult==0 )
			pLogCtl->bEnableTimeTick=FALSE;//default
		else{
			#ifdef UNICODE
				wcstombs(strTmp, (const wchar_t *)strEnable, sizeof(strTmp)); 
				nLevel=atoi( strTmp );
			#else
				nLevel=atoi( strEnable );
			#endif

			if( nLevel!=0 && nLevel!=1 )
				nLevel=0;//default level
			//
			if( nLevel==1 )
				pLogCtl->bEnableTimeTick=TRUE;
			else
				pLogCtl->bEnableTimeTick=FALSE;
		}
		
		//setting open at generating log and close.
		memset( strEnable,0,sizeof(TCHAR)*_MAX_PATH );
		dwResult=GetPrivateProfileString(_T("LogSetting"),_T("logonlyopen"),_T("100"),strEnable,_MAX_PATH,Inifilename);
		if( dwResult==0 )
			pLogCtl->bOnlyOpenGen=FALSE;//default
		else{
			#ifdef UNICODE
				wcstombs(strTmp, (const wchar_t *)strEnable, sizeof(strTmp)); 
				nLevel=atoi( strTmp );
			#else
				nLevel=atoi( strEnable );
			#endif

			if( nLevel!=0 && nLevel!=1 )
				nLevel=0;//default level
			//
			if( nLevel==1 )
				pLogCtl->bOnlyOpenGen=TRUE;
			else
				pLogCtl->bOnlyOpenGen=FALSE;
		}
		
		//setting log directory
		memset( strEnable,0,sizeof(TCHAR)*_MAX_PATH );
		dwResult=GetPrivateProfileString(_T("LogSetting"),_T("logdir"),_T(".\\"),strEnable,_MAX_PATH,Inifilename);
		if( dwResult==0 )
			SetLogDirectory( pLogCtl,_T(".\\") );//default
		else{//check trailing backslash.
			for( i=_tcslen( strEnable )-1; i>-1; i-- ){
				if( strEnable[i]==_T('\\') ){
					strEnable[i+1]=NULL;
					break;//exit for
				}
				else if( strEnable[i]==_T(' ') ){
					strEnable[i]=NULL;
				}
				else{
					strEnable[i+1]=_T('\\');
					strEnable[i+2]=NULL;
					break;
				}
			}//end for
			SetLogDirectory( pLogCtl,strEnable );
		}
		
		//setting log base name
		memset( strEnable,0,sizeof(TCHAR)*_MAX_PATH );
		dwResult=GetPrivateProfileString(_T("LogSetting"),_T("logname"),_T("ELlog"),strEnable,_MAX_PATH,Inifilename);
		if( dwResult==0 )
			_tcscpy( pLogCtl->sFileBaseName,_T("ELlog") );//default
		else{//remove trailing space
			for( i=_tcslen( strEnable )-1; i>-1; i-- ){
				if( strEnable[i]==_T(' ') )
					strEnable[i]=NULL;
				else{
					strEnable[i+1]=NULL;
					break;
				}
			}//end for
			_tcscpy(	pLogCtl->sFileBaseName,strEnable );
		}
			
		//setting log file extension
		memset( strEnable,0,sizeof(TCHAR)*_MAX_PATH );
		dwResult=GetPrivateProfileString(_T("LogSetting"),_T("logext"),_T(".txt"),strEnable,_MAX_PATH,Inifilename);
		if( dwResult==0 )
			_tcscpy( pLogCtl->sFileBaseExt,_T(".txt") );//default
		else{
			psData=strEnable;
			nSize=_tcslen( strEnable );
			for( i=0; i<nSize; i++ ){
				if( (*psData)==_T(' ') )
					psData+=sizeof(TCHAR);
				else if( (*psData)==_T('.') )
					break;//exit for
				else{
					_stprintf( strExten,_T(".%s"),psData );
					psData=strExten;
					break;//exit for
				}
			}//end for

			for( i=_tcslen( psData )-1; i>-1; i-- ){
				if( psData[i]==_T(' ') )
					psData[i]=NULL;
				else{
					psData[i+1]=NULL;
					break;
				}
			}//end for

			_tcscpy( pLogCtl->sFileBaseExt,psData );
		}
			
		//setting the number of maximum log files.
		memset( strEnable,0,sizeof(TCHAR)*_MAX_PATH );
		dwResult=GetPrivateProfileString(_T("LogSetting"),_T("logmax"),_T("7"),strEnable,_MAX_PATH,Inifilename);
		if( dwResult==0 )
			pLogCtl->lMaxLog=DEF_EL_LOG_NUMBER;//default
		else{
			#ifdef UNICODE
				wcstombs(strTmp, (const wchar_t *)strEnable, sizeof(strTmp)); 
				nLevel=atoi( strTmp );
			#else
				nLevel=atoi( strEnable );
			#endif

			if( nLevel<=0 )
				nLevel=DEF_EL_LOG_NUMBER;//default level
			//
			pLogCtl->lMaxLog=nLevel;
		}
		
		//
		if( !SetCurLogFileName(pLogCtl) )
			pLogCtl->bEnable=FALSE;//disable log
		//
	}
	else
		pLogCtl->bEnable=FALSE;//disable log
}

//Search available log file
BOOL SetCurLogFileName( PLOGGENCTL pLogCtl )
{
	TCHAR sFile[_MAX_FNAME];
//	TCHAR sLogFolder[_MAX_PATH];
	TCHAR sLogFile[_MAX_FNAME];
	HANDLE *hFind;
	WIN32_FIND_DATA *FindFileData;
	int i,nold;
	int nSize;
	LONG lResult;

	if( pLogCtl==NULL )
		return FALSE;
	//
	if( pLogCtl->lMaxLog<1 )
		return FALSE;
	
	hFind=(HANDLE*)malloc(sizeof(HANDLE)*(pLogCtl->lMaxLog+1));
	if( hFind==NULL )
		return FALSE;
	//
	FindFileData=(WIN32_FIND_DATA*)malloc(sizeof(WIN32_FIND_DATA)*pLogCtl->lMaxLog);
	if( FindFileData==NULL ){
		free(hFind);
		return FALSE;
	}
	
	//search log directory
	_tcscpy( sFile,pLogCtl->sLogFolder );

	nSize=_tcslen(sFile);
	//remove trailing backslash.
	for( i=nSize-1; i>=0; i-- ){
		if( sFile[i]==_T('\\') ){
			sFile[i]=NULL;
			break;//exit for
		}
	}//end for

	hFind[0]=FindFirstFile( (LPCTSTR)sFile,&(FindFileData[0]) );
	if( hFind[0] == INVALID_HANDLE_VALUE) {
		//Create directory
		if( !CreateDirectory(sFile,NULL) ){
			free(hFind);	free(FindFileData);
			return FALSE;
		}
	}
	else{
		//directory search OK
		FindClose(hFind[0]);
	}

	//search log files - pinpadlog0.txt-pinpadlog7.txt
	for( i=0; i<pLogCtl->lMaxLog; i++ ){
		_stprintf( sLogFile,_T("%s%d%s"),pLogCtl->sFileBaseName,i,pLogCtl->sFileBaseExt );
		_tcscpy( sFile,pLogCtl->sLogFolder );
		_tcscat( sFile,sLogFile );
		
		hFind[i]=FindFirstFile( (LPCTSTR)sFile,&(FindFileData[i]) );
		if( hFind[i] == INVALID_HANDLE_VALUE) {
			pLogCtl->lCurIndex=i;
			free(hFind);	free(FindFileData);
			return TRUE;
		}
		FindClose(hFind[i]);
	}//end for

	//exist pinpadlog0.txt-pinpadlog7.txt
	/*
	LONG CompareFileTime(
		CONST FILETIME *lpFileTime1,  // first file time
		CONST FILETIME *lpFileTime2   // second file time
	);
	return :
	-1 :lpFileTime1<lpFileTime2
	 0 :lpFileTime1=lpFileTime2
	 1 :lpFileTime1>lpFileTime2
	*/
	nold=0;
	for( i=1; i<pLogCtl->lMaxLog; i++ ){
		lResult=CompareFileTime( 
			&(FindFileData[nold].ftLastWriteTime),
			&(FindFileData[i].ftLastWriteTime)
		);

		if( lResult<0 ){
			//FindFileData[nold].ftLastWriteTime<FindFileData[i].ftLastWriteTime

		}
		else if( lResult==0 ){
			//FindFileData[nold].ftLastWriteTime==FindFileData[i].ftLastWriteTime
		
		}
		else{
			//FindFileData[nold].ftLastWriteTime>FindFileData[i].ftLastWriteTime
			nold=i;
		}
	}//end for

	pLogCtl->lCurIndex=nold;

	//remove file
	_stprintf( sLogFile,_T("%s%d%s"),pLogCtl->sFileBaseName,nold,pLogCtl->sFileBaseExt );
	_tcscpy( sFile,pLogCtl->sLogFolder );
	_tcscat( sFile,sLogFile );
	_tremove( sFile );

	//_stprintf( sLogFile,"\\pinpadlog%d.txt",nold );
	
	free(hFind);	free(FindFileData);
	return TRUE;
}

//setting sFileBaseName and sFileBaseExt member of LOGGENCTL structure
BOOL SetLogBaseName( PLOGGENCTL pLogCtl,const TCHAR *basename )
{
	if( pLogCtl==NULL || basename==NULL )
		return FALSE;
	//
	_tsplitpath( basename,NULL,NULL,pLogCtl->sFileBaseName,pLogCtl->sFileBaseExt );
	return TRUE;
}

//update 20090911
//setting sLogFolder member of LOGGENCTL structure
//the last character must be '\'.
BOOL SetLogDirectory( PLOGGENCTL pLogCtl,const TCHAR *sLogDircetory )
{
	if( pLogCtl==NULL || sLogDircetory==NULL )
		return FALSE;
	
	//this code needs more checking .......
	if( pLogCtl->hModule == NULL ){
		_tfullpath( pLogCtl->sLogFolder, sLogDircetory, _MAX_PATH );//20090911
	}
	else{
		
		if( EL_IsThisRelativePath( sLogDircetory ) ){
			//given path is relative path
			//EL_GetCurFullPath( TCHAR *psFullPath,pLogCtl->hModule );
			_tfullpath( pLogCtl->sLogFolder, sLogDircetory, _MAX_PATH );//20090911
		}
		else//given path is absolute path
			_tcscpy( pLogCtl->sLogFolder,sLogDircetory );

	}
	return TRUE;
}


//setting bEnable member of LOGGENCTL structure
BOOL EnableELLog( PLOGGENCTL pLogCtl,BOOL bEnable )
{
	if( pLogCtl==NULL )
		return FALSE;
	pLogCtl->bEnable=bEnable;
	return TRUE;
}

//setting bEnableTimeStemp member of LOGGENCTL structure
BOOL EnableELLogTimeStemp( PLOGGENCTL pLogCtl,BOOL bEnable )
{
	if( pLogCtl==NULL )
		return FALSE;
	pLogCtl->bEnableTimeStemp=bEnable;
	return TRUE;
}

//setting bEnableTimeTick member of LOGGENCTL structure
BOOL EnableELLogTimeTick( PLOGGENCTL pLogCtl,BOOL bEnable )
{
	if( pLogCtl==NULL )
		return FALSE;
	pLogCtl->bEnableTimeTick=bEnable;
	return TRUE;
}

//setting bOnlyOpenGen member of LOGGENCTL structure
BOOL EnableELLogWriteOnlyOpen( PLOGGENCTL pLogCtl,BOOL bEnable )
{
	if( pLogCtl==NULL )
		return FALSE;
	pLogCtl->bOnlyOpenGen=bEnable;
	return TRUE;
}

//setting lBaseLevel member of LOGGENCTL structure
BOOL SetELLogBaseLevel( PLOGGENCTL pLogCtl,long lBaseLevel )
{
	if( pLogCtl==NULL )
		return FALSE;
	
	if( lBaseLevel>MAX_EL_LOG_LEVEL || lBaseLevel<0 )
		return FALSE;
	else
		pLogCtl->lBaseLevel=lBaseLevel;
	return TRUE;
}

//setting lMaxLog member of LOGGENCTL structure
BOOL SetELLogMax( PLOGGENCTL pLogCtl,long lMax )
{
	if( pLogCtl==NULL )
		return FALSE;
	
	if( lMax>MAX_EL_LOG_NUMBER || lMax<1 )
		return FALSE;
	else
		pLogCtl->lMaxLog=lMax;
	return TRUE;
}
