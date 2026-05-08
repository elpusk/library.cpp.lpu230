#pragma once

//////////////////////////////////////////////
// load only boolean key value
//////////////////////////////////////////////


#include <map>
#include <string>

using namespace std;
using namespace ATL;

class CIniFile
{

public:
	typedef	pair<_tstring,_tstring>		TypeKey;
	typedef	std::map< pair<_tstring,_tstring>,_tstring>		TypeItemMap;

public:
	CIniFile(void);
	CIniFile( const _tstring & sIniFileName );

	~CIniFile(void);

	void setFile( const _tstring & sIniFileName );

	void Reset();

	bool LoadKey( const _tstring & sSection, const _tstring & sKey );

	const _tstring & getValue( const _tstring & sSection, const _tstring & sKey );

	int GetSize(){	return m_mapItems.size();	}

private:
	TypeItemMap m_mapItems;
	_tstring m_sIniFile;

private:
	CIniFile( const CIniFile & );
};

