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

	const _tstring & get_msr_dll_path(){	return m_s_dll_msr;	}
	void set_msr_dll_path( const _tstring & s_dll_ibutton ){	m_s_dll_msr = s_dll_ibutton; }

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

private:
	CGlobalVar(void);
	CGlobalVar( const CGlobalVar & );

private:
	HINSTANCE m_hInstance;
	_tstring m_sWorkPath;
	_tstring m_s_dll_msr;

	CIniFile m_IniFile; 

	bool m_ctl_enable_ertcard_ms_error;
	bool m_ctl_enable_erttrack_ms_error;
	bool m_ctl_enable_commuicate_error;
};

