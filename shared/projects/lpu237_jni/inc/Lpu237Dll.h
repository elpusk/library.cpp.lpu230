#pragma once

#include <tg_lpu237_dll.h>

class CLpu237Dll
{
private:
	typedef	DWORD (WINAPI *typeLPU237_get_list)( LPTSTR );
	typedef	HANDLE (WINAPI *typeLPU237_open)( LPCTSTR  );

	typedef	DWORD (WINAPI *typeLPU237_close)( HANDLE  );
	typedef	DWORD (WINAPI *typeLPU237_enable)( HANDLE  );
	typedef	DWORD (WINAPI *typeLPU237_disable)( HANDLE  );
	typedef	DWORD (WINAPI *typeLPU237_cancel_wait_swipe)( HANDLE  );
	typedef	DWORD (WINAPI *typeLPU237_wait_swipe_with_waits)( HANDLE  );
	typedef	DWORD (WINAPI *typeLPU237_wait_swipe_with_callback)( HANDLE , type_callback, void*  );
	typedef	DWORD (WINAPI *typeLPU237_wait_swipe_with_message)( HANDLE, HWND , UINT );
	typedef	DWORD (WINAPI *typeLPU237_get_data)( DWORD, DWORD, BYTE* );
	typedef	DWORD (WINAPI *typeLPU237_dll_on)();
	typedef	DWORD (WINAPI *typeLPU237_dll_off)();
	typedef	DWORD (WINAPI *typeLPU237_get_id)( HANDLE hDev, BYTE *sId );

	// SCR functions
	typedef	DWORD (WINAPI *typeLPU237_SCR_bypass_IccPowerOn)( HANDLE hDev, BYTE cPower, BYTE *sRx,	LPDWORD lpnRx );
	typedef	DWORD (WINAPI *typeLPU237_SCR_bypass_IccPowerOff)( HANDLE hDev );
	typedef	DWORD (WINAPI *typeLPU237_SCR_bypass_XfrBlock)( HANDLE hDev, BYTE cBWI,CONST BYTE *sTx,	DWORD nTx,	BYTE *sRx,LPDWORD lpnRx);
	typedef	DWORD (WINAPI *typeLPU237_SCR_bypass_GetParameters)( HANDLE hDev, BYTE *sRx, LPDWORD lpnRx  );
	typedef	DWORD (WINAPI *typeLPU237_SCR_bypass_SetParameters)( HANDLE hDev,BYTE cProtocol,BYTE bmFindexDindex,	BYTE bmTCCKST,BYTE bGuardTime,BYTE bWaitingInteger,BYTE bIFSC );
	typedef	DWORD (WINAPI *typeLPU237_SCR_bypass_ResetParameters)( HANDLE hDev );
	typedef	DWORD (WINAPI *typeLPU237_SCR_bypass_GetSlotStatus)( HANDLE hDev );
	typedef	DWORD (WINAPI *typeLPU237_SCR_bypass_Escape)( HANDLE hDev,CONST BYTE *sTx,	DWORD nTx,	BYTE *sRx,LPDWORD lpnRx);

	//SCR helper function
	typedef	DWORD (WINAPI *typeLPU237_SCR_helper_GetLastError)();
	typedef	BOOL (WINAPI *typeLPU237_SCR_helper_GetFirmwareVersion)( HANDLE hDev, BYTE *sRx, LPDWORD lpnRx );
	typedef	BOOL (WINAPI *typeLPU237_SCR_helper_GetReaderMode)( HANDLE hDev, BYTE *lpcMode  );
	typedef	BOOL (WINAPI *typeLPU237_SCR_helper_SetReaderMode)( HANDLE hDev, BYTE cMode, BYTE cPower );
	typedef	BOOL (WINAPI *typeLPU237_SCR_helper_XfrBlock)( HANDLE hDev,CONST BYTE *sTx,DWORD nTx,BYTE *sRx,LPDWORD lpnRx);
	typedef	BOOL (WINAPI *typeLPU237_SCR_helper_SetCardParameters)( HANDLE hDev, BYTE cProtocol, BYTE cParameter, BYTE cNewValue );
	typedef	BOOL (WINAPI *typeLPU237_SCR_helper_ResetCardParameters)( HANDLE hDev );
	typedef	BOOL (WINAPI *typeLPU237_SCR_helper_result_is_success)( DWORD dwScrResult );
	typedef	BOOL (WINAPI *typeLPU237_SCR_helper_SetParameters)( HANDLE hDev ,CONST BYTE *sAtr, DWORD nAtr );
	typedef	BOOL (WINAPI *typeLPU237_SCR_helper_GetATRParameterString)( TCHAR *sOut, LPDWORD lpnOut, CONST BYTE *sAtr,	DWORD nAtr, TCHAR cDelimiter );

private:
	CLpu237Dll(void);
	CLpu237Dll( LPCTSTR sDll );
public:
	static CLpu237Dll *get_instance(LPCTSTR sDll = NULL)
	{
		static CLpu237Dll dll;

		if( sDll )
			dll.Load( sDll );

		return &dll;
	}

