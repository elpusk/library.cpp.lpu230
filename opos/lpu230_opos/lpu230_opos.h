
// lpu230_opos.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// Clpu230oposApp:
// See lpu230_opos.cpp for the implementation of this class
//

class Clpu230oposApp : public CWinApp
{
public:
	Clpu230oposApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern Clpu230oposApp theApp;
