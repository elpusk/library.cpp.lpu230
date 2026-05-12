#pragma once

#include "TTR_tchar.h"
#include <vector>

using namespace std;

class CUserPara
{
public:
	CUserPara( INT32 argc, _TCHAR* argv[] );
	virtual ~CUserPara(void);

	INT32 GetSize(){		return m_vArg.size();	}
	const _tstring & GetParameter( INT32 nArg );

	virtual void Load(  INT32 argc, _TCHAR* argv[] );

	CUserPara(void);

protected:
	vector<_tstring> m_vArg;
	const static _tstring NullString;
	
};

