#include "Lpu237Dll.h"

#ifdef	_WIN32
#include <atltrace.h>
#endif

CLpu237Dll::CLpu237Dll(void)
	: m_hMode(NULL)
{
	ini();
}

CLpu237Dll::CLpu237Dll( LPCTSTR sDll )
	: m_hMode(NULL)
{
	ini();
	Load( sDll );
}

CLpu237Dll::~CLpu237Dll(void)
{
	if( m_hMode )
		Unload();
}

void CLpu237Dll::Unload()
{
	if( m_hMode ){
		if( m_FunLPU237_dll_off )
			m_FunLPU237_dll_off();
		//
		FreeLibrary( m_hMode );
		m_hMode = NULL;
		//
		m_FunLPU237_get_list = NULL;
		m_FunLPU237_open = NULL;
		m_FunLPU237_close = NULL;
		m_FunLPU237_enable = NULL;
		m_FunLPU237_disable = NULL;
		m_FunLPU237_cancel_wait_swipe = NULL;
		m_FunLPU237_wait_swipe_with_waits = NULL;
		m_FunLPU237_wait_swipe_with_callback = NULL;
		m_FunLPU237_wait_swipe_with_message = NULL;
		m_FunLPU237_get_data = NULL;
		m_FunLPU237_dll_on = NULL;
		m_FunLPU237_dll_off = NULL;
		m_FunLPU237_get_id = NULL;

		m_FunLPU237_SCR_bypass_IccPowerOn = NULL;
		m_FunLPU237_SCR_bypass_IccPowerOff = NULL;
		m_FunLPU237_SCR_bypass_XfrBlock = NULL;
		m_FunLPU237_SCR_bypass_GetParameters = NULL;
		m_FunLPU237_SCR_bypass_SetParameters = NULL;
		m_FunLPU237_SCR_bypass_ResetParameters = NULL;
		m_FunLPU237_SCR_bypass_GetSlotStatus = NULL;
		m_FunLPU237_SCR_bypass_Escape = NULL;

		m_FunLPU237_SCR_helper_GetLastError = NULL;
		m_FunLPU237_SCR_helper_GetFirmwareVersion = NULL;
		m_FunLPU237_SCR_helper_GetReaderMode = NULL;
		m_FunLPU237_SCR_helper_SetReaderMode = NULL;
		m_FunLPU237_SCR_helper_XfrBlock = NULL;
		m_FunLPU237_SCR_helper_SetCardParameters = NULL;
		m_FunLPU237_SCR_helper_ResetCardParameters = NULL;
		m_FunLPU237_SCR_helper_result_is_success = NULL;
		m_FunLPU237_SCR_helper_SetParameters = NULL;
		m_FunLPU237_SCR_helper_GetATRParameterString = NULL;
	}
}

void CLpu237Dll::ini()
{
	m_FunLPU237_get_list = NULL;
	m_FunLPU237_open = NULL;
	m_FunLPU237_close = NULL;
	m_FunLPU237_enable = NULL;
	m_FunLPU237_disable = NULL;
	m_FunLPU237_cancel_wait_swipe = NULL;
	m_FunLPU237_wait_swipe_with_waits = NULL;
	m_FunLPU237_wait_swipe_with_callback = NULL;
	m_FunLPU237_wait_swipe_with_message = NULL;
	m_FunLPU237_get_data = NULL;
	m_FunLPU237_dll_on = NULL;
	m_FunLPU237_dll_off = NULL;
	m_FunLPU237_get_id = NULL;

	m_FunLPU237_SCR_bypass_IccPowerOn = NULL;
	m_FunLPU237_SCR_bypass_IccPowerOff = NULL;
	m_FunLPU237_SCR_bypass_XfrBlock = NULL;
	m_FunLPU237_SCR_bypass_GetParameters = NULL;
	m_FunLPU237_SCR_bypass_SetParameters = NULL;
	m_FunLPU237_SCR_bypass_ResetParameters = NULL;
	m_FunLPU237_SCR_bypass_GetSlotStatus = NULL;
	m_FunLPU237_SCR_bypass_Escape = NULL;

	m_FunLPU237_SCR_helper_GetLastError = NULL;
	m_FunLPU237_SCR_helper_GetFirmwareVersion = NULL;
	m_FunLPU237_SCR_helper_GetReaderMode = NULL;
	m_FunLPU237_SCR_helper_SetReaderMode = NULL;
	m_FunLPU237_SCR_helper_XfrBlock = NULL;
	m_FunLPU237_SCR_helper_SetCardParameters = NULL;
	m_FunLPU237_SCR_helper_ResetCardParameters = NULL;
	m_FunLPU237_SCR_helper_result_is_success = NULL;
	m_FunLPU237_SCR_helper_SetParameters = NULL;
	m_FunLPU237_SCR_helper_GetATRParameterString = NULL;
}

