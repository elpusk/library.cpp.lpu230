#include "StdAfx.h"
#include "SysVer.h"
#include "info_sys.h"
#include "info_sys_cnst.h"

CSysVer::CSysVer()
{
	m_nMajor = m_nMinor = m_nFix = m_nBuild = 0;
}

CSysVer::CSysVer( const LPBYTE sVer )
{
	try{
		INT32 i=0;

		if( sVer[0]>=0 && sVer[0] <= 9 ){
			// sVer[0].sVer[1].sVer[2].sVer[3] format.
			m_nMajor = static_cast<UINT32>(sVer[i++]);
			m_nMinor = static_cast<UINT32>(sVer[i++]);
			m_nFix = static_cast<UINT32>(sVer[i++]);
			m_nBuild = static_cast<UINT32>(sVer[i++]);
		}
		else{//BTC version format : zero string
			m_nMajor = m_nMinor = m_nFix = m_nBuild = 0;
			UINT32 *pV[] = { &m_nMajor, &m_nMinor, &m_nFix, &m_nBuild };
			int j = 0;
			//
			while( sVer[i] ){

				if( sVer[i] >= '0' && sVer[i] <= '9' ){
					*pV[j] = static_cast<UINT32>( sVer[i]-'0' );
					j++;
				}

				i++;
			}//end while
		}
	}
	catch(...){
		m_nMajor = m_nMinor = m_nFix = m_nBuild = 0;
	}
}

CSysVer::CSysVer( 	UINT32 nMajor, UINT32 nMinor,UINT32 nFix, UINT32 nBuild )
{
	m_nMajor = nMajor;	m_nMinor = nMinor;	m_nFix = nFix;	m_nBuild = nBuild;
}

CSysVer::CSysVer( const CSysVer & Ver )
{
	m_nMajor = Ver.m_nMajor;	m_nMinor = Ver.m_nMinor;	m_nFix = Ver.m_nFix;	m_nBuild = Ver.m_nBuild;
}

CSysVer::~CSysVer(void)
{
}

CSysVer & CSysVer::operator=( const CSysVer & src )
{
	m_nMajor = src.m_nMajor;
	m_nMinor = src.m_nMinor;
	m_nFix = src.m_nFix;
	m_nBuild = src.m_nBuild;

	return *this;
}

bool CSysVer::operator > ( const CSysVer & rhs )
{
	if( m_nMajor > rhs.m_nMajor )
		return true;
	else if( m_nMajor < rhs.m_nMajor )
		return false;

	//
	if( m_nMinor > rhs.m_nMinor )
		return true;
	else if( m_nMinor < rhs.m_nMinor )
		return false;

	//
	if( m_nFix > rhs.m_nFix )
		return true;
	else if( m_nFix < rhs.m_nFix )
		return false;

	//
	if( m_nBuild > rhs.m_nBuild )
		return true;
	else if( m_nBuild < rhs.m_nBuild )
		return false;

	return false;
}

bool CSysVer::operator == ( const CSysVer & rhs )
{
	if( m_nMajor == rhs.m_nMajor && 
		m_nMinor == rhs.m_nMinor && 
		m_nFix == rhs.m_nFix && 
		m_nBuild == rhs.m_nBuild )
	{
		return true;
	}
	else
		return false;
}

bool CSysVer::operator < ( const CSysVer & rhs )
{
	if( *this > rhs )
		return false;
	else if( *this == rhs )
		return false;
	else
		return true;
}



