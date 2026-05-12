///////////////////////////////////////////
//ELLogGen.cpp
//Log generate body
//////////////////////////////////////////
#include "stdafx.h"
#include <stdio.h>
#include <time.h>
#include "TTR_LogGen.h"
#include "TTR_etc.h"
#include <locale.h>
#include <time.h>

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>


const INT CTTR_LogSystem::DefValue::DefV_enable=1;
const INT CTTR_LogSystem::DefValue::DefV_disablelevel=3;
const INT CTTR_LogSystem::DefValue::DefV_printtime=1;
const INT CTTR_LogSystem::DefValue::DefV_printtimetick=1;
const INT CTTR_LogSystem::DefValue::DefV_maxnumber=7;

const TCHAR CTTR_LogSystem::DefValue::DefV_folder[] = _T(".\\");
const TCHAR CTTR_LogSystem::DefValue::DefV_format[] = _T("ttr_log%2d");
const TCHAR CTTR_LogSystem::DefValue::DefV_extention[] = _T(".txt");
		//

const TCHAR CTTR_LogSystem::DefValue::SectionName[] = _T("set");
const TCHAR CTTR_LogSystem::DefValue::ssKeyName[TTR_LOG_KEY_NUM][TTR_DEF_NAME_BUF_SIZE] ={

			_T("enable"),
			_T("disablelevel"),
			_T("printtime"),
			_T("printtimetick"),
			_T("maxnumber"),
			_T("folder"),
			_T("format"),
			_T("extention")
		};

CRITICAL_SECTION CTTR_LogSystem::m_CSSafe;		//critical section in a processor

using namespace std;


//default constrcture
CTTR_LogSystem::CTTR_LogSystem()
{
	//////////////////
	//loading default
	_tsetlocale( LC_ALL, _T( "" ) );	//set local

	m_bStart = FALSE;

	InitializeCriticalSectionAndSpinCount( &m_CSSafe,TTR_DEF_SPIN_COUNT );

	m_nEnable = CTTR_LogSystem::DefValue::DefV_enable;	//32 bits value - enable or disable
	m_nBaseLevel = CTTR_LogSystem::DefValue::DefV_disablelevel;	//log generate base level
	m_nMaxLog = CTTR_LogSystem::DefValue::DefV_maxnumber;		//the number of maximum log files.

	m_nEnableTimeStemp = CTTR_LogSystem::DefValue::DefV_printtime;	//generate time stemp
	m_nEnableTimeTick = CTTR_LogSystem::DefValue::DefV_printtimetick;	//generate time tick

	_tcscpy( m_sLogFolderFormat,CTTR_LogSystem::DefValue::DefV_folder );		//log generating folder.
	_tcscpy( m_sFileNameFormat,CTTR_LogSystem::DefValue::DefV_format );	//log file format name(except extension & index)
	_tcscpy( m_sFileNameExt,CTTR_LogSystem::DefValue::DefV_extention );	//log file extension(include dot)

	m_nCurIndex = 0;				//log file current index number
	m_hModule = NULL;				//App module

	m_sCurLogFullPath[0] = NULL;
	
	m_pLogFileStream = NULL;
}

CTTR_LogSystem::CTTR_LogSystem( HMODULE hModule )
{
	CTTR_LogSystem();

	m_hModule = hModule;
}

//Ini file constrcture
CTTR_LogSystem::CTTR_LogSystem( HMODULE hModule,LPCTSTR sInifilename )
{
	CTTR_LogSystem();

	Initial( hModule, sInifilename );
}	

//Ini registry constrcture
//HKCU\Software\company_name\product_name\TTR_LogSystem' node

//default destructure
CTTR_LogSystem::~CTTR_LogSystem()
{
	if( m_pLogFileStream ){

		fflush( m_pLogFileStream );
		fclose( m_pLogFileStream );

	}

	DeleteCriticalSection(&m_CSSafe);//remove critical section

}

