#include <Windows.h>
#include "RegControl.h"

#include <iostream>
#include <vector>
#include <algorithm>
#include <tuple>

#define WIDE_HELPER(x) L##x
#define TO_WIDE(x) WIDE_HELPER(x)

CRegControl::~CRegControl(void)
{
}

bool CRegControl::Register(
	const std::wstring & sProviderName //L"Elpusk"
	, const std::wstring& sClassRootKey //L"DefaultMsr", "DefaultLock"
	, const std::wstring& sDeviceName //L"Lpu230MSR", L"Lpu230Lock"
	, const std::wstring& sDeviceNameKey //L"OposSo.Lpu230" , L"OposSo.Lpu230Lock"
	, const std::wstring& sOposClassKey //OPOS_CLASSKEY_MSR, OPOS_CLASSKEY_LOCK of OPOS spec
	, const std::wstring& sAppUIDKey
	, const std::wstring& sAppUID
	,  bool bTest /*= false*/
)
{
	bool bResult = true;
	std::wstring sServiceKey(std::wstring(TO_WIDE(OPOS_ROOTKEY_PROVIDER)) +
		L"\\" +	sProviderName +	L"\\"
	);
	std::wstring sDeviceKey(
		std::wstring(TO_WIDE(OPOS_ROOTKEY)) +
		L"\\" +
		sOposClassKey +
		L"\\" +
		sDeviceName +
		L"\\"
	);
	std::wstring sDeviceRootKey(
		std::wstring(TO_WIDE(OPOS_ROOTKEY)) +
		L"\\" +
		sOposClassKey +
		L"\\"
	);

	if (bTest)
		bResult = CRegControl::SetString(sServiceKey, std::wstring(L""), sDeviceNameKey);
	else {
		bResult = CRegControl::SetString(sDeviceKey, std::wstring(L""), sDeviceNameKey);
		if (bResult) {
			bResult = CRegControl::SetString(sDeviceRootKey, sClassRootKey, sDeviceName);
			if (!sAppUIDKey.empty() &&!sAppUID.empty() && bResult) {
				bResult = CRegControl::SetString(sDeviceKey, sAppUIDKey, sAppUID);
			}
		}
	}
	return bResult;
}

bool CRegControl::UnRegister(
	const std::wstring& sPhysicalDeviceName //L"Lpu230MSR","Lpu230Lock"
	, const std::wstring& sOposClassKey //OPOS_CLASSKEY_MSR, OPOS_CLASSKEY_LOCK of OPOS spec
)
{
	bool bResult = true;

	std::wstring sDeviceKey(
		std::wstring(TO_WIDE(OPOS_ROOTKEY)) +
		L"\\" +
		sOposClassKey +
		L"\\" +
		sPhysicalDeviceName +
		L"\\"
	);

	bResult = CRegControl::Delete( sDeviceKey, std::wstring( L"") );
#ifdef	_YSS_ENABLE_STDOUT_FOR_DEBUG_
	if(bResult)
		wprintf(L"SUCCESS : del : %ls : \n", sDeviceKey.c_str());
	else {
		wprintf(L"ERROR : del : %ls : \n", sDeviceKey.c_str());
	}
#endif		//_YSS_ENABLE_STDOUT_FOR_DEBUG_

	return bResult;
}

bool CRegControl::UnRegister(
	const std::list<std::wstring>& list_physical_device_name
	, const std::wstring& sOposClassKey
)
{
	bool b_result(false);

	std::wstring s_dev_class_key(
		std::wstring(TO_WIDE(OPOS_ROOTKEY)) +
		L"\\" +
		sOposClassKey +
		L"\\"
	);

	std::list<std::wstring> list_value_name;

	do {
		b_result = true;

		// delete subkey(physical device name)
		std::for_each(std::cbegin(list_physical_device_name), std::cend(list_physical_device_name), [=](const std::wstring& s_sub_key_name) {
			if (!s_sub_key_name.empty()) {
				CRegControl::Delete(s_dev_class_key, s_sub_key_name);// ignore del error
			}
			});

		// delete value with data == device_name of list_device_name
		size_t n_number_value = CRegControl::GetAllValueNameOfHLM(s_dev_class_key, list_value_name);
		if (n_number_value == 0) {
			continue;
		}
		std::for_each(std::cbegin(list_value_name), std::cend(list_value_name), [=](const std::wstring& s_value_name) {
			if (!s_value_name.empty()) {
				std::wstring s_data = CRegControl::GetString(s_dev_class_key, s_value_name);
				if (!s_data.empty()) {
					std::list<std::wstring>::const_iterator cit = std::find(list_physical_device_name.cbegin(), list_physical_device_name.cend(), s_data);
					if (cit != std::cend(list_physical_device_name)) {
						CRegControl::DeleteValue(s_dev_class_key, s_value_name);
					}
				}
			}
			});

	} while (false);

	return b_result;
}

