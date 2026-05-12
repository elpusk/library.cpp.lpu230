//////////////////////////////////////////////////////////
//ELLogGen header file
// last coding - 2009.9.11 : 20090911
//////////////////////////////////////////////////////////

#if !defined(__EL_LOG_GENERATE_HEADER__)
#define __EL_LOG_GENERATE_HEADER__

#include "stdio.h"
#include "stdlib.h"

#define		MAX_EL_LOG_NUMBER		10000
#define		DEF_EL_LOG_NUMBER		7

#define		MAX_EL_LOG_LEVEL		10
////////////////////////////////////////////////
//the definition of elpusk Log control structure
//////////////////////////////////////////////
typedef	struct TagLOGGENCTL{
	BOOL bEnable;		//32 bits value - enable or disable
	long lBaseLevel;	//log generate base level
	BOOL bEnableTimeStemp;	//generate time stemp
	BOOL bEnableTimeTick;	//generate time tick
	BOOL bOnlyOpenGen;		//open at generating log and close.
	
	TCHAR sLogFolder[_MAX_PATH];		//log generating folder.
	TCHAR sFileBaseName[_MAX_FNAME];	//log base file name(except extension & index)
	TCHAR sFileBaseExt[_MAX_EXT];	//log file extension(include dot)
	FILE *pLogFileStream;
	long lCurIndex;				//log file current index number
	long lMaxLog;				//the number of maximum log files.
	HMODULE hModule;			//20090911 . called module(file )
	
}LOGGENCTL, *LPLOGGENCTL,*PLOGGENCTL;

//++ log file full path=prinf( "%s%s%d%s",sLogFolder,sFileBaseName,lMaxLog,sFileBaseExt)

//log file open & generate log with time-stemp and close file.
void TraceTimeTemp( const TCHAR *filename,const TCHAR *pszFormat, ...);

//log file open & generate log and close file.
void TraceTemp( const TCHAR *filename,const TCHAR *pszFormat, ...);

void Trace( PLOGGENCTL pLogCtl,long lLogLev,const TCHAR *pszFormat, ...);

//Trace without time mark( date,time,timetick and so on....... )
void TraceNoTimeMark( PLOGGENCTL pLogCtl,long lLogLev,const TCHAR *pszFormat, ...);

//Create Log control(allocate memory)
PLOGGENCTL CreateLogControl();

//20090911
//Create Log control(allocate memory) for supporting hModule member
PLOGGENCTL CreateLogControlWithModule( HMODULE hModule );


//Delete Log control(free memory)
void DeleteLogControl(PLOGGENCTL pLogCtl);

//Initial Log control structure by ini file
void IniLogControlByIniFile( PLOGGENCTL pLogCtl,const TCHAR *Inifilename );

//Initial Log control structure by registry
//long IniLogControlByRegistry( PLOGGENCTL pLogCtl );

//setting sFileBaseName and sFileBaseExt member of LOGGENCTL structure
BOOL SetLogBaseName( PLOGGENCTL pLogCtl,const TCHAR *basename );

//setting sLogFolder member of LOGGENCTL structure
//the last character must be '\'.
BOOL SetLogDirectory( PLOGGENCTL pLogCtl,const TCHAR *sLogDircetory );

//setting bEnable member of LOGGENCTL structure
BOOL EnableELLog( PLOGGENCTL pLogCtl,BOOL bEnable );

//setting bEnableTimeStemp member of LOGGENCTL structure
BOOL EnableELLogTimeStemp( PLOGGENCTL pLogCtl,BOOL bEnable );

//setting bEnableTimeTick member of LOGGENCTL structure
BOOL EnableELLogTimeTick( PLOGGENCTL pLogCtl,BOOL bEnable );

//setting bOnlyOpenGen member of LOGGENCTL structure
BOOL EnableELLogWriteOnlyOpen( PLOGGENCTL pLogCtl,BOOL bEnable );

//setting lBaseLevel member of LOGGENCTL structure
BOOL SetELLogBaseLevel( PLOGGENCTL pLogCtl,long lBaseLevel );

//setting lMaxLog member of LOGGENCTL structure
BOOL SetELLogMax( PLOGGENCTL pLogCtl,long lMax );

//
#endif//__EL_LOG_GENERATE_HEADER__
