#pragma once

#include <Opos.hi>
#include <string>

using namespace std;

class CRegControl
{
public:
	virtual ~CRegControl(void);

	static bool Register( bool bTest = false );
	static bool UnRegister( bool bTest = false );

	static bool SetNumber( const _tstring & sBaseKey, const _tstring & sKeyName, int nValue );
	static int GetNumber( const _tstring & sBaseKey, const _tstring & sKeyName );

	static bool SetString( const _tstring & sBaseKey, const _tstring & sKeyName, _tstring & sValue );
	static _tstring GetString( const _tstring & sBaseKey, const _tstring & sKeyName );

	static bool Delete( const _tstring & sBaseKey, const _tstring & sKeyName );

private:
	CRegControl(void);
	CRegControl(CRegControl &);

	static const _tstring m_sDeviceRootKey;
	static const _tstring m_sDeviceKey;
	static const _tstring m_sServiceKey;

};