	virtual ~CLpu237Dll(void);

	bool Load( LPCTSTR sDll );

	void Unload();
	//
	DWORD LPU237_get_list( LPTSTR sMultiPaths )
	{
		if( m_FunLPU237_get_list )	return m_FunLPU237_get_list( sMultiPaths );
		else								return 0;
	}

	HANDLE LPU237_open( LPCTSTR sPath )
	{
		if( m_FunLPU237_open )		return m_FunLPU237_open( sPath );
		else								return INVALID_HANDLE_VALUE ;
	}

	DWORD LPU237_close( HANDLE  hDev )
	{
		if( m_FunLPU237_close )	return m_FunLPU237_close( hDev );
		else							return LPU237_DLL_RESULT_ERROR;
	}

	DWORD LPU237_enable( HANDLE  hDev )
	{
		if( m_FunLPU237_enable )	return m_FunLPU237_enable( hDev );
		else								return LPU237_DLL_RESULT_ERROR;
	}

	DWORD LPU237_disable( HANDLE  hDev )
	{
		if( m_FunLPU237_disable )	return m_FunLPU237_disable( hDev );
		else								return LPU237_DLL_RESULT_ERROR;
	}

	DWORD LPU237_cancel_wait_swipe( HANDLE  hDev )
	{
		if( m_FunLPU237_cancel_wait_swipe )	return m_FunLPU237_cancel_wait_swipe( hDev );
		else											return LPU237_DLL_RESULT_ERROR;
	}

	DWORD LPU237_wait_swipe_with_waits( HANDLE  hDev )
	{
		if( m_FunLPU237_wait_swipe_with_waits )	return m_FunLPU237_wait_swipe_with_waits( hDev );
		else													return LPU237_DLL_RESULT_ERROR;
	}

	DWORD LPU237_wait_swipe_with_callback( HANDLE  hDev, type_callback pFun, void *pParameter )
	{
		if( m_FunLPU237_wait_swipe_with_callback )	return m_FunLPU237_wait_swipe_with_callback( hDev, pFun, pParameter );
		else														return LPU237_DLL_RESULT_ERROR;
	}

	DWORD LPU237_wait_swipe_with_message( HANDLE  hDev, HWND hWnd, UINT nMsg )
	{
		if( m_FunLPU237_wait_swipe_with_message )	return m_FunLPU237_wait_swipe_with_message( hDev, hWnd, nMsg );
		else														return LPU237_DLL_RESULT_ERROR;
	}

	DWORD LPU237_get_data( DWORD dwBufferIndex, DWORD dwIsoTrack, BYTE *sTrackData )
	{
		if( m_FunLPU237_get_data )	return m_FunLPU237_get_data( dwBufferIndex, dwIsoTrack, sTrackData );
		else								return LPU237_DLL_RESULT_ERROR;
	}

	DWORD LPU237_dll_on()
	{
		if( m_FunLPU237_dll_on )	return m_FunLPU237_dll_on();
		else							return LPU237_DLL_RESULT_ERROR;
	}

	DWORD LPU237_dll_off()
	{
		if( m_FunLPU237_dll_off )	return m_FunLPU237_dll_off();
		else							return LPU237_DLL_RESULT_ERROR;
	}

