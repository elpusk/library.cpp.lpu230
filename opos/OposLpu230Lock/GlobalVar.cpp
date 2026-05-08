#include "StdAfx.h"
#include "GlobalVar.h"

CGlobalVar * CGlobalVar::GetInstance()
{
	static CGlobalVar Variables;
	return &Variables;
}


CGlobalVar::CGlobalVar(void) 
	: m_hInstance(NULL),
	m_ctl_enable_ertcard_ms_error(false),
	m_ctl_enable_erttrack_ms_error(true),
	m_ctl_enable_commuicate_error(true),
	m_n_key(8),
	m_b_generate_exception_in_set_binary_conversion(true)//default i sold style
{
}


CGlobalVar::~CGlobalVar(void)
{
}
