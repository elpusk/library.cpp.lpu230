#pragma once

#include <cstddef>
#include <string>
#
#include <tg_lpu237_dll.h>

class CLpu237Dll
{
private:
	typedef	unsigned long (_CALLTYPE_ *typeLPU237_get_list)( wchar_t* );
	typedef	HANDLE (_CALLTYPE_ *typeLPU237_open)( const wchar_t*  );

	typedef	unsigned long (_CALLTYPE_ *typeLPU237_close)( HANDLE  );
	typedef	unsigned long (_CALLTYPE_ *typeLPU237_enable)( HANDLE  );
	typedef	unsigned long (_CALLTYPE_ *typeLPU237_disable)( HANDLE  );
	typedef	unsigned long (_CALLTYPE_ *typeLPU237_cancel_wait_swipe)( HANDLE  );
	typedef	unsigned long (_CALLTYPE_ *typeLPU237_wait_swipe_with_waits)( HANDLE  );
	typedef	unsigned long (_CALLTYPE_ *typeLPU237_wait_swipe_with_callback)( HANDLE , type_callback, void*  );
	typedef	unsigned long (_CALLTYPE_ *typeLPU237_wait_swipe_with_message)( HANDLE, HWND , UINT );
	typedef	unsigned long (_CALLTYPE_ *typeLPU237_get_data)( unsigned long, unsigned long, unsigned char* );
	typedef	unsigned long (_CALLTYPE_ *typeLPU237_dll_on)();
	typedef	unsigned long (_CALLTYPE_ *typeLPU237_dll_off)();
	typedef	unsigned long (_CALLTYPE_ *typeLPU237_get_id)( HANDLE hDev, unsigned char *sId );

	// SCR functions
	typedef	unsigned long (_CALLTYPE_ *typeLPU237_SCR_bypass_IccPowerOn)( HANDLE hDev, unsigned char cPower, unsigned char *sRx,	unsigned long* lpnRx );
	typedef	unsigned long (_CALLTYPE_ *typeLPU237_SCR_bypass_IccPowerOff)( HANDLE hDev );
	typedef	unsigned long (_CALLTYPE_ *typeLPU237_SCR_bypass_XfrBlock)( HANDLE hDev, unsigned char cBWI,const unsigned char *sTx,	unsigned long nTx,	unsigned char *sRx,unsigned long* lpnRx);
	typedef	unsigned long (_CALLTYPE_ *typeLPU237_SCR_bypass_GetParameters)( HANDLE hDev, unsigned char *sRx, unsigned long* lpnRx  );
	typedef	unsigned long (_CALLTYPE_ *typeLPU237_SCR_bypass_SetParameters)( HANDLE hDev,unsigned char cProtocol,unsigned char bmFindexDindex,	unsigned char bmTCCKST,unsigned char bGuardTime,unsigned char bWaitingInteger,unsigned char bIFSC );
	typedef	unsigned long (_CALLTYPE_ *typeLPU237_SCR_bypass_ResetParameters)( HANDLE hDev );
	typedef	unsigned long (_CALLTYPE_ *typeLPU237_SCR_bypass_GetSlotStatus)( HANDLE hDev );
	typedef	unsigned long (_CALLTYPE_ *typeLPU237_SCR_bypass_Escape)( HANDLE hDev,const unsigned char *sTx,	unsigned long nTx,	unsigned char *sRx,unsigned long* lpnRx);