	DWORD LPU237_get_id( HANDLE hDev, BYTE *sId )
	{
		if( m_FunLPU237_get_id )	return m_FunLPU237_get_id( hDev, sId );
		else							return LPU237_DLL_RESULT_ERROR;
	}

	DWORD LPU237_SCR_bypass_IccPowerOn( HANDLE hDev, BYTE cPower, BYTE *sRx,	LPDWORD lpnRx )
	{
		if( m_FunLPU237_SCR_bypass_IccPowerOn )	return m_FunLPU237_SCR_bypass_IccPowerOn( hDev, cPower, sRx, lpnRx );
		else							return LPU237_DLL_SCR_RESULT_STATUS_CMD_FAIL | LPU237_DLL_SCR_RESULT_STATUS_ICC_UNKNOWN;
	}

	DWORD LPU237_SCR_bypass_IccPowerOff( HANDLE hDev )
	{
		if( m_FunLPU237_SCR_bypass_IccPowerOff )	return m_FunLPU237_SCR_bypass_IccPowerOff( hDev );
		else							return LPU237_DLL_SCR_RESULT_STATUS_CMD_FAIL | LPU237_DLL_SCR_RESULT_STATUS_ICC_UNKNOWN;
	}

	DWORD LPU237_SCR_bypass_XfrBlock( HANDLE hDev, BYTE cBWI,CONST BYTE *sTx,DWORD nTx,BYTE *sRx,LPDWORD lpnRx)
	{
		if( m_FunLPU237_SCR_bypass_XfrBlock )	return m_FunLPU237_SCR_bypass_XfrBlock( hDev,cBWI,sTx,nTx, sRx,lpnRx );
		else							return LPU237_DLL_SCR_RESULT_STATUS_CMD_FAIL | LPU237_DLL_SCR_RESULT_STATUS_ICC_UNKNOWN;
	}

	DWORD LPU237_SCR_bypass_GetParameters( HANDLE hDev, BYTE *sRx, LPDWORD lpnRx  )
	{
		if( m_FunLPU237_SCR_bypass_GetParameters )	return m_FunLPU237_SCR_bypass_GetParameters( hDev,sRx,lpnRx );
		else							return LPU237_DLL_SCR_RESULT_STATUS_CMD_FAIL | LPU237_DLL_SCR_RESULT_STATUS_ICC_UNKNOWN;
	}

	DWORD LPU237_SCR_bypass_SetParameters( HANDLE hDev,BYTE cProtocol,BYTE bmFindexDindex,	BYTE bmTCCKST,BYTE bGuardTime,BYTE bWaitingInteger,BYTE bIFSC )
	{
		if( m_FunLPU237_SCR_bypass_SetParameters )	return m_FunLPU237_SCR_bypass_SetParameters( hDev,cProtocol, bmFindexDindex,bmTCCKST,bGuardTime, bWaitingInteger, bIFSC   );
		else							return LPU237_DLL_SCR_RESULT_STATUS_CMD_FAIL | LPU237_DLL_SCR_RESULT_STATUS_ICC_UNKNOWN;
	}

	DWORD LPU237_SCR_bypass_ResetParameters( HANDLE hDev )
	{
		if( m_FunLPU237_SCR_bypass_ResetParameters )	return m_FunLPU237_SCR_bypass_ResetParameters( hDev );
		else							return LPU237_DLL_SCR_RESULT_STATUS_CMD_FAIL | LPU237_DLL_SCR_RESULT_STATUS_ICC_UNKNOWN;
	}

	DWORD LPU237_SCR_bypass_GetSlotStatus( HANDLE hDev )
	{
		if( m_FunLPU237_SCR_bypass_GetSlotStatus )	return m_FunLPU237_SCR_bypass_GetSlotStatus( hDev );
		else							return LPU237_DLL_SCR_RESULT_STATUS_CMD_FAIL | LPU237_DLL_SCR_RESULT_STATUS_ICC_UNKNOWN;
	}

