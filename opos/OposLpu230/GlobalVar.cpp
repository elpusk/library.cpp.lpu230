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
	m_ctl_enable_commuicate_error(true)
{
}


CGlobalVar::~CGlobalVar(void)
{
}
