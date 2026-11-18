// OposLpu230.cpp : DLL 내보내기의 구현입니다.


#include "stdafx.h"
#include "resource.h"
#include "OposLpu230_i.h"
#include "dllmain.h"
#include "RegControl.h"
#include "SoMsr.h"
#include "el_opos/el_opos.h"
#include <map>

#define WIDE_HELPER(x) L##x
#define TO_WIDE(x) WIDE_HELPER(x)

// DLL이 OLE에 의해 언로드될 수 있는지 결정하는 데 사용됩니다.
STDAPI DllCanUnloadNow(void)
{
	CLog::GetLog()->Log( true, CLog::LEV_HIGH, _T("[DETAIL] CALLED : DllCanUnloadNow.\n")  );

	HRESULT hResult = _AtlModule.DllCanUnloadNow();

	if( hResult == S_OK  ){
		CSoMsr::GetInstance(true);//free
		CLog::GetLog()->Log( true, CLog::LEV_HIGH, _T("[DETAIL] Removed CSoMsr.\n")  );
	}
	else{// it is dregon!
		if( _AtlModule.GetLockCount() == 0 ){//ORG == 1
			//force termination
			CSoMsr::GetInstance(true);//free
			CLog::GetLog()->Log( true, CLog::LEV_HIGH, _T("[DETAIL] Removed CSoMsr(CNT==0).\n")  );
		}
	}
	

	ATLTRACE(_T(" @DllCanUnloadNow() = %d.\n"), hResult );
	return hResult;
}

// 클래스 팩터리를 반환하여 요청된 형식의 개체를 만듭니다.
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	CLog::GetLog()->Log( true, CLog::LEV_HIGH, _T("[DETAIL] CALLED : DllGetClassObject.\n")  );

	HRESULT hResult = _AtlModule.DllGetClassObject(rclsid, riid, ppv);

	ATLTRACE(_T(" @DllGetClassObject() = %d.\n"), hResult );

	return hResult;
}

// DllRegisterServer - 시스템 레지스트리에 항목을 추가합니다.
STDAPI DllRegisterServer(void)
{
	
	// default device name 으로 개체, 형식 라이브러리 및 형식 라이브러리에 들어 있는 모든 인터페이스를 등록합니다.
	CRegControl::Register(
		std::wstring(TO_WIDE(EL_OPOS_PROVIDER))
		, std::wstring(TO_WIDE(EL_CLASS_ROOT_KEY_MSR))
		, std::wstring(TO_WIDE(EL_DEVICE_NAME_DEFAULT_MSR))
		, std::wstring(TO_WIDE(EL_CLASS_KEY_MSR))
		, std::wstring(TO_WIDE(OPOS_CLASSKEY_MSR))
		, std::wstring(TO_WIDE(EL_APPUID_KEY))
		, std::wstring(TO_WIDE(EL_APPUID_SO_MSR))
	);

	HRESULT hr = _AtlModule.DllRegisterServer();

	ATLTRACE(_T(" @DllRegisterServer() = %d.\n"), hr );
	return hr;
}

// DllUnregisterServer - 시스템 레지스트리에서 항목을 제거합니다.
STDAPI DllUnregisterServer(void)
{
	std::list<std::wstring> list_physical_device_name = CRegControl::GetOposPhysicalDeviceName(
		std::wstring(TO_WIDE(OPOS_CLASSKEY_MSR))
		, std::wstring(TO_WIDE(EL_APPUID_KEY))
		, std::wstring(TO_WIDE(EL_APPUID_SO_MSR))
	);

	if (!list_physical_device_name.empty()) {
		CRegControl::UnRegister(
			list_physical_device_name
			, std::wstring(TO_WIDE(OPOS_CLASSKEY_MSR))
		);
	}
	else {
		ATLTRACE(_T(" @DllUnregisterServer() = None DeviceName.\n"));
	}

	HRESULT hr = _AtlModule.DllUnregisterServer();

	ATLTRACE(_T(" @DllUnregisterServer() = %d.\n"), hr );
	return hr;
}

// DllInstall - 항목을 사용자별 및 컴퓨터별로 시스템 레지스트리에 추가하거나 시스템 레지스트리에서 제거합니다.
STDAPI DllInstall(BOOL bInstall, LPCWSTR pszCmdLine)
{
	HRESULT hr = E_FAIL;
	static const wchar_t szUserSwitch[] = L"user";

	if (pszCmdLine != NULL)
	{
		if (_wcsnicmp(pszCmdLine, szUserSwitch, _countof(szUserSwitch)) == 0)
		{
			ATL::AtlSetPerUserRegistration(true);
		}
	}

	if (bInstall)
	{	
		hr = DllRegisterServer();
		if (FAILED(hr))
		{
			DllUnregisterServer();
		}
	}
	else
	{
		hr = DllUnregisterServer();
	}

	ATLTRACE(_T(" @DllInstall() = %d.\n"), hr );

	return hr;
}