bool CRegControl::SetNumber( const std::wstring & sBaseKey, const std::wstring & sKeyName, int nValue )
{
	bool bResult = true;

    HKEY hKey;   
    std::wstring sTemp = std::to_wstring( static_cast<long long>(nValue) );

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

int CRegControl::GetNumber( const std::wstring & sBaseKey, const std::wstring & sKeyName )
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
   
    return std::stoi( std::wstring(strVal) );   
}

bool CRegControl::RenameKey(const std::wstring& sBaseKey, const std::wstring& sOldKeyName, const std::wstring& sNewKeyName)
{
	bool b_result(false);
	HKEY hKey(nullptr);

	do {
		if (sBaseKey.empty() || sOldKeyName.empty() || sNewKeyName.empty()) {
			continue;
		}
		
		if (::RegOpenKeyEx(HKEY_LOCAL_MACHINE, sBaseKey.c_str(), 0, KEY_WRITE, &hKey) != ERROR_SUCCESS) {
			continue;
		}

		if (::RegRenameKey(hKey, sOldKeyName.c_str(), sNewKeyName.c_str()) != ERROR_SUCCESS) {
			continue;
		}

		::RegFlushKey(hKey);
		b_result = true;
	} while (false);

	if(hKey){
		::RegCloseKey(hKey);
	}
	return b_result;
}

bool CRegControl::SetString( const std::wstring & sBaseKey, const std::wstring & sValueName, const std::wstring & sData )
{
	bool bResult = false;

	HKEY hKey;

	if ( ::RegCreateKeyEx(HKEY_LOCAL_MACHINE, sBaseKey.c_str(), NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, NULL, &hKey, NULL) != ERROR_SUCCESS ){
		return bResult;
	}   
   
	long lResult = ::RegSetValueEx(hKey, sValueName.c_str(), 0, REG_SZ, (unsigned char *)sData.c_str(), sData.length()*sizeof(wchar_t) );
	if (lResult != ERROR_SUCCESS) {   
		::RegCloseKey(hKey);   
		return bResult;
	}
   
	::RegFlushKey( hKey );
	::RegCloseKey(hKey);

	bResult = true;

	return bResult;
}

/**
* @brief change value name of the sBaseKey key
*/
bool CRegControl::ChangeValueName(const std::wstring& sBaseKey, const std::wstring& sCurValueName, const std::wstring& sNewValueName)
{
	std::wstring s_data = CRegControl::GetString(sBaseKey, sCurValueName);
	CRegControl::DeleteValue(sBaseKey, sCurValueName);// 오류 무시
	return CRegControl::SetString(sBaseKey, sNewValueName, s_data);
}


