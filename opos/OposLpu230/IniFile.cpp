#include "StdAfx.h"
#include "IniFile.h"
#include <fstream>
#include <algorithm>
#include <vector>

CIniFile::CIniFile(void)
{
}


CIniFile::~CIniFile(void)
{
}

CIniFile::CIniFile( const _tstring & sIniFileName ) : m_sIniFile( sIniFileName )
{
	
}

void CIniFile::setFile( const _tstring & sIniFileName )
{
	m_sIniFile = sIniFileName;
}

void CIniFile::Reset()
{
	m_mapItems.clear();
}

bool CIniFile::LoadKey( const _tstring & sSection, const _tstring & sKey )
{
	bool bResult = true;
	
	//check exist file
	_tifstream ifs( m_sIniFile.c_str(), ios::in );
	if( ifs.bad() ){
		return false;
	}

	ifs.close();
	//
	TCHAR sValue[_MAX_PATH];
	const _tstring sDefault( _T("BABO") );

	::memset( sValue, 0, sizeof(sValue) );
	if( ::GetPrivateProfileString( sSection.c_str(), sKey.c_str(), sDefault.c_str(),sValue ,_MAX_PATH, m_sIniFile.c_str() ) >0  ){

		_tstring Value( sValue );

		if( Value.compare( sDefault ) != 0 ){
			m_mapItems.insert( TypeItemMap::value_type( TypeKey( sSection, sKey ), Value ) );
		}
		else{
			bResult = false;
		}
	}
	else
		bResult = false;

	return bResult;
}

const _tstring & CIniFile::getValue( const _tstring & sSection, const _tstring & sKey )
{
	static _tstring Value;

	TypeItemMap::iterator iterFind = m_mapItems.find( TypeKey( sSection, sKey ) );

	if( iterFind != m_mapItems.end() ){
		//find value
		Value = iterFind->second;
	}
	else
		Value = _T("");

	return Value;
}

