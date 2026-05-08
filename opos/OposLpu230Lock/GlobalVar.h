#pragma once

#include <string>
#include "IniFile.h"

using namespace std;

/////////////////////////////
// global variables class

class CGlobalVar
{
public:
	static CGlobalVar *GetInstance();

	~CGlobalVar(void);

	//////////////////////////////////
	//getter & setter
	HINSTANCE getInstanceHandle(){		return m_hInstance;	}
	void setInstanceHandle( HINSTANCE hInstance ){		m_hInstance = hInstance;	}

	const _tstring & getWorkPath();
	void setWorkPath( const _tstring & sWorkPath ){	m_sWorkPath = sWorkPath;	}

	const _tstring & get_ibutton_dll_path(){	return m_s_dll_ibutton;	}
	void set_ibutton_dll_path( const _tstring & s_dll_ibutton ){	m_s_dll_ibutton = s_dll_ibutton; }

	const _tstring & get_ibutton_btc_dll_path(){		return m_s_dll_ibutton_btc;	}
	void set_ibutton_btc_dll_path( const _tstring & s_dll_ibutton_btc ){	m_s_dll_ibutton_btc = s_dll_ibutton_btc; }

	CIniFile & getIniFile(){	return m_IniFile;	}

	bool get_ctl_enable_ertcard_ms_error(){		return m_ctl_enable_ertcard_ms_error;	}
	void set_ctl_enable_ertcard_ms_error( bool b_ctl_enable_ertcard_ms_error )
	{	
		m_ctl_enable_ertcard_ms_error = b_ctl_enable_ertcard_ms_error;
	}
	
	bool get_ctl_enable_erttrack_ms_error(){		return m_ctl_enable_erttrack_ms_error;	}
	void set_ctl_enable_erttrack_ms_error( bool b_ctl_enable_erttrack_ms_error )
	{	
		m_ctl_enable_erttrack_ms_error = b_ctl_enable_erttrack_ms_error;
	}

	bool get_ctl_enable_commuicate_error(){	return m_ctl_enable_commuicate_error;	}
	void set_ctl_enable_commuicate_error( bool b_ctl_enable_commuicate_error )
	{	
		m_ctl_enable_commuicate_error = b_ctl_enable_commuicate_error;
	}

	long get_lock_key_size()
	{
		return m_n_key;
	}

	void set_lock_key_size( long n_key )
	{
		m_n_key = n_key;
	}

	bool is_generate_exception_in_set_binary_conversion() const
	{
		return m_b_generate_exception_in_set_binary_conversion;
	}

	void set_generate_exception_in_set_binary_conversion(bool b_generate_exception)
	{
		m_b_generate_exception_in_set_binary_conversion = b_generate_exception;
	}

private:
	CGlobalVar(void);
	CGlobalVar( const CGlobalVar & );

private:
	HINSTANCE m_hInstance;
	_tstring m_sWorkPath;
	_tstring m_s_dll_ibutton;
	_tstring m_s_dll_ibutton_btc;

	CIniFile m_IniFile; 

	bool m_ctl_enable_ertcard_ms_error;
	bool m_ctl_enable_erttrack_ms_error;
	bool m_ctl_enable_commuicate_error;

	long m_n_key;	//i-button key size
	bool m_b_generate_exception_in_set_binary_conversion;
};