std::pair<size_t, size_t> CRegControl::GetAllSubKeyNameAndValueNameOfHLM(
	const std::wstring& sBaseKey
	, std::list<std::wstring>& sOut_list_subkey_name
	, std::list<std::wstring>& sOut_list_value_name
)
{
	size_t n_number_of_key(0), n_number_of_value(0);
	HKEY h_base_key(nullptr);
	do {
		sOut_list_subkey_name.clear();
		sOut_list_value_name.clear();
		//
		if (::RegOpenKeyEx(HKEY_LOCAL_MACHINE, sBaseKey.c_str(), 0, KEY_READ, &h_base_key) != ERROR_SUCCESS) {
			continue;
		}

		DWORD cSubKeys = 0;           // 하위 키 개수
		DWORD cMaxSubKeyLen = 0;      // 가장 긴 하위 키 이름 길이
		DWORD cValues = 0;            // 값(Value) 개수
		DWORD cMaxValueNameLen = 0;   // 가장 긴 값 이름 길이

		// 키의 정보(개수 및 최대 길이) 조회
		LONG lResult = RegQueryInfoKey(
			h_base_key,
			nullptr, nullptr, nullptr,
			&cSubKeys, &cMaxSubKeyLen,
			nullptr,
			&cValues, &cMaxValueNameLen,
			nullptr, nullptr, nullptr
		);

		if (lResult != ERROR_SUCCESS) {
			continue;
		}

		//모든 하위 키(Subkey) 이름 열거
		if (cSubKeys > 0) {
			// 널 종료 문자(\0)를 포함하기 위해 +1 크기 할당
			std::vector<wchar_t> subKeyName(cMaxSubKeyLen + 1);
			for (DWORD i = 0; i < cSubKeys; ++i) {
				DWORD nameSize = static_cast<DWORD>(subKeyName.size());
				lResult = RegEnumKeyEx(
					h_base_key,
					i,
					subKeyName.data(),
					&nameSize,
					nullptr, nullptr, nullptr, nullptr
				);
				if (lResult != ERROR_SUCCESS) {
					break;
				}
				else {
					sOut_list_subkey_name.push_back(subKeyName.data());
				}
			}//end for
		}

		if (lResult != ERROR_SUCCESS) {
			sOut_list_subkey_name.clear();
			continue;// error
		}

		// 모든 값(Value) 이름 열거
		if (cValues > 0) {
			// 널 종료 문자(\0)를 포함하기 위해 +1 크기 할당
			std::vector<wchar_t> valueName(cMaxValueNameLen + 1);
			for (DWORD i = 0; i < cValues; ++i) {
				DWORD nameSize = static_cast<DWORD>(valueName.size());
				lResult = RegEnumValue(
					h_base_key,
					i,
					valueName.data(),
					&nameSize,
					nullptr, nullptr, nullptr, nullptr
				);
				if (lResult == ERROR_SUCCESS) {
					if (nameSize == 0) {
						// 이름이 빈 문자열("")인 경우 레지스트리 편집기의 '(기본값)'을 의미함
						sOut_list_value_name.push_back(L"");
					}
					else {
						sOut_list_value_name.push_back(valueName.data());
					}
				}
				else {
					break;
				}
			}//end for
		}
		if (lResult != ERROR_SUCCESS) {
			sOut_list_subkey_name.clear();
			sOut_list_value_name.clear();
			continue;// error
		}

	} while (false);

	if (h_base_key) {
		::RegCloseKey(h_base_key);
	}

	n_number_of_key = sOut_list_subkey_name.size();
	n_number_of_value = sOut_list_value_name.size();

	return std::make_pair(n_number_of_key, n_number_of_value);
}

size_t CRegControl::GetAllValueNameOfHLM(const std::wstring& sBaseKey, std::list<std::wstring>& sOut_list_value_name)
{
	size_t n_number_of_value(0);
	HKEY h_base_key(nullptr);
	do {
		sOut_list_value_name.clear();
		//
		if (::RegOpenKeyEx(HKEY_LOCAL_MACHINE, sBaseKey.c_str(), 0, KEY_READ, &h_base_key) != ERROR_SUCCESS) {
			continue;
		}

		DWORD cValues = 0;            // 값(Value) 개수
		DWORD cMaxValueNameLen = 0;   // 가장 긴 값 이름 길이

		// 키의 value 정보(개수 및 최대 길이) 조회
		LONG lResult = RegQueryInfoKey(
			h_base_key,
			nullptr, nullptr, nullptr,
			nullptr, nullptr,
			nullptr,
			&cValues, &cMaxValueNameLen,
			nullptr, nullptr, nullptr
		);

		if (lResult != ERROR_SUCCESS) {
			continue;
		}

		// 모든 값(Value) 이름 열거
		if (cValues > 0) {
			// 널 종료 문자(\0)를 포함하기 위해 +1 크기 할당
			std::vector<wchar_t> valueName(cMaxValueNameLen + 1);
			for (DWORD i = 0; i < cValues; ++i) {
				DWORD nameSize = static_cast<DWORD>(valueName.size());
				lResult = RegEnumValue(
					h_base_key,
					i,
					valueName.data(),
					&nameSize,
					nullptr, nullptr, nullptr, nullptr
				);
				if (lResult == ERROR_SUCCESS) {
					if (nameSize == 0) {
						// 이름이 빈 문자열("")인 경우 레지스트리 편집기의 '(기본값)'을 의미함
						sOut_list_value_name.push_back(L"");
					}
					else {
						sOut_list_value_name.push_back(valueName.data());
					}
				}
				else {
					break;
				}
			}//end for
		}
		if (lResult != ERROR_SUCCESS) {
			sOut_list_value_name.clear();
			continue;// error
		}

	} while (false);

	if (h_base_key) {
		::RegCloseKey(h_base_key);
	}

	n_number_of_value = sOut_list_value_name.size();

	return n_number_of_value;
}

