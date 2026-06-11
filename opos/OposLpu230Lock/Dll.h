#pragma once

#include <string>
#include <Windows.h>
#include "TTR_tchar.h"

using namespace std;

class CDll
{
public:
	// new exported member.
	typedef	void (WINAPI *type_key_callback)(void*);

	typedef	DWORD (WINAPI *typeLPU237Lock_get_list)( LPTSTR );
	typedef	HANDLE (WINAPI *typeLPU237Lock_open)( LPCTSTR  );

	typedef	DWORD (WINAPI *typeLPU237Lock_close)( HANDLE  );
	typedef	DWORD (WINAPI *typeLPU237Lock_enable)( HANDLE  );
	typedef	DWORD (WINAPI *typeLPU237Lock_disable)( HANDLE  );
	typedef	DWORD (WINAPI *typeLPU237Lock_cancel_wait_key)( HANDLE  );
	typedef	DWORD (WINAPI *typeLPU237Lock_wait_key_with_callback)( HANDLE , type_key_callback, void*  );
	typedef	DWORD (WINAPI *typeLPU237Lock_get_data)( DWORD, BYTE* );
	typedef	DWORD (WINAPI *typeLPU237Lock_dll_on)();
	typedef	DWORD (WINAPI *typeLPU237Lock_dll_off)();
	typedef	DWORD (WINAPI *typeLPU237Lock_get_id)( HANDLE hDev, BYTE *sId );
	typedef	DWORD(WINAPI* typeLPU237Lock_get_data_last_error)(unsigned long);

	enum{
		dll_result_success = 0,
		dll_result_error = 0xFFFFFFFF
	};

	enum : unsigned long {
		dll_get_data_error_success = 0,
		dll_get_data_error_invalid_item_index = 1,
		dll_get_data_error_none_device_client = 2,
		dll_get_data_error_none_device_client_result_object = 3,
		dll_get_data_error_get_result_failed_none_response_data_field = 4,
		dll_get_data_error_get_result_failed_cancel_string = 5,
		dll_get_data_error_get_result_failed_error_string = 6,
		dll_get_data_error_get_result_failed_any_string = 7,
		dll_get_data_error_get_result_success_with_less_then_3_plus_8_bytes_data = 8,
		dll_get_data_error_get_result_success_but_result_code_is_cancel = 9,
		dll_get_data_error_get_result_success_but_result_code_is_error = 10
	};

public:
	static CDll & get_instance( const _tstring & s_dll_file_name = _tstring(_T("")) )
	{
		static CDll dll;

		if( !s_dll_file_name.empty() && !dll.is_load_ok() ){
			dll.Load( s_dll_file_name.c_str() );
		}

		return dll;
	}


	~CDll(void)
	{
		if( m_hMode ){
			FreeLibrary( m_hMode );
			m_hMode = 0;
			m_bLoadOk = false;
		}
	}

	bool Load( LPCTSTR sDll )
	{
		bool b_result(false);

		do{
			if( sDll == NULL )
				continue;
			//
			m_hMode = ::LoadLibrary( sDll );
			if( m_hMode == NULL )
				continue;

			//
			// load new exported functions.
			m_get_list =						reinterpret_cast<typeLPU237Lock_get_list>( ::GetProcAddress( m_hMode, "LPU237Lock_get_list" ) );
			m_open =						reinterpret_cast<typeLPU237Lock_open>( ::GetProcAddress( m_hMode, "LPU237Lock_open" ) );						
			m_close =						reinterpret_cast<typeLPU237Lock_close>( ::GetProcAddress( m_hMode, "LPU237Lock_close" ) );
			m_enable =						reinterpret_cast<typeLPU237Lock_enable>( ::GetProcAddress( m_hMode, "LPU237Lock_enable" ) );
			m_disable =						reinterpret_cast<typeLPU237Lock_disable>( ::GetProcAddress( m_hMode, "LPU237Lock_disable" ) );
			m_cancel_wait_key =			reinterpret_cast<typeLPU237Lock_cancel_wait_key>( ::GetProcAddress( m_hMode, "LPU237Lock_cancel_wait_key" ) );
			m_wait_key_with_callback =	reinterpret_cast<typeLPU237Lock_wait_key_with_callback>( ::GetProcAddress( m_hMode, "LPU237Lock_wait_key_with_callback" ) );
			m_get_data =					reinterpret_cast<typeLPU237Lock_get_data>( ::GetProcAddress( m_hMode, "LPU237Lock_get_data" ) );
			m_dll_on =						reinterpret_cast<typeLPU237Lock_dll_on>( ::GetProcAddress( m_hMode, "LPU237Lock_dll_on" ) );
			m_dll_off =						reinterpret_cast<typeLPU237Lock_dll_off>( ::GetProcAddress( m_hMode, "LPU237Lock_dll_off" ) );
			m_get_id =						reinterpret_cast<typeLPU237Lock_get_id>( ::GetProcAddress( m_hMode, "LPU237Lock_get_id" ) );
			m_get_data_last_error = reinterpret_cast<typeLPU237Lock_get_data_last_error>(::GetProcAddress(m_hMode, "LPU237Lock_get_data_last_error"));

			if( m_get_list == NULL )
				continue;
			if( m_open == NULL )
				continue;
			if( m_close == NULL )
				continue;
			if( m_enable == NULL )
				continue;
			if( m_disable == NULL )
				continue;
			if( m_cancel_wait_key == NULL )
				continue;
			if( m_wait_key_with_callback == NULL )
				continue;
			if( m_get_data == NULL )
				continue;
			if( m_dll_on == NULL )
				continue;
			if( m_dll_off == NULL )
				continue;
			if( m_get_id == NULL )
				continue;
			if(m_get_data_last_error == NULL )
				continue;
			//
			m_bLoadOk = true;
			b_result = true;
		}while(0);

		return b_result;
	}