//Ini system by ini file
BOOL CTTR_LogSystem::Initial( HMODULE hModule,LPCTSTR sInifilename )
{
	BOOL bResult = TRUE;
	INT nResult = -1;
	LPINT pnVal[TTR_LOG_INT_KEY_NUM];
	INT i;
	LPTSTR psVal[TTR_LOG_STR_KEY_NUM];
	TCHAR sAbsInifilename[_MAX_PATH];

	if( hModule==NULL )
		return FALSE;

	//get abs path from relative oath.......
	TTR_GetAbsPathFromRelPathInModule( sAbsInifilename,_MAX_PATH,hModule,sInifilename );

	EnterCriticalSection(&m_CSSafe);
	//

	if( !TTR_IsExistFile(sAbsInifilename) ){
		bResult = FALSE;
	}
	else{
		m_hModule = hModule;
		//
		i=0;
		pnVal[i++] = &m_nEnable;			//32 bits value - enable or disable
		pnVal[i++] = &m_nBaseLevel;			//log generate base level
		pnVal[i++] = &m_nEnableTimeStemp;	//generate time stemp
		pnVal[i++] = &m_nEnableTimeTick;	//generate time tick
		pnVal[i++] = &m_nMaxLog;			//the number of maximum log files.

		i=0;
		psVal[i++] = m_sLogFolderFormat;		//log generating folder.
		psVal[i++] = m_sFileNameFormat;	//log file format name(except extension & index)
		psVal[i++] = m_sFileNameExt;	//log file extension(include dot)


		for( i=0; i<TTR_LOG_INT_KEY_NUM; i++ ){

			nResult = TTR_GetPosZeroIntegerFromInfKey( 
				CTTR_LogSystem::DefValue::SectionName,
				CTTR_LogSystem::DefValue::ssKeyName[i],
				sAbsInifilename
				);

			if( nResult>=0 )
				*pnVal[i] = nResult;
			else{
				LoadDefaultValue( i );//error ....... load default vaule
			}

		}//end for

		for( i=0; i<TTR_LOG_STR_KEY_NUM; i++ ){

			bResult = TTR_GetStringFromInfKey( 
				psVal[i],
				CTTR_LogSystem::DefValue::SectionName,
				CTTR_LogSystem::DefValue::ssKeyName[i+TTR_LOG_INT_KEY_NUM],
				sAbsInifilename
				);

			if( !bResult ){
				LoadDefaultValue( i+TTR_LOG_INT_KEY_NUM );//error ....... load default vaule
			}
		}//end for

	}

	//
	LeaveCriticalSection(&m_CSSafe);

	return bResult;

}

	
//starting  or stop log system
BOOL CTTR_LogSystem::Start( BOOL bStart /*=TRUE*/)
{
	BOOL bResult = TRUE;

	EnterCriticalSection(&m_CSSafe);
	///////////////////////////////////////////////

	if( bStart ){
		if( SetCurLogFileName() )
			m_bStart = bStart;
		else
			bResult = FALSE;
	}
	else
		m_bStart = bStart;

	
	///////////////////////////////////////////////
	LeaveCriticalSection(&m_CSSafe);

	return bResult;
}

	
//setting sFileBaseName and sFileBaseExt member of LOGGENCTL structure
BOOL CTTR_LogSystem::SetFileNameFormat( LPCTSTR sFormatname )
{
	BOOL bResult = TRUE;


	EnterCriticalSection(&m_CSSafe);
	///////////////////////////////////////////////

	if( sFormatname==NULL ){
		bResult = FALSE;
	}
	else{
		_tcscpy( m_sFileNameFormat,sFormatname );

	}

	///////////////////////////////////////////////
	LeaveCriticalSection(&m_CSSafe);

	return bResult;
}

//setting sLogFolder member of LOGGENCTL structure
//the last character must be '\'.
BOOL CTTR_LogSystem::SetFolderFormat( LPCTSTR sLogDircetory )
{
	BOOL bResult = TRUE;

	EnterCriticalSection(&m_CSSafe);
	///////////////////////////////////////////////

	if( sLogDircetory ){
		//setting m_sLogFolder
		_tcscpy( m_sLogFolderFormat,sLogDircetory );
	}
	else{
		bResult = FALSE;
	}

	///////////////////////////////////////////////
	LeaveCriticalSection(&m_CSSafe);

	return bResult;
}

BOOL CTTR_LogSystem::SetFileExt( LPCTSTR sExt )
{
	BOOL bResult = TRUE;

	EnterCriticalSection(&m_CSSafe);
	///////////////////////////////////////////////

	if( sExt ){
		//setting m_sLogFolder
		_tcscpy( m_sFileNameExt,sExt );
	}
	else{
		bResult = FALSE;
	}

	///////////////////////////////////////////////
	LeaveCriticalSection(&m_CSSafe);

	return bResult;
}