bool CLpu237Dll::Load( LPCTSTR sDll )
{
	m_hMode = ::LoadLibrary( sDll );

	if( m_hMode == NULL )
		m_hMode = ::LoadLibrary( _T(".\\tg_lpu237_dll.dll") );

	if( m_hMode ){
#ifdef  UNICODE  
		m_FunLPU237_get_list = reinterpret_cast<typeLPU237_get_list>( ::GetProcAddress( m_hMode, "LPU237_get_list_w" ) );
		m_FunLPU237_open = reinterpret_cast<typeLPU237_open>( ::GetProcAddress( m_hMode, "LPU237_open_w" ) );
#else
		m_FunLPU237_get_list = reinterpret_cast<typeLPU237_get_list>( ::GetProcAddress( m_hMode, "LPU237_get_list_a" ) );
		m_FunLPU237_open = reinterpret_cast<typeLPU237_open>( ::GetProcAddress( m_hMode, "LPU237_open_a" ) );
#endif
		m_FunLPU237_close = reinterpret_cast<typeLPU237_close>( ::GetProcAddress( m_hMode, "LPU237_close" ) );

		m_FunLPU237_enable = reinterpret_cast<typeLPU237_close>( ::GetProcAddress( m_hMode, "LPU237_enable" ) );
		m_FunLPU237_disable = reinterpret_cast<typeLPU237_close>( ::GetProcAddress( m_hMode, "LPU237_disable" ) );
	
		m_FunLPU237_cancel_wait_swipe = reinterpret_cast<typeLPU237_cancel_wait_swipe>( ::GetProcAddress( m_hMode, "LPU237_cancel_wait_swipe" ) );
	
		m_FunLPU237_wait_swipe_with_waits = reinterpret_cast<typeLPU237_wait_swipe_with_waits>( ::GetProcAddress( m_hMode, "LPU237_wait_swipe_with_waits" ) );
		m_FunLPU237_wait_swipe_with_callback = reinterpret_cast<typeLPU237_wait_swipe_with_callback>( ::GetProcAddress( m_hMode, "LPU237_wait_swipe_with_callback" ) );
		m_FunLPU237_wait_swipe_with_message = reinterpret_cast<typeLPU237_wait_swipe_with_message>( ::GetProcAddress( m_hMode, "LPU237_wait_swipe_with_message" ) );
	
		m_FunLPU237_get_data = reinterpret_cast<typeLPU237_get_data>( ::GetProcAddress( m_hMode, "LPU237_get_data" ) );

		m_FunLPU237_dll_on = reinterpret_cast<typeLPU237_dll_on>( ::GetProcAddress( m_hMode, "LPU237_dll_on" ) );
		m_FunLPU237_dll_off = reinterpret_cast<typeLPU237_dll_off>( ::GetProcAddress( m_hMode, "LPU237_dll_off" ) );

		m_FunLPU237_get_id = reinterpret_cast<typeLPU237_get_id>( ::GetProcAddress( m_hMode, "LPU237_get_id" ) );

		m_FunLPU237_SCR_bypass_IccPowerOn = reinterpret_cast<typeLPU237_SCR_bypass_IccPowerOn>( ::GetProcAddress( m_hMode, "LPU237_SCR_bypass_IccPowerOn" ) );
		m_FunLPU237_SCR_bypass_IccPowerOff = reinterpret_cast<typeLPU237_SCR_bypass_IccPowerOff>( ::GetProcAddress( m_hMode, "LPU237_SCR_bypass_IccPowerOff" ) );
		m_FunLPU237_SCR_bypass_XfrBlock = reinterpret_cast<typeLPU237_SCR_bypass_XfrBlock>( ::GetProcAddress( m_hMode, "LPU237_SCR_bypass_XfrBlock" ) );
		m_FunLPU237_SCR_bypass_GetParameters = reinterpret_cast<typeLPU237_SCR_bypass_GetParameters>( ::GetProcAddress( m_hMode, "LPU237_SCR_bypass_GetParameters" ) );
		m_FunLPU237_SCR_bypass_SetParameters = reinterpret_cast<typeLPU237_SCR_bypass_SetParameters>( ::GetProcAddress( m_hMode, "LPU237_SCR_bypass_SetParameters" ) );
		m_FunLPU237_SCR_bypass_ResetParameters = reinterpret_cast<typeLPU237_SCR_bypass_ResetParameters>( ::GetProcAddress( m_hMode, "LPU237_SCR_bypass_ResetParameters" ) );
		m_FunLPU237_SCR_bypass_GetSlotStatus = reinterpret_cast<typeLPU237_SCR_bypass_GetSlotStatus>( ::GetProcAddress( m_hMode, "LPU237_SCR_bypass_GetSlotStatus" ) );
		m_FunLPU237_SCR_bypass_Escape = reinterpret_cast<typeLPU237_SCR_bypass_Escape>( ::GetProcAddress( m_hMode, "LPU237_SCR_bypass_Escape" ) );

		m_FunLPU237_SCR_helper_GetLastError = reinterpret_cast<typeLPU237_SCR_helper_GetLastError>( ::GetProcAddress( m_hMode, "LPU237_SCR_helper_GetLastError" ) );
		m_FunLPU237_SCR_helper_GetFirmwareVersion = reinterpret_cast<typeLPU237_SCR_helper_GetFirmwareVersion>( ::GetProcAddress( m_hMode, "LPU237_SCR_helper_GetFirmwareVersion" ) );
		m_FunLPU237_SCR_helper_GetReaderMode = reinterpret_cast<typeLPU237_SCR_helper_GetReaderMode>( ::GetProcAddress( m_hMode, "LPU237_SCR_helper_GetReaderMode" ) );
		m_FunLPU237_SCR_helper_SetReaderMode = reinterpret_cast<typeLPU237_SCR_helper_SetReaderMode>( ::GetProcAddress( m_hMode, "LPU237_SCR_helper_SetReaderMode" ) );
		m_FunLPU237_SCR_helper_XfrBlock = reinterpret_cast<typeLPU237_SCR_helper_XfrBlock>( ::GetProcAddress( m_hMode, "LPU237_SCR_helper_XfrBlock" ) );
		m_FunLPU237_SCR_helper_SetCardParameters = reinterpret_cast<typeLPU237_SCR_helper_SetCardParameters>( ::GetProcAddress( m_hMode, "LPU237_SCR_helper_SetCardParameters" ) );
		m_FunLPU237_SCR_helper_ResetCardParameters = reinterpret_cast<typeLPU237_SCR_helper_ResetCardParameters>( ::GetProcAddress( m_hMode, "LPU237_SCR_helper_ResetCardParameters" ) );

		m_FunLPU237_SCR_helper_result_is_success = reinterpret_cast<typeLPU237_SCR_helper_result_is_success>( ::GetProcAddress( m_hMode, "LPU237_SCR_helper_result_is_success" ) );
		m_FunLPU237_SCR_helper_SetParameters = reinterpret_cast<typeLPU237_SCR_helper_SetParameters>( ::GetProcAddress( m_hMode, "LPU237_SCR_helper_SetParameters" ) );
#ifdef  UNICODE  
		m_FunLPU237_SCR_helper_GetATRParameterString = reinterpret_cast<typeLPU237_SCR_helper_GetATRParameterString>( ::GetProcAddress( m_hMode, "LPU237_SCR_helper_GetATRParameterString_w" ) );
#else
		m_FunLPU237_SCR_helper_GetATRParameterString = reinterpret_cast<typeLPU237_SCR_helper_GetATRParameterString>( ::GetProcAddress( m_hMode, "LPU237_SCR_helper_GetATRParameterString_a" ) );
#endif//UNICODE

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		if( m_FunLPU237_dll_on )
			m_FunLPU237_dll_on();

		return true;
	}
	else
		return false;
}