std::wstring CRegControl::GetOposPhysicalDeviceName(
	const std::wstring& sClassRootKey //L"DefaultMsr", "DefaultLock"
	, const std::wstring& sOposClassKey //OPOS_CLASSKEY_MSR, OPOS_CLASSKEY_LOCK of OPOS spec
	, const std::wstring& sAppUIDKey //for elpusk device verification
	, const std::wstring& sAppUID //for elpusk device verification
)
{
	std::wstring sDeviceName;
	std::wstring sDeviceRootKey(
		std::wstring(TO_WIDE(OPOS_ROOTKEY)) +
		L"\\" +
		sOposClassKey +
		L"\\"
	);

	sDeviceName = CRegControl::GetString(sDeviceRootKey, sClassRootKey);

	if (!sAppUIDKey.empty() && !sAppUID.empty() && !sDeviceName.empty()) {
		std::wstring sDeviceKey(
			sDeviceRootKey +
			sDeviceName +
			L"\\"
		);

		// sDeviceName is elpusk device, check AppUID
		std::wstring sUid = CRegControl::GetString(sDeviceKey, sAppUIDKey);

		if (sAppUID.compare(sUid) != 0) {
			sDeviceName.clear(); //failed to verify elpusk device
		}
	}

	return sDeviceName;
}

std::list<std::wstring> CRegControl::GetOposPhysicalDeviceName(
	const std::wstring& sOposClassKey
	, const std::wstring& sAppUIDKey
	, const std::wstring& sAppUID
)
{
	std::map<std::wstring,std::list<std::wstring>> m = CRegControl::GetOposDeviceName(
		sOposClassKey, sAppUIDKey, sAppUID
	);

	std::list<std::wstring> list_device_name;
	std::map<std::wstring, std::list<std::wstring>>::iterator it = m.begin();

	std::for_each(std::begin(m), std::end(m), [&](const std::pair<std::wstring, std::list<std::wstring>>& p) {
		if (!p.first.empty()) {
			list_device_name.push_back(p.first);// key 값(physical device name) 저장.
		}
		});

	return list_device_name;
}

std::list<std::wstring> CRegControl::GetOposLogicalDeviceName(const std::wstring& sOposClassKey, const std::wstring& sAppUIDKey, const std::wstring& sAppUID)
{
	std::map<std::wstring, std::list<std::wstring>> m = CRegControl::GetOposDeviceName(
		sOposClassKey, sAppUIDKey, sAppUID
	);

	std::list<std::wstring> list_device_name;
	std::map<std::wstring, std::list<std::wstring>>::iterator it = m.begin();

	std::for_each(std::begin(m), std::end(m), [&](const std::pair<std::wstring, std::list<std::wstring>>& p) {
		if (!p.first.empty() && !p.second.empty()) {
			list_device_name.insert(list_device_name.end(), p.second.begin(), p.second.end());
		}
		});

	return list_device_name;
}

