
#if !defined(__TTRL_LOG_GENERATE_HEADER_20091112__)
#define __TTRL_LOG_GENERATE_HEADER_20091112__

#include "stdio.h"
#include "stdlib.h"
#include "TTR_def.h"
#include "TTR_tchar.h"


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#define		MAX_EL_LOG_NUMBER		10000
#define		DEF_EL_LOG_NUMBER		7

#define		MAX_EL_LOG_LEVEL		10

////////////////////////////////////////
#define		TTR_LOG_INT_KEY_NUM		5
#define		TTR_LOG_STR_KEY_NUM		3
#define		TTR_LOG_KEY_NUM			(TTR_LOG_INT_KEY_NUM+TTR_LOG_STR_KEY_NUM)


//
class CTTR_LogSystem
{

public:
	static class DefValue{

		public:

		static const INT DefV_enable;
		static const INT DefV_disablelevel;
		static const INT DefV_printtime;
		static const INT DefV_printtimetick;
		static const INT DefV_maxnumber;

		static const TCHAR DefV_folder[_MAX_PATH];
		static const TCHAR DefV_format[_MAX_PATH];
		static const TCHAR DefV_extention[_MAX_PATH];
		//

		static const TCHAR SectionName[TTR_DEF_NAME_BUF_SIZE];
		static const TCHAR ssKeyName[TTR_LOG_KEY_NUM][TTR_DEF_NAME_BUF_SIZE];

	};

	//formatting parameter
	struct _TTR_FM_PARA{
	
	INT nCnt;							//if negative, return format may be %d
	INT nYear;
	INT nMonth;
	INT nDay;
	INT nHour;
	INT nMinute;
	INT nSecond;

	TCHAR sWindows[_MAX_PATH];			//GetWindowsDirectory
	TCHAR sSystem[_MAX_PATH];			//GetSystemDirectory
	TCHAR sSystemWindows[_MAX_PATH];	//GetSystemWindowsDirectory
	TCHAR sSystemWow64[_MAX_PATH];		//GetSystemWow64Directory

	};

protected:
	//format string check result.
	//the return value of CheckFomatString()
	enum ENUM_CheckResult{
		CR_ERROR,
		CR_OK_STATIC,
		CR_OK_DYNAMIC
	};

	//fi[le accesstime compare structure
	struct _TTR_FA_ITEM{
		FILETIME ftLastAccessTime;	//the file last  access time or wrriten time
		_tstring sFileNmae;			//the file name
	};
	///////////
	// methods
public:
	//default constrcture
	CTTR_LogSystem();

	CTTR_LogSystem( HMODULE hModule );

	//Ini file constrcture
	CTTR_LogSystem( HMODULE hModule,LPCTSTR sInifilename );	

	//Ini registry constrcture
	//HKCU\Software\company_name\product_name\TTR_LogSystem' node

	//default destructure
	virtual ~CTTR_LogSystem();

	//Ini system by ini file
	BOOL Initial( HMODULE hModule,LPCTSTR sInifilename );

	
	//starting or stopping log system
	BOOL Start( BOOL bStart=TRUE );

	
	//setting sFileBaseName and sFileBaseExt member of LOGGENCTL structure
	BOOL SetFileNameFormat( LPCTSTR sFormatname );

	//the last character must be '\'.
	BOOL SetFolderFormat( LPCTSTR sLogDircetory );

	//setting log file extention
	BOOL SetFileExt( LPCTSTR sExt );

	//setting nBaseLevel member of LOGGENCTL structure
	BOOL SetBaseLevel( INT nBaseLevel );

	//setting nMaxLog member of LOGGENCTL structure
	BOOL SetMaxLog( INT nMax );

	
	//setting bEnable member of LOGGENCTL structure
	BOOL Enable( BOOL bEnable=TRUE );

	//setting bEnableTimeStemp member of LOGGENCTL structure
	BOOL EnableTimeStemp( BOOL bEnable=TRUE );

	//setting bEnableTimeTick member of LOGGENCTL structure
	BOOL EnableTimeTick( BOOL bEnable=TRUE );

	//logging
	VOID Log( INT nLevel,LPCTSTR pszFormat, ...);

	//logging
	VOID Log( INT nLevel,LPCTSTR pszFormat,va_list argptr );

	//logging memory values
	VOID Dump( INT nLevel,LPBYTE lpbData,INT nSize );

	//logging only message
	VOID Dump( INT nLevel,LPCTSTR sMsg );

	BOOL IsStart()
	{
		return m_bStart;
	}

protected:
	//load default value to member.
	//nKeyIndex is index of ssKeyName
	VOID LoadDefaultValue( INT nKeyIndex );

	//check format string
	CTTR_LogSystem::ENUM_CheckResult CTTR_LogSystem::CheckFomatString( LPTSTR psIn,CTTR_LogSystem::_TTR_FM_PARA *pPara );

	//Make formatted string
	BOOL MakeFomatString( LPTSTR psOut,LPTSTR psIn,CTTR_LogSystem::_TTR_FM_PARA *pPara );

	//Setting cuurent log file name
	BOOL SetCurLogFileName();

	//Set format string param
	VOID SetFormatStringPara( CTTR_LogSystem::_TTR_FM_PARA *pPara,INT nCnt=-1 );

	//Vector compare file callback function in sort()
	// Item1 is greater then Item2.
	static BOOL CB_VectorCompareFile( 
		CTTR_LogSystem::_TTR_FA_ITEM Item1,
		CTTR_LogSystem::_TTR_FA_ITEM Item2
		);

private:

	////////////
	//variables
public:
protected:

	static CRITICAL_SECTION m_CSSafe;		//critical section in a processor

	static const INT TTR_LOG_MaxlogNum = 7;
	static const INT TTR_LOG_Baselevel = 3;

	BOOL	m_bStart;				//start log or not

	INT		m_nEnable;			//32 bits value - enable or disable
	INT		m_nBaseLevel;		//log generate base level
	INT		m_nEnableTimeStemp;	//generate time stemp
	INT		m_nEnableTimeTick;	//generate time tick
	INT		m_nMaxLog;			//the number of maximum log files.


	TCHAR	m_sLogFolderFormat[_MAX_PATH];	//log generating folder.
	TCHAR	m_sFileNameFormat[_MAX_FNAME];	//log file format name(except extension & index)
	TCHAR	m_sFileNameExt[_MAX_EXT];		//log file extension(include dot)

	HMODULE	m_hModule;						//App module

	FILE	*m_pLogFileStream;
	INT		m_nCurIndex;					//log file current index number

	TCHAR	m_sCurLogFullPath[_MAX_PATH];	//th curent log full path


private:

};


//
#endif//__TTRL_LOG_GENERATE_HEADER_20091112__
