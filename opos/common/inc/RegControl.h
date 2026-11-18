#pragma once

#include <Opos.hi>
#include <string>
#include <utility>
#include <list>
#include <map>

class CRegControl
{
public:
	virtual ~CRegControl(void);

	static bool CRegControl::Register(
		const std::wstring& sProviderName //L"Elpusk"
		, const std::wstring& sClassRootKey //L"DefaultMsr", "DefaultLock"
		, const std::wstring& sDeviceName //L"Lpu230MSR", "Lpu230Lock"
		, const std::wstring& sDeviceNameKey //L"OposSo.Lpu230" , L"OposSo.Lpu230Lock"
		, const std::wstring& sOposClassKey //OPOS_CLASSKEY_MSR, OPOS_CLASSKEY_LOCK of OPOS spec
		, const std::wstring& sAppUIDKey //for elpusk device verification
		, const std::wstring& sAppUID //for elpusk device verification
		, bool bTest = false
	);
	static bool UnRegister( 
		const std::wstring& sPhysicalDeviceName //L"Lpu230MSR", "Lpu230Lock"
		, const std::wstring& sOposClassKey //OPOS_CLASSKEY_MSR, OPOS_CLASSKEY_LOCK of OPOS spec
	);
	static bool UnRegister(
		const std::list<std::wstring>& list_physical_device_name //the list of L"Lpu230MSR", "Lpu230Lock"
		, const std::wstring& sOposClassKey //OPOS_CLASSKEY_MSR, OPOS_CLASSKEY_LOCK of OPOS spec
	);

	static bool SetNumber( const std::wstring & sBaseKey, const std::wstring & sKeyName, int nValue );
	static int GetNumber( const std::wstring & sBaseKey, const std::wstring & sKeyName );

	static bool RenameKey(const std::wstring& sBaseKey, const std::wstring& sOldKeyName, const std::wstring& sNewKeyName);

	/**
	* @brief set data of a value in the sBaseKey key
	*/
	static bool SetString( const std::wstring & sBaseKey, const std::wstring & sValueName, const std::wstring & sData);

	/**
	* @brief change value name of the sBaseKey key
	*/
	static bool ChangeValueName(const std::wstring& sBaseKey, const std::wstring& sCurValueName, const std::wstring& sNewValueName);

	static std::wstring GetString( const std::wstring & sBaseKey, const std::wstring & sValueName );

	/**
	* @return first - the number of subkey
	* @return second - the number of value
	*/
	static std::pair<size_t, size_t> GetAllSubKeyNameAndValueNameOfHLM(
		const std::wstring& sBaseKey
		, std::list<std::wstring>& sOut_list_subkey_name
		, std::list<std::wstring>& sOut_list_value_name
	);

	static size_t GetAllValueNameOfHLM(
		const std::wstring& sBaseKey
		, std::list<std::wstring>& sOut_list_value_name
	);

	/**
	* @brief get physical device name of the given logical device
	* @param sClassRootKey - logical device anme
	* @return physical device name
	*/
	static std::wstring GetOposPhysicalDeviceName(
		const std::wstring& sClassRootKey //L"DefaultMsr", "DefaultLock"
		, const std::wstring& sOposClassKey //OPOS_CLASSKEY_MSR, OPOS_CLASSKEY_LOCK of OPOS spec
		, const std::wstring& sAppUIDKey //for elpusk device verification
		, const std::wstring& sAppUID //for elpusk device verification
	);

	/**
	* @brief get physical device name of the given logical device
	* @param sClassRootKey - logical device anme
	* @return physical device name list
	*/
	static std::list<std::wstring> GetOposPhysicalDeviceName(
		const std::wstring& sOposClassKey //OPOS_CLASSKEY_MSR, OPOS_CLASSKEY_LOCK of OPOS spec
		, const std::wstring& sAppUIDKey //for elpusk device verification
		, const std::wstring& sAppUID //for elpusk device verification
	);

	/**
	* @brief get logical device name of the given logical device
	* @param sClassRootKey - logical device anme
	* @return logical device name list
	*/
	static std::list<std::wstring> GetOposLogicalDeviceName(
		const std::wstring& sOposClassKey //OPOS_CLASSKEY_MSR, OPOS_CLASSKEY_LOCK of OPOS spec
		, const std::wstring& sAppUIDKey //for elpusk device verification
		, const std::wstring& sAppUID //for elpusk device verification
	);

	/**
	* @brief get physical & logical device name
	* 
	*	A single physical device name can have multiple logical device names
	* 
	* @return map key - the device name of CCO's open method.(physical device name)
	* @return map value  - the lgoical device list(LogicalNames of POS for .Net DeviceInfo class )
	*/
	static std::map<std::wstring, std::list<std::wstring>>
		GetOposDeviceName(
		const std::wstring& sOposClassKey //OPOS_CLASSKEY_MSR, OPOS_CLASSKEY_LOCK of OPOS spec
		, const std::wstring& sAppUIDKey //for elpusk device verification
		, const std::wstring& sAppUID //for elpusk device verification
	);

	/**
	* @brief change physical device name
	* @param sCurPhysicalDeviceName - current physical device name
	* @param sNewPhysicalDeviceName - new physical device name
	*/
	static bool SetOposPhysicalDeviceName(
		const std::wstring& sCurPhysicalDeviceName //current Physical device name
		, const std::wstring& sNewPhysicalDeviceName //New Physical device name
		, const std::wstring& sOposClassKey //OPOS_CLASSKEY_MSR, OPOS_CLASSKEY_LOCK of OPOS spec
		, const std::wstring& sAppUIDKey //for elpusk device verification
		, const std::wstring& sAppUID //for elpusk device verification
	);

	/**
	* @brief change logical device name
	* @param sCurLogicalDeviceName - current logical device name
	* @param sNewPhysicalDeviceName - new physical device name
	*/
	static bool SetOposLogicalDeviceName(
		const std::wstring& sCurLogicalDeviceName //current logical device name
		, const std::wstring& sNewLogicalDeviceName //New logical device name
		, const std::wstring& sOposClassKey //OPOS_CLASSKEY_MSR, OPOS_CLASSKEY_LOCK of OPOS spec
		, const std::wstring& sAppUIDKey //for elpusk device verification
		, const std::wstring& sAppUID //for elpusk device verification
	);

	/**
	* @description delete subkey of sBaseKey
	*/
	static bool Delete( const std::wstring & sBaseKey, const std::wstring & sSubKeyName );

	/**
	* @description delete value of a key
	*/
	static bool DeleteValue(const std::wstring& s_base_key, const std::wstring& s_value_name);

private:
	CRegControl(void);
	CRegControl(CRegControl &);
};

