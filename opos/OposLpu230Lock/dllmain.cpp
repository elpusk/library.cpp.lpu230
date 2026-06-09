// dllmain.cpp : DllMain이 구현된 것입니다.

#include "stdafx.h"
#include "resource.h"
#include "OposLpu230Lock_i.h"
#include "dllmain.h"
#include "Log.h"
#include <shlobj.h>
#include "DetectWnd.h"
#include "SoLock.h"
#include "EL_Support.h"
#include "GlobalVar.h"
#include <stdlib.h>
#include <tchar.h>
#include <ct_ini_component.h>

COposLpu230LockModule _AtlModule;

static void _process_attach( HINSTANCE hInstance );
static void ProcessDetach();
static void IniVariables( HINSTANCE hInstance );


// DLL 진입점입니다.
extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	switch( dwReason ){
		case DLL_PROCESS_ATTACH:
			ATLTRACE( _T(" = DllMain : DLL_PROCESS_ATTACH.\n") );
			_process_attach( hInstance );
			break;
		case DLL_PROCESS_DETACH:
			ATLTRACE( _T(" = DllMain : DLL_PROCESS_DETACH+.\n") );
			ProcessDetach();
			ATLTRACE( _T(" = DllMain : DLL_PROCESS_DETACH-.\n") );
			break;
		case DLL_THREAD_ATTACH:
			ATLTRACE( _T(" = DllMain : DLL_THREAD_ATTACH.\n") );
			break;
		case DLL_THREAD_DETACH:
			ATLTRACE( _T(" = DllMain : DLL_THREAD_DETACH.\n") );
			break;
		default:
			break;
	}//end switch

	return _AtlModule.DllMain(dwReason, lpReserved); 
}


void _process_attach( HINSTANCE hInstance )
{
	CLog *pLog = NULL;
	CDetectWnd *pDetectWnd = NULL;
	CSoLock *pSoLock = NULL;

	pLog = CLog::GetLog();
	ATLTRACE( _T(" * DllMain::pLog = 0x%x.\n"),pLog );

	do {
		if (!pLog) {
			continue;
		}
		_ns_tools::ct_ini_component::get_instance().set_module(hInstance).set_old_ini_file(L"lpu230Lock.ini");

		std::wstring s_ini(_ns_tools::ct_ini_component::get_instance().get_ini_path());
		if (s_ini.empty())
			continue;

		std::wstring s_log_prefix(_ns_tools::ct_ini_component::get_instance().get_cur_module_file_name() + L"_");
		pLog->Config(s_ini, s_log_prefix);

		std::wstring s_dll(_ns_tools::ct_ini_component::get_instance().get_cur_module_name());
		s_dll = L"*****Attach " + s_dll + L" *****\n";
		pLog->Log(true, CLog::LEV_LOW, s_dll.c_str());
		pLog->Log(true, CLog::LEV_LOW, _T(" ***** START LPU230Lock Service Object *****\n"));

		_ns_tools::ct_ini_component::get_instance().load_ini_file();
	} while (false);

	IniVariables(hInstance);

	pDetectWnd = CDetectWnd::GetInstance(hInstance);

	if( pDetectWnd ){
		pLog->Log( true, CLog::LEV_LOW, _T(" : SUCCESS : CREATE : DETECT WND.\n") );
	}

	//CSoMsr::GetInstance();
	
#ifdef	_YSS_ENABLE_STDOUT_FOR_DEBUG_
	::AllocConsole();
	_tfreopen( _T("CONOUT$"), _T("wt"), stdout );
	_tprintf( _T(">> Start console << \n") );
#endif		//_YSS_ENABLE_STDOUT_FOR_DEBUG_

}

void ProcessDetach()
{
	CLog *pLog = NULL;

	//CSoLock::GetInstance(true);//free

	CDetectWnd::GetInstance(NULL,true);//free
	pLog = CLog::GetLog();
	pLog->Log( true, CLog::LEV_LOW, _T(" = DllMain : DLL_PROCESS_DETACH.\n") );
	CLog::GetLog(true);//free log

#ifdef	_YSS_ENABLE_STDOUT_FOR_DEBUG_
			::FreeConsole();
#endif		//_YSS_ENABLE_STDOUT_FOR_DEBUG_

}

static void IniVariables( HINSTANCE hInstance )
{
	CGlobalVar *pVariables = NULL;
	CLog *pLog = CLog::GetLog();

	TCHAR sFullPath[MAX_PATH];
	EL_GetCurFullPath( sFullPath, (HMODULE)hInstance );
	pLog->Log( true, CLog::LEV_LOW, _T(" : CURRENT PATH : %s\n"),sFullPath );

	pVariables = CGlobalVar::GetInstance();
	
	if( pVariables == NULL )
		return;

	pVariables->setInstanceHandle( hInstance );
					
	TCHAR sDrv[_MAX_DRIVE], sDir[_MAX_DIR];
	::_tsplitpath( sFullPath, sDrv, sDir, NULL, NULL );
	pVariables->setWorkPath( _tstring( sFullPath ) );

	//build control ini file full path
	std::wstring s_ini(_ns_tools::ct_ini_component::get_instance().get_ini_path());
	if (!s_ini.empty()) {
		pVariables->getIniFile().setFile(s_ini);
	}

	// tg_lpu237_ibutton.dll 은 OposLpu230Lock.dll 과 같은 폴더에 있어야 한다.
	TCHAR sDir_dll[_MAX_DIR];
	TCHAR sDll_Path_buffer[_MAX_PATH];
	_tcscpy( sDir_dll, sDir );
	::_tmakepath( sDll_Path_buffer,sDrv, sDir_dll, _T("tg_lpu237_ibutton"), _T("dll") );
	pVariables->set_ibutton_dll_path( _tstring(sDll_Path_buffer) );

	::_tmakepath( sDll_Path_buffer,sDrv, sDir_dll, _T("BTCHidDev"), _T("dll") );
	pVariables->set_ibutton_btc_dll_path( _tstring(sDll_Path_buffer) );

	//load control variables
	pLog->Log( true, CLog::LEV_HIGH, _T(" = CONTROL VARIABLES = \n") );

	int nVal = 0;
	TCHAR *pEnd = NULL;

	//[keylock] section
	_tstring sSection( _T("keylock") );
	_tstring sKey( _T("size") );

	if( pVariables->getIniFile().LoadKey( sSection,sKey ) ){
		pLog->Log( true, CLog::LEV_HIGH, _T("( %s : %s ) = %s.\n"),sSection.c_str(), sKey.c_str(), pVariables->getIniFile().getValue( sSection,sKey ).c_str() );
		nVal = ::_tcstol( pVariables->getIniFile().getValue( sSection,sKey ).c_str(), &pEnd, 10 ); 
		if( nVal == 6 ){
			pVariables->set_lock_key_size( nVal );
		}
		else{
			pVariables->set_lock_key_size( 8 );
		}
	}

	sKey = _T("generate_exception_setbinaryconversion");
	if (pVariables->getIniFile().LoadKey(sSection, sKey)) {
		pLog->Log(true, CLog::LEV_HIGH, _T("( %s : %s ) = %s.\n"), sSection.c_str(), sKey.c_str(), pVariables->getIniFile().getValue(sSection, sKey).c_str());

		if (pVariables->getIniFile().getValue(sSection, sKey).compare(L"0") == 0) {
			pVariables->set_generate_exception_in_set_binary_conversion(false);
		}
		else {
			pVariables->set_generate_exception_in_set_binary_conversion(true);
		}
	}
	else {
		pVariables->set_generate_exception_in_set_binary_conversion(true);
	}

}