//setting nBaseLevel member of LOGGENCTL structure
BOOL CTTR_LogSystem::SetBaseLevel( INT nBaseLevel )
{
	BOOL bResult = TRUE;

	EnterCriticalSection(&m_CSSafe);
	///////////////////////////////////////////////
	m_nBaseLevel = nBaseLevel;
	///////////////////////////////////////////////
	LeaveCriticalSection(&m_CSSafe);

	return bResult;
}

//setting nMaxLog member of LOGGENCTL structure
BOOL CTTR_LogSystem::SetMaxLog( INT nMax )
{
	BOOL bResult = TRUE;

	EnterCriticalSection(&m_CSSafe);
	///////////////////////////////////////////////
	m_nMaxLog = nMax;
	///////////////////////////////////////////////
	LeaveCriticalSection(&m_CSSafe);

	return bResult;
}

	
//setting bEnable member of LOGGENCTL structure
BOOL CTTR_LogSystem::Enable( BOOL bEnable/*=TRUE*/ )
{
	BOOL bResult = TRUE;

	EnterCriticalSection(&m_CSSafe);
	///////////////////////////////////////////////
	m_nEnable = bEnable;
	///////////////////////////////////////////////
	LeaveCriticalSection(&m_CSSafe);

	return bResult;
}

//setting bEnableTimeStemp member of LOGGENCTL structure
BOOL CTTR_LogSystem::EnableTimeStemp( BOOL bEnable/*=TRUE*/ )
{
	BOOL bResult = TRUE;

	EnterCriticalSection(&m_CSSafe);
	///////////////////////////////////////////////
	m_nEnableTimeStemp = bEnable;
	///////////////////////////////////////////////
	LeaveCriticalSection(&m_CSSafe);

	return bResult;
}

//setting bEnableTimeTick member of LOGGENCTL structure
BOOL CTTR_LogSystem::EnableTimeTick( BOOL bEnable/*=TRUE*/ )
{
	BOOL bResult = TRUE;

	EnterCriticalSection(&m_CSSafe);
	///////////////////////////////////////////////
	m_nEnableTimeTick = bEnable;
	///////////////////////////////////////////////
	LeaveCriticalSection(&m_CSSafe);

	return bResult;
}



//load default value to member.
//nKeyIndex is index of ssKeyName
VOID CTTR_LogSystem::LoadDefaultValue( INT nKeyIndex )
{

	switch( nKeyIndex ){

		case 0:m_nEnable = CTTR_LogSystem::DefValue::DefV_enable;	break;
		case 1:m_nBaseLevel = CTTR_LogSystem::DefValue::DefV_disablelevel;	break;
		case 2:m_nEnableTimeStemp = CTTR_LogSystem::DefValue::DefV_printtime;	break;
		case 3:m_nEnableTimeTick = CTTR_LogSystem::DefValue::DefV_printtimetick;	break;
		case 4:m_nMaxLog = CTTR_LogSystem::DefValue::DefV_maxnumber;	break;

		case 5:_tcscpy(m_sLogFolderFormat, CTTR_LogSystem::DefValue::DefV_folder );	break;
		case 6:_tcscpy(m_sFileNameFormat,CTTR_LogSystem::DefValue::DefV_format );	break;
		case 7:_tcscpy(m_sFileNameExt, CTTR_LogSystem::DefValue::DefV_extention );	break;
		default:
			break;
	}//end switch
}


