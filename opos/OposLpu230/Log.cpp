#include "stdafx.h"
#include "Log.h"
#include <time.h>
#include <shlobj.h>
#include <ct_ini_component.h>

using namespace std;
using namespace ATL;

CLog::CLog(void) :
	m_nLevel(LEV_LOW),
	m_bEnable(false),
	m_bTimeStemp(true),
	m_bSysTick(false)
{
	//::InitializeCriticalSectionAndSpinCount( &m_csLog, 0x4000 );
	ATLTRACE( _T(" $ CLog()::CLog().\n") );

	if( !m_Locker.IsIniOk() ){
		ATLTRACE( _T(" == FAIL CLog - Lock ini \n") );
	}

	struct tm *dt;
	time_t t;
	t = ::time(NULL);
	dt = ::localtime(&t);

	CString stemp;
	stemp.Format( _T(".\\log\\g%02d%02d%02d%02d%02d.txt"), dt->tm_mon+1,
							dt->tm_mday, dt->tm_hour, dt->tm_min, dt->tm_sec);
	
	m_sLogFilePath = stemp;

	m_sLogPath = _T(".\\log");	//default log folder.

}

CLog::~CLog(void)
{
	//::DeleteCriticalSection( &m_csLog );
	ATLTRACE( _T(" $ CLog()::~CLog().\n") );
}

CLog *CLog::GetLog( bool bFree /*= false*/ )
{
	static CLog *pLog = NULL;
	//

	if( bFree ){

		if( pLog )
			delete pLog;

		pLog = NULL;
	}
	else{

		if( pLog == NULL )
			pLog = new CLog();
	}

	return pLog;
}

bool CLog::Enable( bool bEnable /*= true*/ )
{
	bool bResult = true;

	//::EnterCriticalSection( &m_csLog );
	m_Locker.Lock( TIMEOUT_LOCK );

	m_bEnable = bEnable;

	//::LeaveCriticalSection( &m_csLog );
	m_Locker.Unlock();
	return bResult;
}

void CLog::Log(  bool bConsiderStemp,  Level nLevel, const TCHAR *_Format, ...)
{
	//::EnterCriticalSection( &m_csLog );
	m_Locker.Lock( TIMEOUT_LOCK );

	if( m_bEnable && m_nLevel >= nLevel ){

		struct tm *dt;
		va_list args;
		time_t t;
		FILE *pstream;

		if( !m_sLogFilePath.empty() ){

			pstream = ::_tfopen( m_sLogFilePath.c_str(), _T("a+") );

			if( pstream ){
				va_start(args, _Format);

				if( bConsiderStemp ){

					if( m_bTimeStemp ){
						t = ::time(NULL);
						dt = ::localtime(&t);

						_ftprintf( pstream, _T("[%02d-%02d %02d:%02d:%02d] "), dt->tm_mon+1,
							dt->tm_mday, dt->tm_hour, dt->tm_min, dt->tm_sec);
					}

					if( m_bSysTick ){
						DWORD dwStTick = ::GetTickCount();
						_ftprintf( pstream, _T("[%09u] "), dwStTick );
					}
				}

				int nLen = _vsctprintf( _Format, args )+1;

				TCHAR *buffer = new TCHAR[nLen];

				_vstprintf_s( buffer, nLen, _Format, args );
				_ftprintf( pstream, _T("%s"), buffer );

				va_end(args);

				delete [] buffer;

				_flushall();
				fclose( pstream );
			}
		}

	}

	//::LeaveCriticalSection( &m_csLog );
	m_Locker.Unlock();
}

bool CLog::Config( const _tstring & sIniFilePath, const std::wstring& s_log_file_prefix/*= std::wstring()*/)
{
	bool bResult = true;

	if( !sIniFilePath.empty() ){

		HANDLE hfile;
		hfile=CreateFile( sIniFilePath.c_str(),GENERIC_READ,FILE_SHARE_READ, NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL, NULL);
		if( hfile==INVALID_HANDLE_VALUE ){
			bResult = false;
		}
		else
			CloseHandle(hfile);

		if( bResult ){

			//make log path
			TCHAR sLogFolder[_MAX_PATH];
			TCHAR sDrv[_MAX_DIR];
			TCHAR sDir[_MAX_PATH];

			::_tsplitpath( sIniFilePath.c_str(), sDrv, sDir, NULL, NULL );
			::_tmakepath( sLogFolder, sDrv, sDir, NULL, NULL );

			m_sLogPath = sLogFolder;
			m_sLogPath += _T("log");

			_ns_tools::ct_file::is_exist_folder(m_sLogPath, true);

			struct tm *dt;
			time_t t;
			t = ::time(NULL);
			dt = ::localtime(&t);

			CString stemp;
			if (s_log_file_prefix.empty()) {
				stemp.Format(_T("log\\so_msr%02d%02d%02d%02d%02d.txt"), dt->tm_mon + 1,
					dt->tm_mday, dt->tm_hour, dt->tm_min, dt->tm_sec);
			}
			else {
				stemp.Format(_T("log\\%s%02d%02d%02d%02d%02d.txt"), s_log_file_prefix.c_str(), dt->tm_mon + 1,
					dt->tm_mday, dt->tm_hour, dt->tm_min, dt->tm_sec);
			}
			::PathAppend(  sLogFolder, stemp );

			m_sLogFilePath = sLogFolder;

			//
			DWORD dwResult;
			TCHAR sValue[_MAX_PATH];
			int nValue = 0;

			//::EnterCriticalSection( &m_csLog );
			m_Locker.Lock( TIMEOUT_LOCK );

			//setting enable property.......
			dwResult=::GetPrivateProfileString( _T("LogSetting"),_T("logenable"),_T("100"),sValue,_MAX_PATH,sIniFilePath.c_str() );
			if( dwResult == 0 )
				m_bEnable = false;
			else{
				nValue = _tstoi( sValue );

				if( nValue == 1 )
					m_bEnable = true;
				else
					m_bEnable = false;
			}

			//setting level property.......
			dwResult=::GetPrivateProfileString( _T("LogSetting"),_T("loglevel"),_T("100"),sValue,_MAX_PATH,sIniFilePath.c_str() );
			if( dwResult == 0 )
				m_nLevel = LEV_LOW;
			else{
				nValue = _tstoi( sValue );

				if( nValue >= 0 && nValue <= LEV_VERY_HIGH ){
					m_nLevel = static_cast<Level>(nValue);
				}
				else
					m_nLevel = LEV_LOW;
			}

			//setting time-stemp property.......
			dwResult=::GetPrivateProfileString( _T("LogSetting"),_T("logtimestemp"),_T("100"),sValue,_MAX_PATH,sIniFilePath.c_str() );
			if( dwResult == 0 )
				m_bTimeStemp = false;
			else{
				nValue = _tstoi( sValue );

				if( nValue == 1 )
					m_bTimeStemp = true;
				else
					m_bTimeStemp = false;
			}

			//setting time-tick property.......
			dwResult=::GetPrivateProfileString( _T("LogSetting"),_T("logtimetick"),_T("100"),sValue,_MAX_PATH,sIniFilePath.c_str() );
			if( dwResult == 0 )
				m_bSysTick = false;
			else{
				nValue = _tstoi( sValue );

				if( nValue == 1 )
					m_bSysTick = true;
				else
					m_bSysTick = false;
			}

			//::LeaveCriticalSection( &m_csLog );
			m_Locker.Unlock();
		}
	}

	return bResult;
}
