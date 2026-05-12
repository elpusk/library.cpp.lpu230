#pragma once
#include "loadini.h"


class CLoadMsrIni :
	public CLoadIni
{
public:
	CLoadMsrIni(void);
	virtual ~CLoadMsrIni(void);

	virtual bool Load( const _tstring sInifileName );

private:
};