//logging
VOID CTTR_LogSystem::Log( INT nLevel,LPCTSTR pszFormat, ...)
{
	FILE *pstream=NULL;
	DWORD dwStTick=0;
	static BOOL bFirst = TRUE;
	
	struct tm *dt;
	va_list args;
	time_t t;


	EnterCriticalSection(&m_CSSafe);
	///////////////////////////////////////////////
	
	if( m_bStart ){

		if( m_nEnable!=0 && nLevel<m_nBaseLevel ){
		
			//open or create or empty
			pstream = _tfopen( m_sCurLogFullPath, _T("a+") );
			if( pstream ){

				va_start(args, pszFormat);
				
				if( m_nEnableTimeStemp ){
					t = time(NULL);
					dt = localtime(&t);
					_ftprintf( pstream, _T("{%02d%02d - %02d:%02d:%02d} "), dt->tm_mon+1,
									dt->tm_mday, dt->tm_hour, dt->tm_min, dt->tm_sec);
				}
				
				if( m_nEnableTimeTick ){
					if( bFirst ){
						dwStTick=GetTickCount();
						_ftprintf( pstream, _T("{%09u} "),0);
						bFirst=FALSE;
					}
					else
						_ftprintf( pstream, _T("{%09u} "),GetTickCount()-dwStTick);
				}
				
				_vftprintf( pstream,pszFormat,args );
				_ftprintf( pstream, _T("\n") );
				fflush( pstream );
				fclose( pstream );
			}//pstream
		}//m_nEnable!=0 && nLevel>=m_nBaseLevel

	}//m_bStart
	///////////////////////////////////////////////
	LeaveCriticalSection(&m_CSSafe);
}

VOID CTTR_LogSystem::Log( INT nLevel,LPCTSTR pszFormat,va_list argptr )
{
	FILE *pstream=NULL;
	DWORD dwStTick=0;
	static BOOL bFirst = TRUE;
	
	struct tm *dt;
	time_t t;


	EnterCriticalSection(&m_CSSafe);
	///////////////////////////////////////////////
	
	if( m_bStart ){

		if( m_nEnable!=0 && nLevel<m_nBaseLevel ){
		
			//open or create or empty
			pstream = _tfopen( m_sCurLogFullPath, _T("a+") );
			if( pstream ){

		
				if( m_nEnableTimeStemp ){
					t = time(NULL);
					dt = localtime(&t);
					_ftprintf( pstream, _T("{%02d%02d - %02d:%02d:%02d} "), dt->tm_mon+1,
									dt->tm_mday, dt->tm_hour, dt->tm_min, dt->tm_sec);
				}
				
				if( m_nEnableTimeTick ){
					if( bFirst ){
						dwStTick=GetTickCount();
						_ftprintf( pstream, _T("{%09u} "),0);
						bFirst=FALSE;
					}
					else
						_ftprintf( pstream, _T("{%09u} "),GetTickCount()-dwStTick);
				}
				
				_vftprintf( pstream,pszFormat,argptr );
				_ftprintf( pstream, _T("\n") );
				fflush( pstream );
				fclose( pstream );
			}//pstream
		}//m_nEnable!=0 && nLevel>=m_nBaseLevel

	}//m_bStart
	///////////////////////////////////////////////
	LeaveCriticalSection(&m_CSSafe);
}

//logging memory values
VOID CTTR_LogSystem::Dump( INT nLevel,LPBYTE lpbData,INT nSize )
{
	FILE *pstream=NULL;
	INT i;
	
	if( nSize<=0 )
		return;

	if( lpbData==NULL )
		return;
	///////////////////////
	EnterCriticalSection(&m_CSSafe);

	if( m_bStart ){

		if( m_nEnable!=0 && nLevel<m_nBaseLevel ){
		
			//open or create or empty
			pstream = _tfopen( m_sCurLogFullPath, _T("a+") );
			if( pstream ){

				for( i=0; i<nSize; i++ ){
					_ftprintf( pstream, _T("%02X,"),(BYTE)(lpbData[i]) );
				}//end for

				//line break
				_ftprintf( pstream, _T("\n") );

				fflush( pstream );
				fclose( pstream );
			}//pstream
		}//m_nEnable!=0 && nLevel>=m_nBaseLevel

	}//m_bStart

	LeaveCriticalSection(&m_CSSafe);

}


//logging only message
VOID CTTR_LogSystem::Dump( INT nLevel,LPCTSTR sMsg )
{
	FILE *pstream=NULL;
	
	if( sMsg==NULL )
		return;
	///////////////////////
	EnterCriticalSection(&m_CSSafe);

	if( m_bStart ){

		if( m_nEnable!=0 && nLevel<m_nBaseLevel ){
		
			//open or create or empty
			pstream = _tfopen( m_sCurLogFullPath, _T("a+") );
			if( pstream ){

				_ftprintf( pstream, _T("%s\n"),sMsg );

				fflush( pstream );
				fclose( pstream );
			}//pstream
		}//m_nEnable!=0 && nLevel>=m_nBaseLevel

	}//m_bStart

	LeaveCriticalSection(&m_CSSafe);

}



