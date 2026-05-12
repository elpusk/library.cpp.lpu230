#pragma once

#include <string>
#include <Sync.h>

using namespace SYNC;

class CLog
{
public:
	enum Level{
		LEV_LOW = 0,			// only error
		LEV_NORMAL = 1,		// error + Called by CCO.
		LEV_HIGH = 2,			// error + Called by CCO + return to CCO
		LEV_VERY_HIGH = 3	// error + Called by CCO + return to CCO + Device IO
	};

private:
	enum{
		TIMEOUT_LOCK = 3000
	};
public:
	static CLog *GetLog( bool bFree = false );

public:
	virtual ~CLog(void);

	bool Enable( bool bEnable = true );

	void Log( bool bConsiderStemp, Level nLevel, const TCHAR *_Format, ...);

	bool Config( const _tstring & sIniFilePath, const std::wstring& s_log_file_prefix = std::wstring());

private:
	CLog(void);
	CLog( const CLog & );

private:
	//CRITICAL_SECTION m_csLog;
	CMutex m_Locker;

	Level m_nLevel;
	bool m_bEnable;

	bool m_bTimeStemp;
	bool m_bSysTick;

	_tstring m_sLogFilePath;
	_tstring m_sLogPath;
};