	//SCR helper function
	typedef	unsigned long (_CALLTYPE_ *typeLPU237_SCR_helper_GetLastError)();
	typedef	BOOL (_CALLTYPE_ *typeLPU237_SCR_helper_GetFirmwareVersion)( HANDLE hDev, unsigned char *sRx, unsigned long* lpnRx );
	typedef	BOOL (_CALLTYPE_ *typeLPU237_SCR_helper_GetReaderMode)( HANDLE hDev, unsigned char *lpcMode  );
	typedef	BOOL (_CALLTYPE_ *typeLPU237_SCR_helper_SetReaderMode)( HANDLE hDev, unsigned char cMode, unsigned char cPower );
	typedef	BOOL (_CALLTYPE_ *typeLPU237_SCR_helper_XfrBlock)( HANDLE hDev,const unsigned char *sTx,unsigned long nTx,unsigned char *sRx,unsigned long* lpnRx);
	typedef	BOOL (_CALLTYPE_ *typeLPU237_SCR_helper_SetCardParameters)( HANDLE hDev, unsigned char cProtocol, unsigned char cParameter, unsigned char cNewValue );
	typedef	BOOL (_CALLTYPE_ *typeLPU237_SCR_helper_ResetCardParameters)( HANDLE hDev );
	typedef	BOOL (_CALLTYPE_ *typeLPU237_SCR_helper_result_is_success)( unsigned long dwScrResult );
	typedef	BOOL (_CALLTYPE_ *typeLPU237_SCR_helper_SetParameters)( HANDLE hDev ,const unsigned char *sAtr, unsigned long nAtr );
	typedef	BOOL (_CALLTYPE_ *typeLPU237_SCR_helper_GetATRParameterString)( wchar_t *sOut, unsigned long* lpnOut, const unsigned char *sAtr,	unsigned long nAtr, wchar_t cDelimiter );

private:
	CLpu237Dll(void);
	CLpu237Dll( const wchar_t* sDll );
public:
	static std::string get_mcsc_from_unicode(const std::wstring& s_unicode)
	{
		std::string s_mcsc;

		do {
			if (s_unicode.empty())
				continue;
			//
#ifdef _WIN32
			size_t size_needed = 0;
			wcstombs_s(&size_needed, nullptr, 0, s_unicode.c_str(), _TRUNCATE);
			if (size_needed > 0)
			{
				s_mcsc.resize(size_needed);
				wcstombs_s(&size_needed, &s_mcsc[0], size_needed, s_unicode.c_str(), _TRUNCATE);
			}
#else
			size_t size_needed = std::wcstombs(nullptr, s_unicode.c_str(), 0);
			if (size_needed != (size_t)-1)
			{
				s_mcsc.resize(size_needed);
				std::wcstombs(&s_mcsc[0], s_unicode.c_str(), size_needed);
			}
#endif
			else
			{
				s_mcsc.clear(); //default for error
			}

		} while (false);

		return s_mcsc;
	}

	static CLpu237Dll *get_instance(const wchar_t* sDll = NULL)
	{
		static CLpu237Dll dll;

		if( sDll )
			dll.Load( sDll );

		return &dll;
	}

	virtual ~CLpu237Dll(void);

	bool Load( const wchar_t* sDll );

	void Unload();
	//
	unsigned long LPU237_get_list( wchar_t* sMultiPaths )
	{
		if( m_FunLPU237_get_list )	return m_FunLPU237_get_list( sMultiPaths );
		else								return 0;
	}

	HANDLE LPU237_open( const wchar_t* sPath )
	{
		if( m_FunLPU237_open )		return m_FunLPU237_open( sPath );
		else								return INVALID_HANDLE_VALUE ;
	}

	unsigned long LPU237_close( HANDLE  hDev )
	{
		if( m_FunLPU237_close )	return m_FunLPU237_close( hDev );
		else							return LPU237_DLL_RESULT_ERROR;
	}

	unsigned long LPU237_enable( HANDLE  hDev )
	{
		if( m_FunLPU237_enable )	return m_FunLPU237_enable( hDev );
		else								return LPU237_DLL_RESULT_ERROR;
	}

	unsigned long LPU237_disable( HANDLE  hDev )
	{
		if( m_FunLPU237_disable )	return m_FunLPU237_disable( hDev );
		else								return LPU237_DLL_RESULT_ERROR;
	}

	unsigned long LPU237_cancel_wait_swipe( HANDLE  hDev )
	{
		if( m_FunLPU237_cancel_wait_swipe )	return m_FunLPU237_cancel_wait_swipe( hDev );
		else											return LPU237_DLL_RESULT_ERROR;
	}

	unsigned long LPU237_wait_swipe_with_waits( HANDLE  hDev )
	{
		if( m_FunLPU237_wait_swipe_with_waits )	return m_FunLPU237_wait_swipe_with_waits( hDev );
		else													return LPU237_DLL_RESULT_ERROR;
	}

	unsigned long LPU237_wait_swipe_with_callback( HANDLE  hDev, type_callback pFun, void *pParameter )
	{
		if( m_FunLPU237_wait_swipe_with_callback )	return m_FunLPU237_wait_swipe_with_callback( hDev, pFun, pParameter );
		else														return LPU237_DLL_RESULT_ERROR;
	}

	unsigned long LPU237_wait_swipe_with_message( HANDLE  hDev, HWND hWnd, UINT nMsg )
	{
		if( m_FunLPU237_wait_swipe_with_message )	return m_FunLPU237_wait_swipe_with_message( hDev, hWnd, nMsg );
		else														return LPU237_DLL_RESULT_ERROR;
	}

