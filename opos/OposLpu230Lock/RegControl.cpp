#include "StdAfx.h"
#include "RegControl.h"

#define	CT_PROVIDER_NAME			"Elpusk"
#define	CT_DEVICE_NAME				"OposSo.Lpu230Lock"
#define	CT_DEVICE_ROOT_KEY					"DefaultLock"
#define	CT_DEVICE_ROOT_KEY_VALUE			"Lpu230Lock"

const _tstring CRegControl::m_sServiceKey( _tstring(_T(OPOS_ROOTKEY_PROVIDER)) +
	_T("\\") + 
	_T(CT_PROVIDER_NAME) + 
	_T("\\")
	);
const _tstring CRegControl::m_sDeviceKey( 
	_tstring(_T(OPOS_ROOTKEY)) +
	_T("\\") +
	_tstring(_T(OPOS_CLASSKEY_LOCK)) +
	_T("\\Lpu230Lock\\")
	);

const _tstring CRegControl::m_sDeviceRootKey( 
	_tstring(_T(OPOS_ROOTKEY)) +
	_T("\\") +
	_tstring(_T(OPOS_CLASSKEY_LOCK)) +
	_T("\\")
	);

CRegControl::~CRegControl(void)
{
}

bool CRegControl::Register( bool bTest /*= false*/ )
{
	bool bResult = true;
	
	if( bTest )
		bResult = CRegControl::SetString( m_sServiceKey, _tstring( _T("") ), _tstring( _T(CT_DEVICE_NAME) ) );
	else{
		bResult = CRegControl::SetString( m_sDeviceKey, _tstring( _T("") ), _tstring( _T(CT_DEVICE_NAME) ) );
		if( bResult ){
			bResult = CRegControl::SetString( m_sDeviceRootKey, _tstring( _T(CT_DEVICE_ROOT_KEY) ), _tstring( _T(CT_DEVICE_ROOT_KEY_VALUE) ) );
		}
	}
	return bResult;
}

bool CRegControl::UnRegister( bool bTest /*= false*/ )
{
	bool bResult = true;

	if( bTest )
		bResult = CRegControl::Delete( m_sServiceKey, _tstring( _T("")   ) );
	else{
		bResult = CRegControl::Delete( m_sDeviceKey, _tstring( _T("")  ) );
	}

	return bResult;
}

bool CRegControl::SetNumber( const _tstring & sBaseKey, const _tstring & sKeyName, int nValue )
{
	bool bResult = true;

    HKEY hKey;   
    _tstring sTemp = to_tstring( static_cast<long long>(nValue) );

	if ( ::RegCreateKeyEx(HKEY_LOCAL_MACHINE, sBaseKey.c_str(), NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, NULL, &hKey, NULL) != ERROR_SUCCESS ){
       
		long lResult = ::RegSetValueEx(hKey, sKeyName.c_str(), 0, REG_SZ, (const BYTE*)sTemp.c_str(), sTemp.length()*sizeof(TCHAR) );   
		if( lResult != ERROR_SUCCESS )
			bResult = false;
		else
			::RegFlushKey( hKey );
	}
	else
		bResult = false;
   
    RegCloseKey(hKey);   
	return bResult;
}

int CRegControl::GetNumber( const _tstring & sBaseKey, const _tstring & sKeyName )
{
	int nValue = -1;

	HKEY hKey;   
	if( ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, sBaseKey.c_str(), 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
		return nValue;
	}   
   
	// Find how much space I need to store this value    
	DWORD intType, intSize;   
	long lResult = ::RegQueryValueEx(hKey, sKeyName.c_str(), NULL, &intType, NULL, &intSize);   
	if (lResult != ERROR_SUCCESS || intType != REG_SZ) {   
		::RegCloseKey(hKey);
		return nValue;
    }   
   
	// Retreive the value from the registry    
	TCHAR strVal[_MAX_PATH];   
	lResult = ::RegQueryValueEx(hKey, sKeyName.c_str(), NULL, &intType, (LPBYTE) strVal, &intSize);   
	if (lResult != ERROR_SUCCESS) {   
		::RegCloseKey(hKey);   
		return nValue;
    }   
   
	::RegCloseKey(hKey);   
   
    return std::stoi( _tstring(strVal) );   
}

bool CRegControl::SetString( const _tstring & sBaseKey, const _tstring & sKeyName, _tstring & sValue )
{
	bool bResult = false;

	HKEY hKey;

	if ( ::RegCreateKeyEx(HKEY_LOCAL_MACHINE, sBaseKey.c_str(), NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, NULL, &hKey, NULL) != ERROR_SUCCESS ){
		return bResult;
	}   
   
	long lResult = ::RegSetValueEx(hKey, sKeyName.c_str(), 0, REG_SZ, (unsigned char *)sValue.c_str(), sValue.length()*sizeof(TCHAR) );   
	if (lResult != ERROR_SUCCESS) {   
		::RegCloseKey(hKey);   
		return bResult;
	}
   
	::RegFlushKey( hKey );
	::RegCloseKey(hKey);

	bResult = true;

	return bResult;
}

_tstring CRegControl::GetString( const _tstring & sBaseKey, const _tstring & sKeyName )
{
	_tstring sValue;

	HKEY hKey;   
	if( ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, sBaseKey.c_str(), 0, KEY_READ, &hKey) != ERROR_SUCCESS) {   
		return sValue;
	}   
   
    // Find how much space I need to store this value    
	DWORD intType, intSize;   
	long lResult = ::RegQueryValueEx(hKey, sKeyName.c_str(), NULL, &intType, NULL, &intSize);   
	if (lResult != ERROR_SUCCESS || intType != REG_SZ) {   
		::RegCloseKey(hKey);   
		return sValue;
    }   
   
	// Retreive the value from the registry    
	TCHAR strVal[_MAX_PATH];   
    lResult = ::RegQueryValueEx(hKey, sKeyName.c_str(), NULL, &intType, (LPBYTE) strVal, &intSize);   
	if (lResult != ERROR_SUCCESS) {   
		::RegCloseKey(hKey);   
		return sValue;
    }   
   
	::RegCloseKey(hKey);   
   
    return _tstring(strVal, intSize - 1); 
}

bool CRegControl::Delete( const _tstring & sBaseKey, const _tstring & sKeyName )
{
	bool bResult = false;

	HKEY hKey;   
	if ( ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, sBaseKey.c_str(), 0, KEY_READ | KEY_WRITE, &hKey) != ERROR_SUCCESS) {   
		return bResult;
	}   
   
	long lResult = ::RegDeleteKey(hKey, sKeyName.c_str() );   
	if (lResult != ERROR_SUCCESS) {   
		::RegCloseKey(hKey);   
		return bResult;
	}
   
	::RegFlushKey( hKey );
	::RegCloseKey(hKey);

	bResult = true;

	return bResult;
}