//Setting cuurent log file name
BOOL CTTR_LogSystem::SetCurLogFileName()
{
	BOOL bResult = TRUE;

	TCHAR sModuleFN[_MAX_PATH];
	TCHAR sAbsLogFolder[_MAX_PATH];
	TCHAR sLogFolder[_MAX_PATH];
	_tstring sPath;
	INT i;

	TCHAR sOnlyOneFN[_MAX_PATH];
	TCHAR sFormatFN[_MAX_PATH];

	CTTR_LogSystem::_TTR_FM_PARA fm_para;
	CTTR_LogSystem::ENUM_CheckResult CheckResult = CTTR_LogSystem::CR_ERROR;

	//
	//setting member ....... : m_sCurLogFullPath

	if( GetModuleFileName( m_hModule,sModuleFN,_MAX_PATH ) == 0 ){
		bResult = FALSE;
	}
	else{
		//generate log folder name.......
		SetFormatStringPara( &fm_para );
		CheckResult = CheckFomatString( m_sLogFolderFormat,&fm_para );
		if( CheckResult == CTTR_LogSystem::CR_ERROR ){
				bResult = FALSE;
		}
		else if( CheckResult == CTTR_LogSystem::CR_OK_STATIC ){
			//generate log file name is only one.
			MakeFomatString( sLogFolder,m_sLogFolderFormat,&fm_para );

		}
		else if( CheckResult == CTTR_LogSystem::CR_OK_DYNAMIC ){
			bResult = FALSE;
		}
		else{
			bResult = FALSE;
		}


		if( bResult ){

			//generate realtive from folder to abs folder . m_sLogFolderFormat
			TTR_GetAbsPathFromRelPathInModule( sAbsLogFolder,_MAX_PATH,m_hModule,sLogFolder );

			//make path . if no exsit path-folder, make folder automatically.
			if( sAbsLogFolder[_tcslen(sAbsLogFolder)-1] != _T('\\') ){
				//concate back slash
				_tcscat( sAbsLogFolder,_T("\\") );
			}
			sPath = sAbsLogFolder;

			bResult = TTR_CreatePath( sPath );//created.......

			if( bResult ){
				//Created log folder path
				SetFormatStringPara( &fm_para );
				CheckResult = CheckFomatString( m_sFileNameFormat,&fm_para );
				if( CheckResult == CTTR_LogSystem::CR_ERROR ){
					bResult = FALSE;
				}
				else if( CheckResult == CTTR_LogSystem::CR_OK_STATIC ){
					//generate log file name is only one.
					MakeFomatString( sOnlyOneFN,m_sFileNameFormat,&fm_para );
					
					sPath = sAbsLogFolder;	//add path
					sPath += sOnlyOneFN;	//add file name
					sPath += m_sFileNameExt;//add extension

					//save current log file name
					_tcscpy( m_sCurLogFullPath,sPath.c_str() );

					//check existing file
					if( TTR_IsExistFile( m_sCurLogFullPath ) ){
						//delete current file
						DeleteFile(m_sCurLogFullPath);
					}

				}
				else if( CheckResult == CTTR_LogSystem::CR_OK_DYNAMIC ){

					//get file format string for dynamic.
					MakeFomatString( sFormatFN,m_sFileNameFormat,&fm_para );

					vector<CTTR_LogSystem::_TTR_FA_ITEM> FcV;
					CTTR_LogSystem::_TTR_FA_ITEM FcItem;
					HANDLE hFile;
					BY_HANDLE_FILE_INFORMATION FInfo;

					for( i=0; i<m_nMaxLog; i++ ){

						_stprintf( sOnlyOneFN, sFormatFN,i );
						sPath = sAbsLogFolder;	//add path
						sPath += sOnlyOneFN;	//add file name
						sPath += m_sFileNameExt;//add extension

						//check exsitens file
						if( !TTR_IsExistFile( sPath.c_str() ) ){
							//exsiting file
							//save current log file name
							_tcscpy( m_sCurLogFullPath,sPath.c_str() );
							break;	//exit for
						}
						else{
							hFile=CreateFile( sPath.c_str(),GENERIC_READ,FILE_SHARE_READ, NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL, NULL);
							GetFileInformationByHandle( hFile, &FInfo );
							CloseHandle( hFile );
							//
							FcItem.sFileNmae = sPath;
							FcItem.ftLastAccessTime.dwHighDateTime = FInfo.ftLastWriteTime.dwHighDateTime;
							FcItem.ftLastAccessTime.dwLowDateTime = FInfo.ftLastWriteTime.dwLowDateTime;

							//
							FcV.push_back( FcItem );

						}

					}//end for

					if( i==m_nMaxLog ){
						//all file existence

						//search th oldest written file.
						//this file will be deleted.
						sort( FcV.begin(), FcV.end(),CB_VectorCompareFile );//sort . older file is the first index.

						//select the oldest file
						//delete file
						DeleteFile( FcV[0].sFileNmae.c_str() );

						//if delete-operation has been failed,
						// log file will is appended. 
						// therefore this case isn't error

						//save current log file name
						_tcscpy( m_sCurLogFullPath,FcV[0].sFileNmae.c_str() );
					}

					FcV.clear();


				}
				else{
					bResult = FALSE;
				}

				if( bResult ){
					//here m_sCurLogFullPath variable has a current log file name.......

				}

			}//
		}//m_sLogFolderFormat is OK

	}

	//
	return bResult;

}

