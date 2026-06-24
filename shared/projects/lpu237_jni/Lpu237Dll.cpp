#include <filesystem>
#include <Lpu237Dll.h>

#ifdef	_WIN32
#include <atltrace.h>
#endif

CLpu237Dll::CLpu237Dll(void)
	: m_hMode(NULL)
{
	_ini();
}

CLpu237Dll::CLpu237Dll( const wchar_t* sDll )
	: m_hMode(NULL)
{
	_ini();
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
		_free_lib( m_hMode );
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

void CLpu237Dll::_ini()
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

bool CLpu237Dll::Load( const wchar_t* sDll )
{
	m_hMode = _load_lib( sDll );

	if (m_hMode == NULL) {
		return false;
	}

	if( m_hMode ){
		m_FunLPU237_get_list = reinterpret_cast<typeLPU237_get_list>(_load_symbol( m_hMode, "LPU237_get_list" ) );
		m_FunLPU237_open = reinterpret_cast<typeLPU237_open>(_load_symbol( m_hMode, "LPU237_open" ) );
		m_FunLPU237_close = reinterpret_cast<typeLPU237_close>( _load_symbol( m_hMode, "LPU237_close" ) );

		m_FunLPU237_enable = reinterpret_cast<typeLPU237_close>( _load_symbol( m_hMode, "LPU237_enable" ) );
		m_FunLPU237_disable = reinterpret_cast<typeLPU237_close>( _load_symbol( m_hMode, "LPU237_disable" ) );
	
		m_FunLPU237_cancel_wait_swipe = reinterpret_cast<typeLPU237_cancel_wait_swipe>( _load_symbol( m_hMode, "LPU237_cancel_wait_swipe" ) );
	
		m_FunLPU237_wait_swipe_with_waits = reinterpret_cast<typeLPU237_wait_swipe_with_waits>( _load_symbol( m_hMode, "LPU237_wait_swipe_with_waits" ) );
		m_FunLPU237_wait_swipe_with_callback = reinterpret_cast<typeLPU237_wait_swipe_with_callback>( _load_symbol( m_hMode, "LPU237_wait_swipe_with_callback" ) );
		m_FunLPU237_wait_swipe_with_message = reinterpret_cast<typeLPU237_wait_swipe_with_message>( _load_symbol( m_hMode, "LPU237_wait_swipe_with_message" ) );
	
		m_FunLPU237_get_data = reinterpret_cast<typeLPU237_get_data>( _load_symbol( m_hMode, "LPU237_get_data" ) );

		m_FunLPU237_dll_on = reinterpret_cast<typeLPU237_dll_on>( _load_symbol( m_hMode, "LPU237_dll_on" ) );
		m_FunLPU237_dll_off = reinterpret_cast<typeLPU237_dll_off>( _load_symbol( m_hMode, "LPU237_dll_off" ) );

		m_FunLPU237_get_id = reinterpret_cast<typeLPU237_get_id>( _load_symbol( m_hMode, "LPU237_get_id" ) );

		m_FunLPU237_SCR_bypass_IccPowerOn = reinterpret_cast<typeLPU237_SCR_bypass_IccPowerOn>( _load_symbol( m_hMode, "LPU237_SCR_bypass_IccPowerOn" ) );
		m_FunLPU237_SCR_bypass_IccPowerOff = reinterpret_cast<typeLPU237_SCR_bypass_IccPowerOff>( _load_symbol( m_hMode, "LPU237_SCR_bypass_IccPowerOff" ) );
		m_FunLPU237_SCR_bypass_XfrBlock = reinterpret_cast<typeLPU237_SCR_bypass_XfrBlock>( _load_symbol( m_hMode, "LPU237_SCR_bypass_XfrBlock" ) );
		m_FunLPU237_SCR_bypass_GetParameters = reinterpret_cast<typeLPU237_SCR_bypass_GetParameters>( _load_symbol( m_hMode, "LPU237_SCR_bypass_GetParameters" ) );
		m_FunLPU237_SCR_bypass_SetParameters = reinterpret_cast<typeLPU237_SCR_bypass_SetParameters>( _load_symbol( m_hMode, "LPU237_SCR_bypass_SetParameters" ) );
		m_FunLPU237_SCR_bypass_ResetParameters = reinterpret_cast<typeLPU237_SCR_bypass_ResetParameters>( _load_symbol( m_hMode, "LPU237_SCR_bypass_ResetParameters" ) );
		m_FunLPU237_SCR_bypass_GetSlotStatus = reinterpret_cast<typeLPU237_SCR_bypass_GetSlotStatus>( _load_symbol( m_hMode, "LPU237_SCR_bypass_GetSlotStatus" ) );
		m_FunLPU237_SCR_bypass_Escape = reinterpret_cast<typeLPU237_SCR_bypass_Escape>( _load_symbol( m_hMode, "LPU237_SCR_bypass_Escape" ) );

		m_FunLPU237_SCR_helper_GetLastError = reinterpret_cast<typeLPU237_SCR_helper_GetLastError>( _load_symbol( m_hMode, "LPU237_SCR_helper_GetLastError" ) );
		m_FunLPU237_SCR_helper_GetFirmwareVersion = reinterpret_cast<typeLPU237_SCR_helper_GetFirmwareVersion>( _load_symbol( m_hMode, "LPU237_SCR_helper_GetFirmwareVersion" ) );
		m_FunLPU237_SCR_helper_GetReaderMode = reinterpret_cast<typeLPU237_SCR_helper_GetReaderMode>( _load_symbol( m_hMode, "LPU237_SCR_helper_GetReaderMode" ) );
		m_FunLPU237_SCR_helper_SetReaderMode = reinterpret_cast<typeLPU237_SCR_helper_SetReaderMode>( _load_symbol( m_hMode, "LPU237_SCR_helper_SetReaderMode" ) );
		m_FunLPU237_SCR_helper_XfrBlock = reinterpret_cast<typeLPU237_SCR_helper_XfrBlock>( _load_symbol( m_hMode, "LPU237_SCR_helper_XfrBlock" ) );
		m_FunLPU237_SCR_helper_SetCardParameters = reinterpret_cast<typeLPU237_SCR_helper_SetCardParameters>( _load_symbol( m_hMode, "LPU237_SCR_helper_SetCardParameters" ) );
		m_FunLPU237_SCR_helper_ResetCardParameters = reinterpret_cast<typeLPU237_SCR_helper_ResetCardParameters>( _load_symbol( m_hMode, "LPU237_SCR_helper_ResetCardParameters" ) );

		m_FunLPU237_SCR_helper_result_is_success = reinterpret_cast<typeLPU237_SCR_helper_result_is_success>( _load_symbol( m_hMode, "LPU237_SCR_helper_result_is_success" ) );
		m_FunLPU237_SCR_helper_SetParameters = reinterpret_cast<typeLPU237_SCR_helper_SetParameters>( _load_symbol( m_hMode, "LPU237_SCR_helper_SetParameters" ) );
		m_FunLPU237_SCR_helper_GetATRParameterString = reinterpret_cast<typeLPU237_SCR_helper_GetATRParameterString>( _load_symbol( m_hMode, "LPU237_SCR_helper_GetATRParameterString_w" ) );

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		if( m_FunLPU237_dll_on )
			m_FunLPU237_dll_on();

		return true;
	}
	else
		return false;
}