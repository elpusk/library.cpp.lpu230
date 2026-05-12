#pragma once
#include "userpara.h"

class CMsrCLPara :	public CUserPara
{
public:
	CMsrCLPara( INT32 argc, _TCHAR* argv[] );
	~CMsrCLPara(void);

	INT32 GetLoopCount(){	return m_nLoop;	}

	void Load(  INT32 argc, _TCHAR* argv[] );

	bool IsGotoBoot(){	return m_bBoot;	}
	bool IsSetRequest(){	return m_bIsSetReq;	}

	const _tstring & GetAppName();
	const _tstring & GetIniFileName(){	return m_FileName;	}

private:
	CMsrCLPara();

	void Display();

private:
	INT32 m_nLoop;
	bool m_bBoot;
	_tstring m_FileName;

	bool m_bIsSetReq;

};