	unsigned long LPU237_get_data( unsigned long dwBufferIndex, unsigned long dwIsoTrack, unsigned char *sTrackData )
	{
		if( m_FunLPU237_get_data )	return m_FunLPU237_get_data( dwBufferIndex, dwIsoTrack, sTrackData );
		else								return LPU237_DLL_RESULT_ERROR;
	}

	unsigned long LPU237_dll_on()
	{
		if( m_FunLPU237_dll_on )	return m_FunLPU237_dll_on();
		else							return LPU237_DLL_RESULT_ERROR;
	}

	unsigned long LPU237_dll_off()
	{
		if( m_FunLPU237_dll_off )	return m_FunLPU237_dll_off();
		else							return LPU237_DLL_RESULT_ERROR;
	}

	unsigned long LPU237_get_id( HANDLE hDev, unsigned char *sId )
	{
		if( m_FunLPU237_get_id )	return m_FunLPU237_get_id( hDev, sId );
		else							return LPU237_DLL_RESULT_ERROR;
	}

	unsigned long LPU237_SCR_bypass_IccPowerOn( HANDLE hDev, unsigned char cPower, unsigned char *sRx,	unsigned long* lpnRx )
	{
		if( m_FunLPU237_SCR_bypass_IccPowerOn )	return m_FunLPU237_SCR_bypass_IccPowerOn( hDev, cPower, sRx, lpnRx );
		else							return LPU237_DLL_SCR_RESULT_STATUS_CMD_FAIL | LPU237_DLL_SCR_RESULT_STATUS_ICC_UNKNOWN;
	}

	unsigned long LPU237_SCR_bypass_IccPowerOff( HANDLE hDev )
	{
		if( m_FunLPU237_SCR_bypass_IccPowerOff )	return m_FunLPU237_SCR_bypass_IccPowerOff( hDev );
		else							return LPU237_DLL_SCR_RESULT_STATUS_CMD_FAIL | LPU237_DLL_SCR_RESULT_STATUS_ICC_UNKNOWN;
	}

	unsigned long LPU237_SCR_bypass_XfrBlock( HANDLE hDev, unsigned char cBWI,const unsigned char *sTx,unsigned long nTx,unsigned char *sRx,unsigned long* lpnRx)
	{
		if( m_FunLPU237_SCR_bypass_XfrBlock )	return m_FunLPU237_SCR_bypass_XfrBlock( hDev,cBWI,sTx,nTx, sRx,lpnRx );
		else							return LPU237_DLL_SCR_RESULT_STATUS_CMD_FAIL | LPU237_DLL_SCR_RESULT_STATUS_ICC_UNKNOWN;
	}

	unsigned long LPU237_SCR_bypass_GetParameters( HANDLE hDev, unsigned char *sRx, unsigned long* lpnRx  )
	{
		if( m_FunLPU237_SCR_bypass_GetParameters )	return m_FunLPU237_SCR_bypass_GetParameters( hDev,sRx,lpnRx );
		else							return LPU237_DLL_SCR_RESULT_STATUS_CMD_FAIL | LPU237_DLL_SCR_RESULT_STATUS_ICC_UNKNOWN;
	}

	unsigned long LPU237_SCR_bypass_SetParameters( HANDLE hDev,unsigned char cProtocol,unsigned char bmFindexDindex,	unsigned char bmTCCKST,unsigned char bGuardTime,unsigned char bWaitingInteger,unsigned char bIFSC )
	{
		if( m_FunLPU237_SCR_bypass_SetParameters )	return m_FunLPU237_SCR_bypass_SetParameters( hDev,cProtocol, bmFindexDindex,bmTCCKST,bGuardTime, bWaitingInteger, bIFSC   );
		else							return LPU237_DLL_SCR_RESULT_STATUS_CMD_FAIL | LPU237_DLL_SCR_RESULT_STATUS_ICC_UNKNOWN;
	}

	unsigned long LPU237_SCR_bypass_ResetParameters( HANDLE hDev )
	{
		if( m_FunLPU237_SCR_bypass_ResetParameters )	return m_FunLPU237_SCR_bypass_ResetParameters( hDev );
		else							return LPU237_DLL_SCR_RESULT_STATUS_CMD_FAIL | LPU237_DLL_SCR_RESULT_STATUS_ICC_UNKNOWN;
	}

