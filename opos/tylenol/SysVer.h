#pragma once

#include "Windows.h"
#include "cmdset.h"

class CSysVer
{
public:
	
	CSysVer();
	CSysVer( const LPBYTE sVer );

	CSysVer( 	UINT32 nMajor, UINT32 nMinor,UINT32 nFix, UINT32 nBuild );

	CSysVer( const CSysVer & Ver );

	virtual ~CSysVer(void);

	CSysVer & operator=( const CSysVer & src );

	bool operator > ( const CSysVer & rhs );

	bool operator == ( const CSysVer & rhs );

	bool operator < ( const CSysVer & rhs );

	//getter
	UINT32 GetMajor(){	return m_nMajor;	}
	UINT32 GetMinor(){	return m_nMinor;	}
	UINT32 GetFix(){	return m_nFix;	}
	UINT32 GetBuild(){	return m_nBuild;	}

protected:
private:
	UINT32 m_nMajor;
	UINT32 m_nMinor;
	UINT32 m_nFix;
	UINT32 m_nBuild;
};