	bool is_load_ok()
	{
		return m_bLoadOk;
	}
	//
	DWORD get_list( LPTSTR sMultiPaths )
	{
		if( m_get_list )	return m_get_list( sMultiPaths );
		else	return 0;
	}

	HANDLE open( LPCTSTR sPath )
	{
		if( m_open )	return m_open( sPath );
		else	return INVALID_HANDLE_VALUE ;
	}

	DWORD close( HANDLE  hDev )
	{
		if( m_close )	return m_close( hDev );
		else	return dll_result_error;
	}

	DWORD enable( HANDLE  hDev )
	{
		if( m_enable )	return m_enable( hDev );
		else return dll_result_error;
	}

	DWORD disable( HANDLE  hDev )
	{
		if( m_disable )	return m_disable( hDev );
		else	return dll_result_error;
	}

	DWORD cancel_wait_key( HANDLE  hDev )
	{
		if( m_cancel_wait_key )	return m_cancel_wait_key( hDev );
		else return dll_result_error;
	}
	
	DWORD wait_key_with_callback( HANDLE  hDev, type_key_callback pFun, void *pParameter )
	{
		if( m_wait_key_with_callback )	return m_wait_key_with_callback( hDev, pFun, pParameter );
		else	return dll_result_error;
	}

	DWORD get_data( DWORD dwBufferIndex,  BYTE *sTrackData )
	{
		if( m_get_data )	return m_get_data( dwBufferIndex, sTrackData );
		else	return dll_result_error;
	}

	DWORD dll_on()
	{
		if( m_dll_on )		return m_dll_on();
		else	return dll_result_error;
	}

	DWORD dll_off()
	{
		if( m_dll_off )		return m_dll_off();
		else	return dll_result_error;
	}

	DWORD get_id( HANDLE hDev, BYTE *sId )
	{
		if( m_get_id )		return m_get_id( hDev, sId );
		else	return dll_result_error;
	}

	DWORD get_data_last_error(unsigned long dwRFU)
	{
		if (m_get_data_last_error) return m_get_data_last_error(dwRFU);
		else return dll_result_error;
	}
private:
	CDll(void) 	  
	{
		ini();
	}

	CDll( LPCTSTR sDll )
	{
		ini();
		Load( sDll );
	}

	void ini()
	{
		m_bLoadOk = false;
		m_hMode = NULL;
		//
		m_get_list = NULL;
		m_open = NULL;
		m_close = NULL;
		m_enable = NULL;
		m_disable = NULL;
		m_cancel_wait_key = NULL;
		m_wait_key_with_callback = NULL;
		m_get_data = NULL;
		m_dll_on = NULL;
		m_dll_off = NULL;
		m_get_id = NULL;
		m_get_data_last_error = NULL;
	}
private:
	HMODULE m_hMode;
	_tstring m_sFullPathName;
	
	bool m_bLoadOk;

	// new exported member.
	typeLPU237Lock_get_list					m_get_list;
	typeLPU237Lock_open						m_open;
	typeLPU237Lock_close						m_close;
	typeLPU237Lock_enable					m_enable;
	typeLPU237Lock_disable					m_disable;
	typeLPU237Lock_cancel_wait_key			m_cancel_wait_key;
	typeLPU237Lock_wait_key_with_callback	m_wait_key_with_callback;
	typeLPU237Lock_get_data					m_get_data;
	typeLPU237Lock_dll_on						m_dll_on;
	typeLPU237Lock_dll_off						m_dll_off;
	typeLPU237Lock_get_id						m_get_id;
	typeLPU237Lock_get_data_last_error 	m_get_data_last_error;
private:
	//don't call these methods
	CDll( const CDll & );
	CDll & operator=( const CDll & );

};