	unsigned long LPU237_SCR_bypass_GetSlotStatus( HANDLE hDev )
	{
		if( m_FunLPU237_SCR_bypass_GetSlotStatus )	return m_FunLPU237_SCR_bypass_GetSlotStatus( hDev );
		else							return LPU237_DLL_SCR_RESULT_STATUS_CMD_FAIL | LPU237_DLL_SCR_RESULT_STATUS_ICC_UNKNOWN;
	}

	unsigned long LPU237_SCR_bypass_Escape( HANDLE hDev,const unsigned char *sTx,unsigned long nTx,unsigned char *sRx,unsigned long* lpnRx)
	{
		if( m_FunLPU237_SCR_bypass_Escape )	return m_FunLPU237_SCR_bypass_Escape( hDev,sTx, nTx, sRx, lpnRx );
		else							return LPU237_DLL_SCR_RESULT_STATUS_CMD_FAIL | LPU237_DLL_SCR_RESULT_STATUS_ICC_UNKNOWN;
	}

	unsigned long LPU237_SCR_helper_GetLastError()
	{
		if( m_FunLPU237_SCR_helper_GetLastError )	return m_FunLPU237_SCR_helper_GetLastError();
		else							return LPU237_DLL_SCR_RESULT_STATUS_CMD_FAIL | LPU237_DLL_SCR_RESULT_STATUS_ICC_UNKNOWN;
	}

	BOOL LPU237_SCR_helper_GetFirmwareVersion( HANDLE hDev, unsigned char *sRx, unsigned long* lpnRx )	
	{
		if( m_FunLPU237_SCR_helper_GetFirmwareVersion )	return m_FunLPU237_SCR_helper_GetFirmwareVersion( hDev, sRx, lpnRx );
		else							return FALSE;
	}

	BOOL LPU237_SCR_helper_GetReaderMode( HANDLE hDev, unsigned char *lpcMode  )
	{
		if( m_FunLPU237_SCR_helper_GetReaderMode )	return m_FunLPU237_SCR_helper_GetReaderMode( hDev, lpcMode );
		else							return FALSE;
	}

	BOOL LPU237_SCR_helper_SetReaderMode( HANDLE hDev, unsigned char cMode, unsigned char cPower )
	{
		if( m_FunLPU237_SCR_helper_SetReaderMode )	return m_FunLPU237_SCR_helper_SetReaderMode( hDev, cMode,cPower );
		else							return FALSE;
	}

	BOOL LPU237_SCR_helper_XfrBlock( HANDLE hDev,const unsigned char *sTx,	unsigned long nTx,unsigned char *sRx,unsigned long* lpnRx)
	{
		if( m_FunLPU237_SCR_helper_XfrBlock )	return m_FunLPU237_SCR_helper_XfrBlock( hDev, sTx, nTx, sRx, lpnRx );
		else							return FALSE;
	}

	BOOL LPU237_SCR_helper_SetCardParameters( HANDLE hDev, unsigned char cProtocol, unsigned char cParameter, unsigned char cNewValue )
	{
		if( m_FunLPU237_SCR_helper_SetCardParameters )	return m_FunLPU237_SCR_helper_SetCardParameters( hDev, cProtocol, cParameter, cNewValue );
		else							return FALSE;
	}

	BOOL LPU237_SCR_helper_ResetCardParameters( HANDLE hDev )
	{
		if( m_FunLPU237_SCR_helper_ResetCardParameters )	return m_FunLPU237_SCR_helper_ResetCardParameters( hDev );
		else							return FALSE;
	}

	BOOL LPU237_SCR_helper_result_is_success( unsigned long dwScrResult )
	{
		if( m_FunLPU237_SCR_helper_result_is_success )	return m_FunLPU237_SCR_helper_result_is_success( dwScrResult );
		else							return FALSE;
	}

	BOOL LPU237_SCR_helper_SetParameters( HANDLE hDev ,const unsigned char *sAtr, unsigned long nAtr )
	{
		if( m_FunLPU237_SCR_helper_SetParameters )	return m_FunLPU237_SCR_helper_SetParameters( hDev, sAtr, nAtr );
		else							return FALSE;
	}

