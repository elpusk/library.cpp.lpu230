// dllmain.h : 모듈 클래스의 선언입니다.

class COposLpu230Module : public ATL::CAtlDllModuleT< COposLpu230Module >
{
public :
	DECLARE_LIBID(LIBID_OposLpu230Lib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_OPOSLPU230, "{5236A4C5-D44A-4AE5-A2B5-50551B8DD152}")
};

extern class COposLpu230Module _AtlModule;