	DWORD LPU237_SCR_bypass_Escape( HANDLE hDev,CONST BYTE *sTx,DWORD nTx,BYTE *sRx,LPDWORD lpnRx)
	{
		if( m_FunLPU237_SCR_bypass_Escape )	return m_FunLPU237_SCR_bypass_Escape( hDev,sTx, nTx, sRx, lpnRx );
		else							return LPU237_DLL_SCR_RESULT_STATUS_CMD_FAIL | LPU237_DLL_SCR_RESULT_STATUS_ICC_UNKNOWN;
	}

	DWORD LPU237_SCR_helper_GetLastError()
	{
		if( m_FunLPU237_SCR_helper_GetLastError )	return m_FunLPU237_SCR_helper_GetLastError();
		else							return LPU237_DLL_SCR_RESULT_STATUS_CMD_FAIL | LPU237_DLL_SCR_RESULT_STATUS_ICC_UNKNOWN;
	}

	BOOL LPU237_SCR_helper_GetFirmwareVersion( HANDLE hDev, BYTE *sRx, LPDWORD lpnRx )	
	{
		if( m_FunLPU237_SCR_helper_GetFirmwareVersion )	return m_FunLPU237_SCR_helper_GetFirmwareVersion( hDev, sRx, lpnRx );
		else							return FALSE;
	}

	BOOL LPU237_SCR_helper_GetReaderMode( HANDLE hDev, BYTE *lpcMode  )
	{
		if( m_FunLPU237_SCR_helper_GetReaderMode )	return m_FunLPU237_SCR_helper_GetReaderMode( hDev, lpcMode );
		else							return FALSE;
	}

	BOOL LPU237_SCR_helper_SetReaderMode( HANDLE hDev, BYTE cMode, BYTE cPower )
	{
		if( m_FunLPU237_SCR_helper_SetReaderMode )	return m_FunLPU237_SCR_helper_SetReaderMode( hDev, cMode,cPower );
		else							return FALSE;
	}

	BOOL LPU237_SCR_helper_XfrBlock( HANDLE hDev,CONST BYTE *sTx,	DWORD nTx,BYTE *sRx,LPDWORD lpnRx)
	{
		if( m_FunLPU237_SCR_helper_XfrBlock )	return m_FunLPU237_SCR_helper_XfrBlock( hDev, sTx, nTx, sRx, lpnRx );
		else							return FALSE;
	}

	BOOL LPU237_SCR_helper_SetCardParameters( HANDLE hDev, BYTE cProtocol, BYTE cParameter, BYTE cNewValue )
	{
		if( m_FunLPU237_SCR_helper_SetCardParameters )	return m_FunLPU237_SCR_helper_SetCardParameters( hDev, cProtocol, cParameter, cNewValue );
		else							return FALSE;
	}

	BOOL LPU237_SCR_helper_ResetCardParameters( HANDLE hDev )
	{
		if( m_FunLPU237_SCR_helper_ResetCardParameters )	return m_FunLPU237_SCR_helper_ResetCardParameters( hDev );
		else							return FALSE;
	}

	BOOL LPU237_SCR_helper_result_is_success( DWORD dwScrResult )
	{
		if( m_FunLPU237_SCR_helper_result_is_success )	return m_FunLPU237_SCR_helper_result_is_success( dwScrResult );
		else							return FALSE;
	}

	BOOL LPU237_SCR_helper_SetParameters( HANDLE hDev ,CONST BYTE *sAtr, DWORD nAtr )
	{
		if( m_FunLPU237_SCR_helper_SetParameters )	return m_FunLPU237_SCR_helper_SetParameters( hDev, sAtr, nAtr );
		else							return FALSE;
	}

	BOOL LPU237_SCR_helper_GetATRParameterString( TCHAR *sOut, LPDWORD lpnOut, CONST BYTE *sAtr,DWORD nAtr, TCHAR cDelimiter )
	{
		if( m_FunLPU237_SCR_helper_GetATRParameterString )	return m_FunLPU237_SCR_helper_GetATRParameterString( sOut, lpnOut, sAtr, nAtr,cDelimiter );
		else							return FALSE;
	}

private:
	void ini();
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

