// SoLpu230Lock.h : CSoLpu230Lock의 선언입니다.

#pragma once
#include "resource.h"       // 주 기호입니다.

#include "OposLpu230Lock_i.h"
#include "_ISoLpu230LockEvents_CP.h"
#include "cmdset.h"
#include "TTR_DevHid.h"
#include "Sync.h"
#include "info_msr.h"
#include "SoLock.h"

#include <OposLock.hi>

#include <Deque>
#include <memory>
#include <vector>
#include <algorithm>
#include <map>

#include "Dll.h"


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "단일 스레드 COM 개체는 전체 DCOM 지원을 포함하지 않는 Windows Mobile 플랫폼과 같은 Windows CE 플랫폼에서 제대로 지원되지 않습니다. ATL이 단일 스레드 COM 개체의 생성을 지원하고 단일 스레드 COM 개체 구현을 사용할 수 있도록 _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA를 정의하십시오. rgs 파일의 스레딩 모델은 DCOM Windows CE가 아닌 플랫폼에서 지원되는 유일한 스레딩 모델이므로 'Free'로 설정되어 있습니다."
#endif

using namespace ATL;
using namespace SYNC;

// CSoLpu230Lock

class ATL_NO_VTABLE CSoLpu230Lock :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CSoLpu230Lock, &CLSID_SoLpu230Lock>,
	public IConnectionPointContainerImpl<CSoLpu230Lock>,
	public CProxy_ISoLpu230LockEvents<CSoLpu230Lock>,
	public IDispatchImpl<ISoLpu230Lock, &IID_ISoLpu230Lock, &LIBID_OposLpu230LockLib, /*wMajor =*/ 1, /*wMinor =*/ 0>
{
public:
	CSoLpu230Lock()
	{
		ATLTRACE( _T(" $ CSoLpu230Lock()::CSoLpu230Lock().c\n") );
		//m_pSoLock = CSoLock::GetInstance();
	}

	~CSoLpu230Lock()
	{
		ATLTRACE( _T(" $ CSoLpu230Lock()::~CSoLpu230Lock().c\n") );
		//m_pSoLock = CSoLock::GetInstance(true);//free
	}

DECLARE_REGISTRY_RESOURCEID(IDR_SOLPU230LOCK)


BEGIN_COM_MAP(CSoLpu230Lock)
	COM_INTERFACE_ENTRY(ISoLpu230Lock)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IConnectionPointContainer)
END_COM_MAP()

BEGIN_CONNECTION_POINT_MAP(CSoLpu230Lock)
	CONNECTION_POINT_ENTRY(__uuidof(_ISoLpu230LockEvents))
END_CONNECTION_POINT_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		ATLTRACE( _T("CSoLpu230Lock::FinalConstruct().\n") );
		return S_OK;
	}

	void FinalRelease()
	{
		ATLTRACE( _T("CSoLpu230Lock::FinalRelease().\n") );
	}

public:
	STDMETHOD(OpenService)(BSTR DeviceClass, BSTR DeviceName, IDispatch* pDispatch, LONG* pRC);
	STDMETHOD(GetPropertyNumber)(LONG PropIndex, LONG* pNumber);
	STDMETHOD(SetPropertyNumber)(LONG PropIndex, LONG Number);
	STDMETHOD(GetPropertyString)(LONG PropIndex, BSTR* pString);
	STDMETHOD(SetPropertyString)(LONG PropIndex, BSTR bstrString);
	STDMETHOD(COFreezeEvents)(VARIANT_BOOL Freeze, LONG* pRC);
	STDMETHOD(CheckHealth)(LONG Level, LONG* pRC);
	STDMETHOD(ClaimDevice)(LONG lTimeout, LONG* pRC);
	STDMETHOD(Close)(LONG* pRC);
	STDMETHOD(DirectIO)(LONG Command, LONG* pData, BSTR* pString, LONG* pRC);
	STDMETHOD(ReleaseDevice)(LONG* pRC);
	STDMETHOD(ResetStatistics)(BSTR StatisticsBuffer, LONG* pRC);
	STDMETHOD(RetrieveStatistics)(BSTR* StatisticsBuffer, LONG* pRC);
	STDMETHOD(UpdateStatistics)(BSTR StatisticsBuffer, LONG* pRC);
	STDMETHOD(WaitForKeylockChange)(LONG nKeyPosition, LONG nTimeout, LONG* pRC);

private:
	CSoLock m_SoLock; 

	
public:
	STDMETHOD(CompareFirmwareVersion)(BSTR FirmwareFileName, LONG* pResult, LONG* pRC);
	STDMETHOD(UpdateFirmware)(BSTR FirmwareFileName, LONG* pRC);
};

OBJECT_ENTRY_AUTO(__uuidof(SoLpu230Lock), CSoLpu230Lock)