//Set format string param
VOID CTTR_LogSystem::SetFormatStringPara( CTTR_LogSystem::_TTR_FM_PARA *pPara,INT nCnt/*=-1*/ )
{
	struct tm *dt;
	time_t t;


	if( pPara ){

		t = time(NULL);
		dt = localtime(&t);

		pPara->nCnt = nCnt;						//if negative, return format may be %d
		pPara->nYear = dt->tm_year+1900;
		pPara->nMonth = dt->tm_mon+1;
		pPara->nDay = dt->tm_mday;
		pPara->nHour = dt->tm_hour;
		pPara->nMinute = dt->tm_min;
		pPara->nSecond = dt->tm_sec;

		GetWindowsDirectory( pPara->sWindows,_MAX_PATH );	//GetWindowsDirectory
		GetWindowsDirectory( pPara->sSystem,_MAX_PATH );			//GetSystemDirectory
		GetWindowsDirectory( pPara->sSystemWindows,_MAX_PATH );	//GetSystemWindowsDirectory
		GetWindowsDirectory( pPara->sSystemWow64,_MAX_PATH );		//GetSystemWow64Directory
	}
}



//Check formatted string
CTTR_LogSystem::ENUM_CheckResult CTTR_LogSystem::CheckFomatString( LPTSTR psIn,CTTR_LogSystem::_TTR_FM_PARA *pPara )
{
	CTTR_LogSystem::ENUM_CheckResult Result = CTTR_LogSystem::CR_OK_STATIC;
	TCHAR ch;
	LPTSTR pTemp;
	TCHAR sWidth[10];
	TCHAR sTokenFormat[20];
	INT nCnt = 0;
	INT nWidth;

#ifdef UNICODE 
	CHAR strTmp[2*(10+1)];
#endif

	if( psIn ){

		ch = *psIn;

		if( ch == _T('%') ){

			psIn ++;//get nexr character

			//get width in format.
			pTemp = psIn;

			while( *pTemp>=_T('0') && *pTemp<=_T('9') ){

				pTemp ++;//get next character
				nCnt++;

			}//end while

			if( nCnt>0 ){
				_tcsncpy( sWidth,psIn,nCnt );
				sWidth[nCnt] = NULL;
				//here sWidth is width.

				//for digit with width
				_stprintf( sTokenFormat,_T("%%0%sd"),sWidth );

				//
			#ifdef UNICODE
				wcstombs(strTmp, (const wchar_t *)sWidth, sizeof(strTmp)); 
				nWidth=atoi( strTmp );
			#else
				nWidth=atoi( sWidth );
			#endif

			}
			else{
				//for digit without width
				_stprintf( sTokenFormat,_T("%%d") );
				nWidth = 0;
			}
			
			psIn += nCnt;//skip width part
			ch = *psIn;

			switch(ch){
				case _T('d')://0부터 시작하는 정수, 자동으로 증가됨.

					Result = CTTR_LogSystem::CR_OK_DYNAMIC;

					break;
				case _T('Y')://년도를 표시함.
				case _T('M')://월을 표시함.
				case _T('D')://날을 표시함.
				case _T('h')://시간을 나타냄.(24시간 형식)
				case _T('m')://분을 나타냄.
				case _T('s')://초를 나타냄.
					break;
				case _T('W')://WIN,WSY,WSW,WS6

					psIn ++;//next character
					if( *psIn == _T('I') ){

						psIn ++;//next character

						if( *psIn == _T('N') ){
							//
							if( pPara->sWindows==NULL )
								Result = CTTR_LogSystem::CR_ERROR;//unknown format
						}
						else{
							Result = CTTR_LogSystem::CR_ERROR;//unknown format
						}

					}
					else if( *psIn == _T('S') ){

						psIn ++;//next character

						if( *psIn == _T('Y') ){
							if( pPara->sSystem==NULL )
								Result = CTTR_LogSystem::CR_ERROR;//unknown format
						}
						else if( *psIn == _T('W') ){
							if( pPara->sSystemWindows==NULL )
								Result = CTTR_LogSystem::CR_ERROR;//unknown format
						}
						else if( *psIn == _T('6') ){
							if( pPara->sSystemWow64==NULL )
								Result = CTTR_LogSystem::CR_ERROR;//unknown format
						}
						else
							Result = CTTR_LogSystem::CR_ERROR;//unknown format
					}
					else
						Result = CTTR_LogSystem::CR_ERROR;//unknown format

					break;
				case _T('%'):
					break;
				default://generic character

					if( ch == NULL )
						Result = CTTR_LogSystem::CR_ERROR;//unknown format
					break;
			}//end switch


			psIn ++;
		}
		else{
			//generic character
			psIn ++;

		}

		if( Result == CTTR_LogSystem::CR_ERROR ){
			return Result;//return .for error
		}

		if( *psIn != NULL ){

			if( Result == CTTR_LogSystem::CR_OK_DYNAMIC ){

				if( CheckFomatString( psIn,pPara ) == CTTR_LogSystem::CR_ERROR ){
					Result = CTTR_LogSystem::CR_ERROR;
				}
			}
			else{
				Result = CheckFomatString( psIn,pPara );
			}
		}


	}
	else
		Result = CTTR_LogSystem::CR_ERROR;
	//

	return Result;
}