/**
* @brief get physical & logical device name
*
*	A single physical device name can have multiple logical device names
*
* @return map key - the device name of CCO's open method.(physical device name)
* @return map value  - the lgoical device list(LogicalNames of POS for .Net DeviceInfo class )
*/
std::map<std::wstring, std::list<std::wstring>> CRegControl::GetOposDeviceName
(
	const std::wstring& sOposClassKey
	, const std::wstring& sAppUIDKey
	, const std::wstring& sAppUID
)
{
	std::map<std::wstring, std::list<std::wstring>> map_result;

	do {
		if (sOposClassKey.empty()) {
			continue;
		}
		if (sAppUIDKey.empty()) {
			continue;
		}
		if (sAppUID.empty()) {
			continue;
		}

		// sDeviceRootKey -> maybe ...\OLEforRetail\ServiceOPOS\MSR or ...\OLEforRetail\ServiceOPOS\KeyLock
		std::wstring sDeviceRootKey(
			std::wstring(TO_WIDE(OPOS_ROOTKEY)) +
			L"\\" +
			sOposClassKey +
			L"\\"
		); 

		size_t n_number_of_key(0), n_number_of_value(0);
		std::list<std::wstring> list_subkey_name;
		std::list<std::wstring> list_value_name;

		std::tie(n_number_of_key, n_number_of_value) = CRegControl::GetAllSubKeyNameAndValueNameOfHLM(
			sDeviceRootKey,	list_subkey_name, list_value_name
		);

		// list_subkey_name : physical device name 은 mandatory.
		// list_value_name : logical device name 은 option
		if (n_number_of_key == 0) {
			continue;
		}

		// list_subkey_name 의 각 key 의 value 중 name 이 sAppUIDKey 인지 검사해서, 없으면
		// list_subkey_name 에서 삭제
		// value name 이 sAppUIDKey 이면 data 가 sAppUID 인지 검사해서, 아니면
		// list_subkey_name 에서 삭제
		std::wstring s_base_key;
		std::wstring sUid;
		std::list<std::wstring>::iterator it_subkey_name = list_subkey_name.begin();
		for (; it_subkey_name != std::end(list_subkey_name); ++it_subkey_name) {
			if (it_subkey_name->empty()) {
				it_subkey_name = list_subkey_name.erase(it_subkey_name);
				continue;
			}
			s_base_key = sDeviceRootKey + *it_subkey_name + L"\\";
			sUid = CRegControl::GetString(s_base_key, sAppUIDKey);

			if (sAppUID.compare(sUid) != 0) {
				it_subkey_name = list_subkey_name.erase(it_subkey_name);
			}
			else {
				//save physical device name
				map_result[*it_subkey_name] = std::list<std::wstring>();
			}
		}//end for

		if (list_subkey_name.empty()) {
			map_result.clear();
			continue; //physical device name 은 mandatory.
		}

		// list_value_name 의 value 의 string data 를 얻어와
		// 이 string data 가 list_subkey_name 에 없으면, list_value_name 에서
		// 해당 value name 을 삭제.
		std::wstring s_value_data;
		std::list<std::wstring>::iterator it_value_name = list_value_name.begin();
		for (; it_value_name != std::end(list_value_name); ++it_value_name) {
			if (it_value_name->empty()) {
				continue;
			}
			s_value_data = CRegControl::GetString(sDeviceRootKey, *it_value_name);

			if (s_value_data.empty()) {
				it_value_name = list_value_name.erase(it_value_name);
				continue;
			}

			std::list<std::wstring>::iterator it = std::find(list_subkey_name.begin(), list_subkey_name.end(), s_value_data);
			if (it == std::end(list_subkey_name)) {
				it_value_name = list_value_name.erase(it_value_name);
			}
			else {
				map_result[*it].push_back(*it_value_name); // save logical device name
			}
		}//end for

		// logical device name 은 option 이므로 list_value_name() 가 비어도 OK.

	} while (false);

	return map_result;
}

bool CRegControl::SetOposPhysicalDeviceName(
	const std::wstring& sCurPhysicalDeviceName //current Physical device name
	, const std::wstring& sNewPhysicalDeviceName	// New Physical device name
	, const std::wstring& sOposClassKey //OPOS_CLASSKEY_MSR, OPOS_CLASSKEY_LOCK of OPOS spec
	, const std::wstring& sAppUIDKey //for elpusk device verification
	, const std::wstring& sAppUID //for elpusk device verification
)
{
	bool b_result(false);

	do {
		if (sCurPhysicalDeviceName.empty()) {
			continue;
		}
		if (sNewPhysicalDeviceName.empty()) {
			continue;
		}
		if(sOposClassKey.empty()) {
			continue;
		}

		std::map<std::wstring, std::list<std::wstring>> m = CRegControl::GetOposDeviceName(sOposClassKey, sAppUIDKey, sAppUID);

		std::wstring sDeviceRootKey(
			std::wstring(TO_WIDE(OPOS_ROOTKEY)) +
			L"\\" +
			sOposClassKey +
			L"\\"
		);

		if (m.empty()) {
			continue;
		}

		b_result = true;

		std::for_each(std::begin(m), std::end(m), [=](const std::pair<std::wstring, std::list<std::wstring>> & p) {
			std::wstring s_pdev;
			s_pdev = p.first;
			if (sCurPhysicalDeviceName.compare(s_pdev)==0) {
				// change logical device names
				std::for_each(std::begin(p.second), std::end(p.second), [=](const std::wstring& s_value_name) {
					if (!s_value_name.empty()) {
						CRegControl::SetString(sDeviceRootKey, s_value_name, sNewPhysicalDeviceName);//에러 무시
					}
					});

				// change physical device name
				CRegControl::RenameKey(sDeviceRootKey, s_pdev, sNewPhysicalDeviceName);//에러 무시
			}
			});

	} while (false);
	return b_result;
}