	BOOL LPU237_SCR_helper_GetATRParameterString( wchar_t *sOut, unsigned long* lpnOut, const unsigned char *sAtr,unsigned long nAtr, wchar_t cDelimiter )
	{
		if( m_FunLPU237_SCR_helper_GetATRParameterString )	return m_FunLPU237_SCR_helper_GetATRParameterString( sOut, lpnOut, sAtr, nAtr,cDelimiter );
		else							return FALSE;
	}

private:
	void _ini();

private:
#ifdef _WIN32
	HMODULE _load_lib(const std::wstring& s_lib)
	{
		if (s_lib.empty())
			return NULL;
		return ::LoadLibrary(s_lib.c_str());
	}
	void _free_lib(HMODULE m)
	{
		FreeLibrary(m);
	}

	FARPROC WINAPI _load_symbol(HMODULE m, const char* s_fun)
	{
		return ::GetProcAddress(m, s_fun);
	}

#else
	HMODULE _load_lib(const std::wstring& s_lib)
	{
		if (s_lib.empty())
			return NULL;
		//
		return dlopen(CLpu237Dll::get_mcsc_from_unicode(s_lib).c_str(), RTLD_LAZY);
	}

	void _free_lib(HMODULE m)
	{
		dlclose(m);
	}

	void* _load_symbol(HMODULE m, const char* s_fun)
	{
		return dlsym(m, s_fun);
	}

#endif // _WIN32

private:
	HMODULE m_hMode;

	typeLPU237_get_list m_FunLPU237_get_list;
	typeLPU237_open m_FunLPU237_open;
	typeLPU237_close m_FunLPU237_close;
	typeLPU237_enable m_FunLPU237_enable;
	typeLPU237_disable m_FunLPU237_disable;
	typeLPU237_cancel_wait_swipe m_FunLPU237_cancel_wait_swipe;
	typeLPU237_wait_swipe_with_waits m_FunLPU237_wait_swipe_with_waits;
	typeLPU237_wait_swipe_with_callback m_FunLPU237_wait_swipe_with_callback;
	typeLPU237_wait_swipe_with_message m_FunLPU237_wait_swipe_with_message;
	typeLPU237_get_data m_FunLPU237_get_data;
	typeLPU237_dll_on m_FunLPU237_dll_on;
	typeLPU237_dll_off m_FunLPU237_dll_off;
	typeLPU237_get_id m_FunLPU237_get_id;

	typeLPU237_SCR_bypass_IccPowerOn		m_FunLPU237_SCR_bypass_IccPowerOn;
	typeLPU237_SCR_bypass_IccPowerOff		m_FunLPU237_SCR_bypass_IccPowerOff;
	typeLPU237_SCR_bypass_XfrBlock				m_FunLPU237_SCR_bypass_XfrBlock;
	typeLPU237_SCR_bypass_GetParameters		m_FunLPU237_SCR_bypass_GetParameters;
	typeLPU237_SCR_bypass_SetParameters		m_FunLPU237_SCR_bypass_SetParameters;
	typeLPU237_SCR_bypass_ResetParameters	m_FunLPU237_SCR_bypass_ResetParameters;
	typeLPU237_SCR_bypass_GetSlotStatus		m_FunLPU237_SCR_bypass_GetSlotStatus;
	typeLPU237_SCR_bypass_Escape				m_FunLPU237_SCR_bypass_Escape;

	typeLPU237_SCR_helper_GetLastError				m_FunLPU237_SCR_helper_GetLastError;
	typeLPU237_SCR_helper_GetFirmwareVersion		m_FunLPU237_SCR_helper_GetFirmwareVersion;
	typeLPU237_SCR_helper_GetReaderMode			m_FunLPU237_SCR_helper_GetReaderMode;
	typeLPU237_SCR_helper_SetReaderMode			m_FunLPU237_SCR_helper_SetReaderMode;
	typeLPU237_SCR_helper_XfrBlock						m_FunLPU237_SCR_helper_XfrBlock;
	typeLPU237_SCR_helper_SetCardParameters			m_FunLPU237_SCR_helper_SetCardParameters;
	typeLPU237_SCR_helper_ResetCardParameters		m_FunLPU237_SCR_helper_ResetCardParameters;
	typeLPU237_SCR_helper_result_is_success			m_FunLPU237_SCR_helper_result_is_success;
	typeLPU237_SCR_helper_SetParameters				m_FunLPU237_SCR_helper_SetParameters;
	typeLPU237_SCR_helper_GetATRParameterString	m_FunLPU237_SCR_helper_GetATRParameterString;
	
private:
	//don't use these methods
	CLpu237Dll( const CLpu237Dll & );
	CLpu237Dll & operator=( const CLpu237Dll & );
};