///////////////////////////////////////////////////////////////////////////////////////////

//Make formatted string
BOOL CTTR_LogSystem::MakeFomatString( LPTSTR psOut,LPTSTR psIn,CTTR_LogSystem::_TTR_FM_PARA *pPara )
{
	BOOL bResult = TRUE;
	TCHAR ch;
	LPTSTR pTemp;
	TCHAR sWidth[10];
	TCHAR sTokenFormat[20];
	TCHAR sFormat[20];
	INT nCnt = 0;
	INT nWidth, nDigit;

#ifdef UNICODE 
	CHAR strTmp[2*(10+1)];
#endif

	if( psIn ){

		ch = *psIn;

		if( ch == _T('%') ){

			psIn ++;//get nexr character

			//get width in format.
			pTemp = psIn;

			while( *pTemp>=_T('0') && *pTemp<=_T('9') ){

				pTemp ++;//get next character
				nCnt++;

			}//end while

			if( nCnt>0 ){
				_tcsncpy( sWidth,psIn,nCnt );
				sWidth[nCnt] = NULL;
				//here sWidth is width.

				//for digit with width
				_stprintf( sTokenFormat,_T("%%0%sd"),sWidth );

				//
			#ifdef UNICODE
				wcstombs(strTmp, (const wchar_t *)sWidth, sizeof(strTmp)); 
				nWidth=atoi( strTmp );
			#else
				nWidth=atoi( sWidth );
			#endif

			}
			else{
				//for digit without width
				_stprintf( sTokenFormat,_T("%%d") );
				nWidth = 0;
			}
			
			psIn += nCnt;//skip width part
			ch = *psIn;

			switch(ch){
				case _T('d')://0부터 시작하는 정수, 자동으로 증가됨.
					if( pPara->nCnt>=0 ){
						nDigit = TTR_GetCuttedDigit( pPara->nCnt,nWidth );
						_stprintf( sFormat,sTokenFormat,nDigit );
					}
					else{
						_tcscpy( sFormat,sTokenFormat );
					}
					break;
				case _T('Y')://년도를 표시함.
					nDigit = TTR_GetCuttedDigit( pPara->nYear,nWidth );
					_stprintf( sFormat,sTokenFormat,nDigit );
					break;
				case _T('M')://월을 표시함.
					nDigit = TTR_GetCuttedDigit( pPara->nMonth,nWidth );
					_stprintf( sFormat,sTokenFormat,nDigit );
					break;
				case _T('D')://날을 표시함.
					nDigit = TTR_GetCuttedDigit( pPara->nDay,nWidth );
					_stprintf( sFormat,sTokenFormat,nDigit );
					break;
				case _T('h')://시간을 나타냄.(24시간 형식)
					nDigit = TTR_GetCuttedDigit( pPara->nHour,nWidth );
					_stprintf( sFormat,sTokenFormat,nDigit );
					break;
				case _T('m')://분을 나타냄.
					nDigit = TTR_GetCuttedDigit( pPara->nMinute,nWidth );
					_stprintf( sFormat,sTokenFormat,nDigit );
					break;
				case _T('s')://초를 나타냄.
					nDigit = TTR_GetCuttedDigit( pPara->nSecond,nWidth );
					_stprintf( sFormat,sTokenFormat,nDigit );
					break;
				case _T('W')://WIN,WSY,WSW,WS6

					psIn ++;//next character
					if( *psIn == _T('I') ){

						psIn ++;//next character

						if( *psIn == _T('N') ){
							//
							if( pPara->sWindows )
								_stprintf( sFormat,_T("%s"),pPara->sWindows );
							else
								bResult = FALSE;//unknown format
						}
						else{
							bResult = FALSE;//unknown format
						}

					}
					else if( *psIn == _T('S') ){

						psIn ++;//next character

						if( *psIn == _T('Y') ){
							if( pPara->sSystem )
								_stprintf( sFormat,_T("%s"),pPara->sSystem );
							else
								bResult = FALSE;//unknown format
						}
						else if( *psIn == _T('W') ){
							if( pPara->sSystemWindows )
								_stprintf( sFormat,_T("%s"),pPara->sSystemWindows );
							else
								bResult = FALSE;//unknown format
						}
						else if( *psIn == _T('6') ){
							if( pPara->sSystemWow64 )
								_stprintf( sFormat,_T("%s"),pPara->sSystemWow64 );
							else
								bResult = FALSE;//unknown format
						}
						else
							bResult = FALSE;//unknown format
					}
					else
						bResult = FALSE;//unknown format

					break;
				case _T('%'):
					_stprintf( sFormat,_T("%%") );
					break;
				default://generic character

					if( ch != NULL )
						_stprintf( sFormat,_T("%c"),ch );
					else
						bResult = FALSE;//unknown format
					break;
			}//end switch

			if( bResult ){
				//copy 
				_tcscpy( psOut,sFormat );

				psOut += _tcslen(sFormat);
				psIn ++;
			}
		}
		else{
			//generic character
			*psOut = *psIn;//copy character

			psOut ++;
			psIn ++;

		}

		if( *psIn != NULL )
			bResult = MakeFomatString( psOut,psIn,pPara );
		else
			*psOut = NULL;	//make zero-string


	}
	else
		bResult = FALSE;
	//

	return bResult;
}


//Vector compare file callback function in sort()
// Item1 is less then Item2.
BOOL CTTR_LogSystem::CB_VectorCompareFile( 
	CTTR_LogSystem::_TTR_FA_ITEM Item1,
	CTTR_LogSystem::_TTR_FA_ITEM Item2
	)
{
	BOOL bResult = FALSE;

	if( Item1.ftLastAccessTime.dwHighDateTime > Item2.ftLastAccessTime.dwHighDateTime ){
		bResult = FALSE;
	}
	else if( Item1.ftLastAccessTime.dwHighDateTime == Item2.ftLastAccessTime.dwHighDateTime ){

		if( Item1.ftLastAccessTime.dwLowDateTime > Item2.ftLastAccessTime.dwLowDateTime ){
			bResult = FALSE;
		}
		else{
			bResult = TRUE;
		}

	}
	else{
		bResult = TRUE;
	}

	return bResult;
}


