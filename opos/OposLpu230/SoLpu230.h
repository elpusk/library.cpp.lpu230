// SoLpu230.h : CSoLpu230의 선언입니다.

#pragma once
#include "resource.h"       // 주 기호입니다.

#include "OposLpu230_i.h"
#include "_ISoLpu230Events_CP.h"
#include "cmdset.h"
#include "TTR_DevHid.h"
#include "Sync.h"
#include "info_msr.h"
#include "SoMsr.h"

#include <OposMsr.hi>

#include <Deque>
#include <memory>
#include <vector>
#include <algorithm>
#include <map>


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "단일 스레드 COM 개체는 전체 DCOM 지원을 포함하지 않는 Windows Mobile 플랫폼과 같은 Windows CE 플랫폼에서 제대로 지원되지 않습니다. ATL이 단일 스레드 COM 개체의 생성을 지원하고 단일 스레드 COM 개체 구현을 사용할 수 있도록 _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA를 정의하십시오. rgs 파일의 스레딩 모델은 DCOM Windows CE가 아닌 플랫폼에서 지원되는 유일한 스레딩 모델이므로 'Free'로 설정되어 있습니다."
#endif


using namespace ATL;
using namespace SYNC;

// CSoLpu230

class ATL_NO_VTABLE CSoLpu230 :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CSoLpu230, &CLSID_SoLpu230>,
	public IConnectionPointContainerImpl<CSoLpu230>,
	public CProxy_ISoLpu230Events<CSoLpu230>,
	public IDispatchImpl<ISoLpu230, &IID_ISoLpu230, &LIBID_OposLpu230Lib, /*wMajor =*/ 1, /*wMinor =*/ 0>
{
public:
	CSoLpu230() : m_pSoMsr(NULL)
	{
		ATLTRACE( _T(" $ CSoLpu230()::CSoLpu230().c\n") );

		m_pSoMsr = CSoMsr::GetInstance();
	}

	~CSoLpu230()
	{
		ATLTRACE( _T(" $ CSoLpu230()::~CSoLpu230().c\n") );
		//m_pSoMsr = CSoMsr::GetInstance(true);//free
	}

DECLARE_REGISTRY_RESOURCEID(IDR_SOLPU230)


BEGIN_COM_MAP(CSoLpu230)
	COM_INTERFACE_ENTRY(ISoLpu230)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IConnectionPointContainer)
END_COM_MAP()

BEGIN_CONNECTION_POINT_MAP(CSoLpu230)
	CONNECTION_POINT_ENTRY(__uuidof(_ISoLpu230Events))
END_CONNECTION_POINT_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		ATLTRACE( _T("CSoLpu230::FinalConstruct().\n") );
		return S_OK;
	}

	void FinalRelease()
	{
		ATLTRACE( _T("CSoLpu230::FinalRelease().\n") );
	}

public:

	STDMETHOD(OpenService)(BSTR DeviceClass, BSTR DeviceName, IDispatch* pDispatch, long* pRC);
	STDMETHOD(GetPropertyNumber)(long PropIndex, long* pNumber);
	STDMETHOD(SetPropertyNumber)(long PropIndex, long Number);
	STDMETHOD(GetPropertyString)(long PropIndex, BSTR* pString);
	STDMETHOD(SetPropertyString)(long PropIndex, BSTR bstrString);
	STDMETHOD(GetOpenResult)(long* pRC);
	STDMETHOD(COFreezeEvents)(VARIANT_BOOL Freeze, long* pRC);
	STDMETHOD(CheckHealth)(long Level, long* pRC);
	STDMETHOD(ClaimDevice)(long lTimeout, long* pRC);
	STDMETHOD(ClearInput)(long* pRC);
	STDMETHOD(Close)(long* pRC);
	STDMETHOD(DirectIO)(long Command, long* pData, BSTR* pString, long* pRC);
	STDMETHOD(ReleaseDevice)(long* pRC);
	STDMETHOD(ResetStatistics)(BSTR StatisticsBuffer, long* pRC);
	STDMETHOD(RetrieveStatistics)(BSTR* StatisticsBuffer, long* pRC);
	STDMETHOD(UpdateStatistics)(BSTR StatisticsBuffer, long* pRC);

private:
	//event dipatch id
	//std::vector<DISPID> m_vEventDispIds;

	CSoMsr *m_pSoMsr; 

public:
	STDMETHOD(UpdateKey)(BSTR Key, BSTR KeyName, long* pRC);
};

OBJECT_ENTRY_AUTO(__uuidof(SoLpu230), CSoLpu230)