bool CRegControl::SetOposLogicalDeviceName(
	const std::wstring& sCurLogicalDeviceName
	, const std::wstring& sNewLogicalDeviceName
	, const std::wstring& sOposClassKey
	, const std::wstring& sAppUIDKey
	, const std::wstring& sAppUID
)
{
	bool b_result(false);

	do {
		if (sCurLogicalDeviceName.empty()) {
			continue;
		}
		if (sNewLogicalDeviceName.empty()) {
			continue;
		}
		if (sOposClassKey.empty()) {
			continue;
		}

		std::map<std::wstring, std::list<std::wstring>> m = CRegControl::GetOposDeviceName(sOposClassKey, sAppUIDKey, sAppUID);

		std::wstring sDeviceRootKey(
			std::wstring(TO_WIDE(OPOS_ROOTKEY)) +
			L"\\" +
			sOposClassKey +
			L"\\"
		);

		if (m.empty()) {
			continue;
		}

		b_result = true;
		//
		std::for_each(std::begin(m), std::end(m), [=](const std::pair<std::wstring, std::list<std::wstring>>& p) {
			// change logical device names
			std::for_each(std::begin(p.second), std::end(p.second), [=](const std::wstring& s_value_name) {
				if (sCurLogicalDeviceName.compare(s_value_name)==0) {
					CRegControl::ChangeValueName(sDeviceRootKey, s_value_name, sNewLogicalDeviceName);//에러 무시
				}
				});
			});

	} while (false);
	return b_result;
}

std::wstring CRegControl::GetString( const std::wstring & sBaseKey, const std::wstring & sValueName)
{
	std::wstring sValue;

	HKEY hKey;   
	if( ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, sBaseKey.c_str(), 0, KEY_READ, &hKey) != ERROR_SUCCESS) {   
		return sValue;
	}   
   
    // Find how much space I need to store this value    
	DWORD intType, intSize;   
	long lResult = ::RegQueryValueEx(hKey, sValueName.c_str(), NULL, &intType, NULL, &intSize);
	if (lResult != ERROR_SUCCESS || intType != REG_SZ) {   
		::RegCloseKey(hKey);   
		return sValue;
    }   
   
	// Retreive the value from the registry    
	wchar_t strVal[_MAX_PATH];
    lResult = ::RegQueryValueEx(hKey, sValueName.c_str(), NULL, &intType, (LPBYTE) strVal, &intSize);
	if (lResult != ERROR_SUCCESS) {   
		::RegCloseKey(hKey);   
		return sValue;
    }   
   
	::RegCloseKey(hKey);   
   
    return std::wstring(strVal, (intSize/sizeof(wchar_t))-1); 
}


bool CRegControl::Delete( const std::wstring & sBaseKey, const std::wstring & sSubKeyName)
{
	bool bResult = false;

	HKEY hKey;   
	if ( ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, sBaseKey.c_str(), 0, KEY_READ | KEY_WRITE, &hKey) != ERROR_SUCCESS) {   
		return bResult;
	}   
   
	long lResult = ::RegDeleteKey(hKey, sSubKeyName.c_str() );
	if (lResult != ERROR_SUCCESS) {   
		::RegCloseKey(hKey);   
		return bResult;
	}
   
	::RegFlushKey( hKey );
	::RegCloseKey(hKey);

	bResult = true;

	return bResult;
}

bool CRegControl::DeleteValue(const std::wstring& s_base_key, const std::wstring& s_value_name)
{
	bool b_result(false);
	HKEY h_key(nullptr);

	do {
		if (s_base_key.empty()) {
			continue;
		}
		if (::RegOpenKeyEx(HKEY_LOCAL_MACHINE, s_base_key.c_str(), 0, KEY_READ | KEY_WRITE, &h_key) != ERROR_SUCCESS) {
			continue;
		}

		if (::RegDeleteValue(h_key, s_value_name.c_str()) != ERROR_SUCCESS) {
			continue;
		}
		::RegFlushKey(h_key);
		b_result = true;
	} while (false);

	if (h_key) {
		::RegCloseKey(h_key);
	}
	return b_result;
}
